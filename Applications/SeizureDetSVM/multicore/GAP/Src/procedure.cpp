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
    #include "fixmath.h"
    #include <string.h>
}
#include "global_config.hpp"

#include "filtering.hpp"
#include "output.hpp"
#include "profile.hpp"
#include "rpeak_del.hpp"
#include "edr.hpp"
#include "rri_features.hpp"
#include "fixpnt_lin_pred_est.hpp"
#include "edr_features.hpp"
#include "svm.hpp"
#include "svm.inc"
using namespace fixpnt_32b;

// those are used for the API used for the cluster
PI_L2 int32_t *arg[12];		// we use this to pass arguments to cluster functions
PI_L2 struct pi_cluster_conf cl_conf;
PI_L2 struct pi_device cluster_dev;

// data used by the cluster functions
PI_L1 static fixed_t feature[NUM_FEATURES];
PI_L1 static int16_t *rPeak;
PI_L1 static int16_t *rri;
PI_L1 RriFeats rriFeats;
PI_L1 int32_t *ecg;
PI_L1 fixed_t edr[MAX_RPEAKS_PER_CORE * NUM_CORES];
PI_L1 fixed_t hrv[MAX_RPEAKS_PER_CORE * NUM_CORES];
PI_L1 fixed_t rPeakTime[MAX_RPEAKS_PER_CORE * NUM_CORES];
PI_L1 fx_rdiv_t ifreq;
PI_L1 RriFreqFeats rriFreqFeats;
PI_L1 static fixed_t edrFreqFeat[EDR_NUM_FREQ_FEATS];
PI_L1 fixed_t out;
PI_L1 static int32_t edrLpc[EDR_LPC_ORDER];
PI_L1 int32_t svm, bias;
PI_L1 int32_t *svmVect, *alpha, *scaleFeat, *meanFeat;


// Profiling of cluster cores
static void prof_cluster_start(void *arg)
{
  pi_cl_team_fork(NUM_CORES, cl_profile_start, NULL);
}

static void prof_cluster_stop(void *arg)
{
  pi_cl_team_fork(NUM_CORES, cl_profile_stop, NULL);
}

// DMA transfer of ECG signal from L2 to L1
static void fCore0_DmaTransfer(int32_t **arg)	{
    pi_cl_dma_cmd_t dmaCp;
    
    pi_cl_dma_cmd((unsigned int)arg[0], (unsigned int)arg[1], *arg[2], PI_CL_DMA_DIR_EXT2LOC, &dmaCp);
    // Wait for dma to finish
    pi_cl_dma_cmd_wait(&dmaCp);
}


// Intermediate functions of all modules for calling the cluster cores
static void DelineateRpeaks_fork(int32_t **arg)	{
    pi_cl_team_fork(NUM_CORES, DelineateRpeaks, arg);
}

static void EcgDerivedRespiration_fork(int32_t **arg)	{
    pi_cl_team_fork(NUM_CORES, EcgDerivedRespiration, arg);
}

static void ExtractRriFeatures_fork(int32_t **arg)	{
    pi_cl_team_fork(NUM_CORES, ExtractRriFeatures, arg);
}

static void ExtractRriFreqFeatures_fork(int32_t **arg)	{
    pi_cl_team_fork(NUM_CORES, ExtractRriFreqFeatures, arg);
}

static void ExtractEdrFreqFeatures_fork(int32_t **arg)	{
    pi_cl_team_fork(NUM_CORES, ExtractEdrFreqFeatures, arg);
}

static void ComputeLinPredCoeffs_fork(int32_t **arg)	{
    pi_cl_team_fork(NUM_CORES, ComputeLinPredCoeffs, arg);
}

static void predict_fork(int32_t **arg)	{
    pi_cl_team_fork(NUM_CORES, predict, arg);
}


// Base function for the Fabric Controller that handles the data flow, activates and calls the cluster with the correct parameters 
bool PredictSeizure(int32_t* ecg_L2) {
    
	// Module 0: Filtering (subtract moving average)
    // Convert to new fixed-point representation
    // Keep in mind input data are 16-bit with 4 bits of decimal part
    for (int i = 0; i < WIN_SIZE; ++i) {
        ecg_L2[i] = fx_xtox(ecg_L2[i], ADC_FRAC, ECG_FRAC);
    }
    remove_moving_average(ecg_L2, WIN_SIZE, MOVING_AVG_WINDOW);

    struct pi_cluster_task cl_task;
    
    // *** SETUP THE CLUSTER CONFIG AND ENABLE IT ***
    pi_cluster_conf_init(&cl_conf);
    cl_conf.id = 0;                /* Set cluster ID. */
    #ifdef GAP9
                       // Enable the special icache for the master core
    cl_conf.icache_conf = PI_CLUSTER_MASTER_CORE_ICACHE_ENABLE |   
                       // Enable the prefetch for all the cores, it's a 9bits mask (from bit 2 to bit 10), each bit correspond to 1 core
                       PI_CLUSTER_ICACHE_PREFETCH_ENABLE |      
                       // Enable the icache for all the cores
                       PI_CLUSTER_ICACHE_ENABLE;
    #endif      
    
    /* Configure & open cluster. */
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        printf("Cluster open failed !\n");
        pmsis_exit(-1);
    }
    #ifdef GAP9
    pi_freq_set(PI_FREQ_DOMAIN_CL, 240000000);
    #elif defined(GAPUINO)
    pi_freq_set(PI_FREQ_DOMAIN_CL, 87000000);
    #endif
    
    // allocate L1 arrays (need to free later due to L1 size limitations)
    ecg = (int32_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int32_t) * WIN_SIZE);
    rPeak = (int16_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int16_t) * RPEAK_CAPACITY);
    rri = (int16_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int16_t) * (RPEAK_CAPACITY-1));

    if (ecg == NULL || rPeak == NULL || rri == NULL)    {
        printf("L1 memory allocation error\n");
        pmsis_exit(-1);
    }
    
    int32_t size = WIN_SIZE * sizeof(int32_t);
    
    arg[0] = ecg_L2;
    arg[1] = ecg;
    arg[2] = &size;
    
    // DATA PREPARATION: DMA transfer
#if defined(CLUSTER_PROFILE_ON) || defined(CLUSTER_PROFILE_FULL)
    printf("\nDMA transfer\n");
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, fCore0_DmaTransfer, arg));
    
#ifdef CLUSTER_PROFILE_ON
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif

    // MODULE 1: R peak delineation
#ifdef CLUSTER_PROFILE_ON
    printf("\nRpeaks\n");
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif
    int rPeakSize;
    int32_t temp = fx_itox(1500, ECG_FRAC);
    size = WIN_SIZE;
    arg[0] = ecg;
    arg[1] = &size;
    arg[2] = (int32_t *) rPeak;
    arg[3] = (int32_t *) &rPeakSize;
    arg[4] = (int32_t *) rri;
    arg[5] = &temp;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, DelineateRpeaks_fork, arg));

#ifdef CLUSTER_PROFILE_ON
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif

    // ToDO: Find small bug when using 1 core
    // This is a scrappy fix
    if (NUM_CORES == 1)
        rPeakSize--;
        
    if (PRINT_DELINEATION) {
        OutputVector(rPeak, rPeakSize, "rPeak");
    }
   
   

    // MODULE 2: ECG derived respiration (EDR).
#ifdef CLUSTER_PROFILE_ON
    printf("\nEDR\n");
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif
    
    arg[5] = (int32_t *) edr;

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, EcgDerivedRespiration_fork, arg));

#ifdef CLUSTER_PROFILE_ON
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif
    
    if (PRINT_EDR) OutputFixVector(edr, rPeakSize, "edr", ECG_FRAC);

    // MUST FREE ecg data to make some space in L1
    pi_cl_l1_free(&cluster_dev, (void *) ecg, sizeof(int32_t) * WIN_SIZE);
    
    // Extraction of RR interval sequence features
#ifdef CLUSTER_PROFILE_ON
    printf("\nRRI sequence\n");
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif
    
    int rriSize = rPeakSize - 1;
    arg[6] = (int32_t *) &rriSize;
    arg[7] = (int32_t *) &rriFeats;
    arg[8] = (int32_t *) feature;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ExtractRriFeatures_fork, arg));

#ifdef CLUSTER_PROFILE_ON
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif
    
    if (PRINT_RRI_FAST_FEATURES) Output(rriFeats);
    
    // Extraction of RR interval frequency features
#ifdef CLUSTER_PROFILE_ON
    printf("\nRRI frequency\n");
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif

    arg[7] = (int32_t *) &rriFreqFeats;
    arg[9] = (int32_t *) hrv;
    arg[10] = (int32_t *) rPeakTime;
    arg[11] = (int32_t *) &ifreq;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ExtractRriFreqFeatures_fork, arg));

#ifdef CLUSTER_PROFILE_ON
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif
    
    if (PRINT_RRI_FREQ_FEATURES) {
        Output(rriFreqFeats);
    }
    
    pi_cl_l1_free(&cluster_dev, (void *) rPeak, sizeof(int16_t) * RPEAK_CAPACITY);
    pi_cl_l1_free(&cluster_dev, (void *) rri, sizeof(int16_t) * (RPEAK_CAPACITY-1));
    

    
    // MODULE 3: Linear prediction coefficients of the EDR signal
#ifdef CLUSTER_PROFILE_ON
    printf("\nLinear Pred Coeffs\n");
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif
    
    arg[9] = edrLpc;

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ComputeLinPredCoeffs_fork, arg));

#ifdef CLUSTER_PROFILE_ON
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif
    
    if (PRINT_EDR_LPC)
        OutputFixVector(edrLpc, EDR_LPC_ORDER, "edrLpc", LPC_FRAC);

    // MODULE 3b: Extraction of EDR signal frequency features
#ifdef CLUSTER_PROFILE_ON
    printf("\nEDR frequency\n");
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif
    
    arg[11] = (int32_t *) edrFreqFeat;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ExtractEdrFreqFeatures_fork, arg));

#ifdef CLUSTER_PROFILE_ON
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif
    
    if (PRINT_FREQ_EDR) OutputFixVector(
        edrFreqFeat, EDR_NUM_FREQ_FEATS, "edrFreqFeat", FLOMB_FRAC
    );
    
    if (PRINT_FEATURES) {
#ifdef SERIAL_AVAILABLE
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
#endif
    }
    
    
    
    // MODULE 4: Support Vector Machine
#ifdef CLUSTER_PROFILE_ON
    printf("\nSVM\n");
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif
    
    alpha = (int32_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int32_t) * SVM_SIZE);
    svmVect = (int32_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int32_t) * SVM_SIZE * NUM_FEATURES);
    scaleFeat = (int32_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int32_t) * NUM_FEATURES);
    meanFeat = (int32_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int32_t) * NUM_FEATURES);
    
    if (alpha == NULL || svmVect == NULL || scaleFeat == NULL || meanFeat == NULL)    {
        printf("L1 memory allocation error\n");
        pmsis_exit(-1);
    }

    size = SVM_SIZE * sizeof(int32_t);
    
    arg[0] = alpha_L2;
    arg[1] = alpha;
    arg[2] = &size;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, fCore0_DmaTransfer, arg));
    size = NUM_FEATURES * sizeof(int32_t);
    
    arg[0] = scaleFeat_L2;
    arg[1] = scaleFeat;
    arg[2] = &size;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, fCore0_DmaTransfer, arg));
    size = NUM_FEATURES * sizeof(int32_t);
    
    arg[0] = meanFeat_L2;
    arg[1] = meanFeat;
    arg[2] = &size;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, fCore0_DmaTransfer, arg));
    size = NUM_FEATURES * SVM_SIZE * sizeof(int32_t);
    
    arg[0] = svmVect_L2[0];
    arg[1] = svmVect;
    arg[2] = &size;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, fCore0_DmaTransfer, arg));
    
    bias = bias_L2;
    
    arg[11] = (int32_t *)&out;
    arg[0] = svmVect;
    arg[1] = alpha;
    arg[2] = scaleFeat;
    arg[3] = meanFeat;
    arg[4] = &bias;
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, predict_fork, arg));
    
    bool seizure = out > 0;

#if defined(CLUSTER_PROFILE_ON) || defined(CLUSTER_PROFILE_FULL)
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif
    
    if (PRINT_SVM) {
        OutputFixVector(&out, 1, "svm", SVM_FRAC);
    }
    
    if (PRINT_PREDICTION) {
#ifdef SERIAL_AVAILABLE
        printf("Prediction: %s\n", seizure? "SEIZURE" : "NON_SEIZURE");
#endif
    }
    
    pi_cl_l1_free(&cluster_dev, (void *) alpha, sizeof(int32_t) * SVM_SIZE);
    pi_cl_l1_free(&cluster_dev, (void *) svmVect, sizeof(int32_t) * SVM_SIZE * NUM_FEATURES);
    pi_cl_l1_free(&cluster_dev, (void *) scaleFeat, sizeof(int32_t) * NUM_FEATURES);
    pi_cl_l1_free(&cluster_dev, (void *) meanFeat, sizeof(int32_t) * NUM_FEATURES);
    
    pi_cluster_close(&cluster_dev);	// power off the cluster until the next window to process 
    
    return seizure;
}
