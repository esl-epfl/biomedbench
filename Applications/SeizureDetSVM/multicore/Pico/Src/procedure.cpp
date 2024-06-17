/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////
// Author:          ESL team                    //  
// Optimizations:   Dimitrios Samakovlis        //
/////////////////////////////////////////////////



#include "procedure.hpp"

extern "C" {
    #include <stdio.h>
    #include <stdint.h>
    #include "fixmath.h"

    #include "pico/multicore.h"
    #include "Pico_management/multicore.h"
}

#include "global_config.hpp"
#include "output.hpp"
#include "rpeak_del.hpp"
#include "edr.hpp"
#include "rri_features.hpp"
#include "fixpnt_lin_pred_est.hpp"
#include "edr_features.hpp"
#include "svm.hpp"
#include "svm.inc"
using namespace fixpnt_32b;

// those are used for the API used for the cluster
int32_t *arg[12];		// we use this to pass arguments to cluster functions

// data used by the cluster functions
static fixed_t feature[NUM_FEATURES];
static int16_t *rPeak;
static int16_t *rri;
RriFeats rriFeats;
fixed_t edr[MAX_RPEAKS_PER_CORE * NUMBER_OF_CORES];
fixed_t hrv[MAX_RPEAKS_PER_CORE * NUMBER_OF_CORES];
fixed_t rPeakTime[MAX_RPEAKS_PER_CORE * NUMBER_OF_CORES];
fx_rdiv_t ifreq;
RriFreqFeats rriFreqFeats;
static fixed_t edrFreqFeat[EDR_NUM_FREQ_FEATS];
fixed_t out;
static int32_t edrLpc[EDR_LPC_ORDER];


// Intermediate functions of all modules for calling the cluster cores
static void multicore_DelineateRpeaks(void)	{
    DelineateRpeaks(arg, 1);
}

static void multicore_EcgDerivedRespiration(void)	{
    EcgDerivedRespiration(arg, 1);
}

static void multicore_ExtractRriFeatures(void)	{
    ExtractRriFeatures(arg, 1);
}

static void multicore_ExtractRriFreqFeatures(void)	{
    ExtractRriFreqFeatures(arg, 1);
}

static void multicore_ExtractEdrFreqFeatures(void)	{
    ExtractEdrFreqFeatures(arg, 1);
}

static void multicore_ComputeLinPredCoeffs(void)	{
    ComputeLinPredCoeffs(arg, 1);
}

static void multicore_predict(void)	{
    predict(arg, 1);
}


// Base function for the Fabric Controller that handles the data flow, activates and calls the cluster with the correct parameters 
bool PredictSeizure(int32_t* ecg_L2) {
    
    // allocate L1 arrays (need to free later due to L1 size limitations)
    rPeak = (int16_t *) malloc(sizeof(int16_t) * RPEAK_CAPACITY);
    rri = (int16_t *) malloc(sizeof(int16_t) * (RPEAK_CAPACITY-1));


    // MODULE 1: R peak delineation
    int rPeakSize;
    int32_t temp = fx_itox(1500, ECG_FRAC);
    int32_t size = WIN_SIZE;
    arg[0] = ecg_L2;
    arg[1] = &size;
    arg[2] = (int32_t *) rPeak;
    arg[3] = (int32_t *) &rPeakSize;
    arg[4] = (int32_t *) rri;
    arg[5] = &temp;
    
    // Call core 1
    multicore_launch_core1(multicore_DelineateRpeaks);
    DelineateRpeaks(arg, 0);
    multicore_reset_core1();
    
    if (PRINT_DELINEATION) {
        OutputVector(rPeak, rPeakSize, "rPeak");
    }


    // MODULE 2: ECG derived respiration (EDR).
    arg[5] = (int32_t *) edr;

    multicore_launch_core1(multicore_EcgDerivedRespiration);
    EcgDerivedRespiration(arg, 0);
    multicore_reset_core1();
    
    if (PRINT_EDR) OutputFixVector(edr, rPeakSize, "edr", ECG_FRAC);
    
    // Extraction of RR interval sequence features
    int rriSize = rPeakSize - 1;
    arg[6] = (int32_t *) &rriSize;
    arg[7] = (int32_t *) &rriFeats;
    arg[8] = (int32_t *) feature;
    
    multicore_launch_core1(multicore_ExtractRriFeatures);
    ExtractRriFeatures(arg, 0);
    multicore_reset_core1();
    
    if (PRINT_RRI_FAST_FEATURES) Output(rriFeats);
    
    // Extraction of RR interval frequency features
    arg[7] = (int32_t *) &rriFreqFeats;
    arg[9] = (int32_t *) hrv;
    arg[10] = (int32_t *) rPeakTime;
    arg[11] = (int32_t *) &ifreq;
    
    multicore_launch_core1(multicore_ExtractRriFreqFeatures);
    ExtractRriFreqFeatures(arg, 0);
    multicore_reset_core1();
    
    if (PRINT_RRI_FREQ_FEATURES) {
        Output(rriFreqFeats);
    }
    
    free((void *) rPeak);
    free((void *) rri);
    

    // MODULE 3: Linear prediction coefficients of the EDR signal
    arg[9] = edrLpc;

    multicore_launch_core1(multicore_ComputeLinPredCoeffs);
    ComputeLinPredCoeffs(arg, 0);
    multicore_reset_core1();
    
    if (PRINT_EDR_LPC)
        OutputFixVector(edrLpc, EDR_LPC_ORDER, "edrLpc", LPC_FRAC);

    // MODULE 3b: Extraction of EDR signal frequency features
    arg[11] = (int32_t *) edrFreqFeat;
    
    multicore_launch_core1(multicore_ExtractEdrFreqFeatures);
    ExtractEdrFreqFeatures(arg, 0);
    multicore_reset_core1();
    
    if (PRINT_FREQ_EDR) OutputFixVector(
        edrFreqFeat, EDR_NUM_FREQ_FEATS, "edrFreqFeat", FLOMB_FRAC
    );
    
    if (PRINT_FEATURES) {
        for (int i = 0; i < NUM_FEATURES; ++i) {
            if (FLOAT_OUTPUT) {
                printf(
                    "%f%c",
                    fx_xtof(feature[i], SVM_FRAC),
                    " \n"[i+1==NUM_FEATURES]
                );
            } else {
                printf("%d%c", feature[i], " \n"[i+1==NUM_FEATURES]);
            }
        }
    }

    // MODULE 4: Support Vector Machine
    arg[11] = (int32_t *)&out;
    arg[0] = (int32_t *)svmVect;
    arg[1] = alpha;
    arg[2] = scaleFeat;
    arg[3] = meanFeat;
    arg[4] = &bias;
    
    multicore_launch_core1(multicore_predict);
    predict(arg, 0);
    multicore_reset_core1();
    
    bool seizure = out > 0;
    
    if (PRINT_SVM) {
        OutputFixVector(&out, 1, "svm", SVM_FRAC);
    }
    
    if (PRINT_PREDICTION) {
        printf("Prediction: %s\n", seizure? "SEIZURE" : "NON_SEIZURE");
    }
    
    return seizure;
}
