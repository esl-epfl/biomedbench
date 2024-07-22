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
// Porting to X-Heep: Francesco Poluzzi         //
/////////////////////////////////////////////////



#include "procedure.hpp"
extern "C" {
    #include <stdio.h>
    #include "lib/fixmath.h"
}
#include "output.hpp"

#include "filtering.hpp"
#include "rpeak_del.hpp"
#include "edr.hpp"
#include "rri_features.hpp"
#include "lib/fixpnt_lin_pred_est.hpp"
#include "edr_features.hpp"
#include "svm.hpp"
#include "print_float.hpp"

#include "global_config.hpp"
using namespace fixpnt_32b;


static fixed_t feature[NUM_FEATURES];
static fixed_t edrFreqFeat[EDR_NUM_FREQ_FEATS];
static fixed_t edrLpc[EDR_LPC_ORDER];
static int16_t rPeak[RPEAK_CAPACITY];
static int16_t rri[RPEAK_CAPACITY-1];

bool PredictSeizure(int32_t* ecg, int ecgSize) {

    // Module 0: Filtering (subtract moving average)
    remove_moving_average(ecg, ecgSize, MOVING_AVG_WINDOW);

    // Module 1: R peak delineation
    int rPeakSize;
    DelineateRpeaks<ECG_FREQ>(
        ecg, 
        ecgSize, 
        rPeak, 
        &rPeakSize, 
        RPEAK_CAPACITY, 
        fx_itox(1500, ECG_FRAC)
    );
    int rriSize = rPeakSize - 1;
    for (int i = 0; i < rriSize; ++i) {
        rri[i] = rPeak[i+1] - rPeak[i];
    }
    if (PRINT_DELINEATION) {
        OutputVector(rPeak, rPeakSize, "rPeak");
    }

    // Module 2: ECG derived respiration (EDR).
    fixed_t edr[rPeakSize];
    EcgDerivedRespiration<ECG_FREQ, WIN_SIZE>(
        ecg, ecgSize, rPeak, rPeakSize, edr
    );
    if (PRINT_EDR) OutputFixVector(edr, rPeakSize, "edr", ECG_FRAC);

    // Extraction of RR interval sequence features
    RriFeats rriFeats = ExtractRriFeatures(rri, rriSize);
    if (PRINT_RRI_FAST_FEATURES) Output(rriFeats);

    // Extraction of RR interval frequency features
    fixed_t hrv[rriSize];
    fixed_t rPeakTime[rPeakSize];
    fx_rdiv_t ifreq;
    fx_invx(fx_itox(ECG_FREQ, FLOMB_FRAC), FLOMB_FRAC, &ifreq);
    for (int i = 0; i < rriSize; ++i) {
        hrv[i] = fx_itox(ECG_FREQ, FLOMB_FRAC) / rri[i];
        rPeakTime[i] = fx_rdivx(fx_itox(rPeak[i], FLOMB_FRAC), &ifreq);
    }
    rPeakTime[rPeakSize-1] =
        fx_rdivx(fx_itox(rPeak[rPeakSize-1], FLOMB_FRAC), &ifreq);
    RriFreqFeats rriFreqFeats =
        ExtractRriFreqFeatures(hrv, rPeakTime, rriSize);
    if (PRINT_RRI_FREQ_FEATURES) {
        Output(rriFreqFeats);
    }

    // Module 3: Linear prediction coefficients of the EDR signal
    ComputeLinPredCoeffs(edr, rPeakSize, EDR_LPC_ORDER, edrLpc, LPC_FRAC);
    if (PRINT_EDR_LPC)
        OutputFixVector(edrLpc, EDR_LPC_ORDER, "edrLpc", LPC_FRAC);   
    // Extraction of EDR signal frequency features
    for (int i = 0; i < rPeakSize; ++i) {
        edr[i] = fx_xtox(edr[i], ECG_FRAC, FLOMB_FRAC);
    }
    ExtractEdrFreqFeatures(
        edr, rPeakTime, rPeakSize, edrFreqFeat, EDR_NUM_FREQ_FEATS
    );
    if (PRINT_FREQ_EDR) OutputFixVector(
        edrFreqFeat, EDR_NUM_FREQ_FEATS, "edrFreqFeat", FLOMB_FRAC
    );

    // Module 4: Support Vector Machine
    feature[0] = fx_xtox(rriFeats.stats.mean, RRI_FRAC, SVM_FRAC);
    feature[1] = fx_xtox(rriFeats.stats.stdDev, RRI_FRAC, SVM_FRAC);
    feature[2] = fx_xtox(
        rriFeats.stats.sqrtOrder2Derivative, RRI_FRAC, SVM_FRAC
    );
    feature[3] = fx_xtox(
        rriFeats.stats.percIrregularities, RRI_FRAC, SVM_FRAC
    );
    feature[4] = fx_xtox(rriFreqFeats.totPow, FLOMB_FRAC, SVM_FRAC);
    feature[5] = fx_xtox(rriFreqFeats.lowFreqRatio, FLOMB_FRAC, SVM_FRAC);
    feature[6] = fx_xtox(rriFreqFeats.highFreqRatio, FLOMB_FRAC, SVM_FRAC);
    feature[7] = fx_xtox(rriFreqFeats.lowToHighFreqRatio, FLOMB_FRAC, SVM_FRAC);
    feature[8] = fx_xtox(rriFeats.lorenz.std1, RRI_FRAC, SVM_FRAC);
    feature[9] = fx_xtox(rriFeats.lorenz.std2, RRI_FRAC, SVM_FRAC);
    feature[10] = fx_xtox(rriFeats.lorenz.L, RRI_FRAC, SVM_FRAC);
    feature[11] = fx_xtox(rriFeats.lorenz.T, RRI_FRAC, SVM_FRAC);
    feature[12] = fx_xtox(rriFeats.lorenz.csi, RRI_FRAC, SVM_FRAC);
    feature[13] = fx_xtox(rriFeats.lorenz.modCsi, RRI_FRAC, SVM_FRAC);
    feature[14] = fx_xtox(rriFeats.lorenz.cvi, RRI_FRAC, SVM_FRAC);

    for (int i = 0; i < EDR_NUM_FREQ_FEATS; ++i) {
        feature[15+i] = fx_xtox(edrFreqFeat[i], FLOMB_FRAC, SVM_FRAC);
    }
    for (int i = 0; i < EDR_LPC_ORDER; ++i) {
        feature[15+EDR_NUM_FREQ_FEATS+i] =
            fx_xtox(edrLpc[i], LPC_FRAC, SVM_FRAC);
    }

    if (PRINT_FEATURES) {
        for (int i = 0; i < NUM_FEATURES; ++i) {
            if (FLOAT_OUTPUT) {
                print_float_cpp(fx_xtof(feature[i], SVM_FRAC), 6);
                printf("%c", " \n"[i+1==NUM_FEATURES]);
            } else {
                printf("%d%c", feature[i], " \n"[i+1==NUM_FEATURES]);
            }
        }
    }
    
    fixed_t out = predict(feature);
    bool seizure = out > 0;
    if (PRINT_SVM) {
        OutputFixVector(&out, 1, "svm", SVM_FRAC);
    }
    if (PRINT_PREDICTION) {
        printf("Prediction: %s\n", seizure? "SEIZURE" : "NON_SEIZURE");
    }
    
    return seizure;
}