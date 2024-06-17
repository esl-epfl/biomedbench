/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////////
// Author:          Stefano Albini                  //
// Contributions:   Dimitrios Samakovlis            //
// Date:            September 2023                  //
//////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <main.h>
#include <window_definitions.h>
#include <signal_pre_proc.h>
#include <utils_functions.h>
#include <biquad_filter.h>
#include <relativeEnergy.h>
#include <featureExtraction.h>
#include <statisticalFeatureExtraction.h>
#include <powerfeatureExtraction.h>

#ifdef GAPUINO
#include "GAPUINO/gpio_mng.h"
#elif defined(GAP9)
#include "GAP9/peripherals_sleep.h"
#endif

#include "pmsis.h"

// variables to power on the cluster
PI_L2 struct pi_device cluster_dev;
PI_L2 struct pi_cluster_conf cl_conf;
#ifdef PROFILING_ON
#include "profile.h"
#endif

// Some helper to compare the results in the development phase
float output_test[N_FEAT_USED_RF] = {-0.193381, -0.015447, 1.288021, -0.211033, -1.498137, -3.384388, 0.029312, -0.004760, -0.220259, -4.493938, -0.400790, -0.978121, 
            -0.835981, 0.003168, -0.662275, -0.046252, 0.442150, -2.471851};

/* Import file for testing hardcoded data */
#include <W14_1_mod_s1d1_f.h>

PI_L1 _features_t features_eeg;                     // used in feature extraction
PI_L1 my_int features_used[N_FEAT_USED_RF];
PI_L1 my_int procpool[WINDOW_LENGTH];

// Move data from L2 -> L1
void DMATransfer(int *arg)   {
    pi_cl_dma_cmd_t dmaCp;
    pi_cl_dma_cmd((unsigned int)arg[0], (unsigned int)arg[1], arg[2]*sizeof(my_int), PI_CL_DMA_DIR_EXT2LOC, &dmaCp);
    // Wait for dma to finish
    pi_cl_dma_wait(&dmaCp);
}

// Change bit depth in the cluster
ClusterChangeBitDepth(int *arg)    {
    change_bit_depth(arg[0], arg[0], arg[1], arg[2], arg[3]);
}

void eGlass()
{
    uint8_t status = 0; // for the features extraction
    uint8_t out = 0;    // for the classification
    int st;             // for relative energy
    uint8_t npeaks;     // for peaks

    // Ranking indexes of the final features
    int32_t ranking[NUM_FEATURES*N_CHANNEL_USED] = {1,16, 12, 35, 7, 66, 17, 50, 14, 15, 45, 56, 57, 60, 38, 54, 29, 32, 2, 3, 5, 37, 13, 22, 4, 11, 26, 63, 6, 10, 41, 62, 19, 53, 34, 67, 23, 65, 36, 44, 43, 49, 21, 58, 24, 68, 51, 55, 39, 59, 27, 61, 9, 52, 18, 28, 33, 48, 20, 31, 8, 40, 47, 64, 25, 30, 42, 46};

    // Cluster variables
    struct pi_cluster_task cl_task;                 // used to send tasks to the cluster
    int cluster_args[5];                            // used to pass the arguments to the cluster functions

    size_t len = sizeof(Signals_raw_test.signal->signal_chA) / sizeof(Signals_raw_test.signal->signal_chA[0]);
    int32_t num_windows = (int32_t)len / WINDOW_LENGTH;
    int32_t num_half_windows = num_windows * 2;

    uint16_t size_proc_buf = WINDOW_LENGTH / 2;     // number of samples to be used for RelEn and blink removal

    
#ifdef FILTER_ACTIVE
    size_t sos_size = sizeof(sos_filter_arm_biquad) / sizeof(sos_filter_arm_biquad[0]);

    my_int new_sos_filter_second[sos_size];
    convert_to_x(sos_filter_arm_biquad_second, new_sos_filter_second, sos_size, N_DEC_BIQ);

    fixed_t fstate_ch[CH_TO_STORE][SOS_N_SAMPLES_NEEDED];

    filter_instance S[CH_TO_STORE];

    // Initialize the filter instances
    // for all the channels
    for (uint8_t i = 0; i < CH_TO_STORE; i++)
    {
        init_filter(&S[i], SOS_N_BIQUAD, new_sos_filter_second, fstate_ch[i]); // second one
    }
#endif


    cluster_args[0] = (int *)WINDOW_LENGTH;
    cluster_args[1] = (int *)SAMPLING_FREQ;
    cluster_args[2] = (int *)N_BATCHES;
    cluster_args[3] = (int *)N_CHANNEL_USED;
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterPowerFeatureExtractionInit, cluster_args));

    // Init statistical feature extraction
    cluster_args[0] = (int *)N_BATCHES;
    cluster_args[1] = (int *)N_CHANNEL_USED;
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterStatisticalFeatures_Init, cluster_args));


    my_int max = fx_ftox(50.0, N_DEC_STAT);
    my_int min = fx_ftox(-50.0, N_DEC_STAT);
    cluster_args[0] = (int *)N_BINS;
    cluster_args[1] = (int *)min;
    cluster_args[2] = (int *)max;
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterStatisticalFeatures_InitHistogram, cluster_args));
    

#ifdef APPLY_BLINK_REMOVAL
    cluster_args[0] = (int *)WINDOW_LENGTH;
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterrelEn_Init, cluster_args)); // Initialize the relative energy module
#endif

    int i = 0;
    int cnt = 0;

    #ifdef PRINT_INFO
    printf("NUM_HALF_WIND:\t%d\n", num_half_windows);
    #endif

    uint16_t start_half_index = 0; // start index of the half_window used for relEn
    uint16_t start_full_index = 0; // start index of the full_window to be processed when the relEn coeffs are available

    #if defined(GAPUINO) || defined(GAP9)
    gpio_enable();
    #endif

    #ifdef PROFILING_ON
    profile_start();
    #endif
    
    // Simulates the infinite loop. Iterates over halfwindows
    for (;;)
    {
        start_half_index = size_proc_buf * i;

    #ifdef PRINT_INFO
        printf("-----------------%d--------------------\n", cnt);
    #endif

#ifdef APPLY_BLINK_REMOVAL
    #ifdef PRINT_INFO
        printf("[%d - %d)\n", start_half_index, start_half_index + size_proc_buf);
    #endif
        /* Channel A */
        change_bit_depth(&Signals_raw_test.signal->signal_chA[start_half_index], procpool, size_proc_buf, N_DEC_BIQ, N_DEC_REL);

        // find peaks
        cluster_args[0] = (int *)procpool;
        cluster_args[1] = (int *)size_proc_buf;
        cluster_args[2] = (int *)&st;
        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterrelEn, cluster_args));

        if (st == REL_EN_AVAILABLE)
        {
    #ifdef PRINT_INFO
            printf("++++++++++++++++++++\n+ RelEn available! +\n++++++++++++++++++++\n");
    #endif
#else
    #ifdef PRINT_INFO
        printf("[%d - %d)\n", start_full_index, start_full_index + WINDOW_LENGTH);
    #endif
#endif
            /* CHANNEL A */
            // get chA data from the beginning
            
            // get a full window
            // Move data to L1
            cluster_args[0] = (int *) &Signals_raw_test.signal->signal_chA[start_full_index];
            cluster_args[1] = (int *) procpool;
            cluster_args[2] = (int *) WINDOW_LENGTH;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, DMATransfer, cluster_args));
            
            
#ifdef APPLY_BLINK_REMOVAL
            // find peaks
            cluster_args[0] = (int *)relEN_coeff;
            cluster_args[1] = (int *)WINDOW_LENGTH;
            cluster_args[2] = (int *)N_DEC_REL;
            cluster_args[3] = (int *)N_DEC_BLINK;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterChangeBitDepth, cluster_args));

            // find peaks
            cluster_args[0] = (int *)relEN_coeff;
            cluster_args[1] = (int *)WINDOW_LENGTH;
            cluster_args[2] = (int *)&npeaks;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterPreProc_FindPeaks, cluster_args));

            // blink removal
            cluster_args[0] = (int *)procpool;
            cluster_args[1] = (int *)npeaks;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterPreProc_BlinkRemoval, cluster_args));
            
            // Biquad Filter
            cluster_args[0] = (int *)&S[CHA];
            cluster_args[1] = (int *)procpool;
            cluster_args[2] = (int *)procpool;
            cluster_args[3] = (int *)WINDOW_LENGTH;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, Clusterbiquad_filter, cluster_args));

            relEn_SetStatus(REL_EN_START); // restart relEn calculation
#endif

            // Feature extraction
            cluster_args[0] = (int *)features_eeg.feature_chA;
            cluster_args[1] = (int *)procpool;
            cluster_args[2] = (int *)CHA;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterFeatureExtraction, cluster_args));

            /* CHANNEL B */
            // Move data to L1
            cluster_args[0] = (int *) &Signals_raw_test.signal->signal_chB[start_full_index];
            cluster_args[1] = (int *) procpool;
            cluster_args[2] = (int *) WINDOW_LENGTH;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, DMATransfer, cluster_args));

#ifdef APPLY_BLINK_REMOVAL
            // blink removal
            cluster_args[0] = (int *)procpool;
            cluster_args[1] = (int *)npeaks;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterPreProc_BlinkRemoval, cluster_args));

            // Biquad Filter
            cluster_args[0] = (int *)&S[CHB];
            cluster_args[1] = (int *)procpool;
            cluster_args[2] = (int *)procpool;
            cluster_args[3] = (int *)WINDOW_LENGTH;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, Clusterbiquad_filter, cluster_args));  
#endif
            
            // Feature extraction
            cluster_args[0] = (int *)features_eeg.feature_chB;
            cluster_args[1] = (int *)procpool;
            cluster_args[2] = (int *)CHB;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterFeatureExtraction, cluster_args));

            // // /* CHANNEL C */
            // Move data to L1
            cluster_args[0] = (int *) &Signals_raw_test.signal->signal_chC[start_full_index];
            cluster_args[1] = (int *) procpool;
            cluster_args[2] = (int *) WINDOW_LENGTH;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, DMATransfer, cluster_args));
#ifdef APPLY_BLINK_REMOVAL
            // blink removal
            cluster_args[0] = (int *)procpool;
            cluster_args[1] = (int *)npeaks;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterPreProc_BlinkRemoval, cluster_args));

            // Biquad Filter
            cluster_args[0] = (int *)&S[CHC];
            cluster_args[1] = (int *)procpool;
            cluster_args[2] = (int *)procpool;
            cluster_args[3] = (int *)WINDOW_LENGTH;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, Clusterbiquad_filter, cluster_args));
#endif

            // Feature extraction
            cluster_args[0] = (int *)features_eeg.feature_chC;
            cluster_args[1] = (int *)procpool;
            cluster_args[2] = (int *)CHC;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterFeatureExtraction, cluster_args));


        
	        /* *** CHANNEL D *** */
            // Move data to L1
            cluster_args[0] = (int *) &Signals_raw_test.signal->signal_chD[start_full_index];
            cluster_args[1] = (int *) procpool;
            cluster_args[2] = (int *) WINDOW_LENGTH;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, DMATransfer, cluster_args));

        
#ifdef APPLY_BLINK_REMOVAL
            // blink removal
            cluster_args[0] = (int *)procpool;
            cluster_args[1] = (int *)npeaks;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterPreProc_BlinkRemoval, cluster_args));

            // Biquad Filter
            cluster_args[0] = (int *)&S[CHD];
            cluster_args[1] = (int *)procpool;
            cluster_args[2] = (int *)procpool;
            cluster_args[3] = (int *)WINDOW_LENGTH;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, Clusterbiquad_filter, cluster_args));
#endif

            // Feature extraction
            cluster_args[0] = (int *)features_eeg.feature_chD;
            cluster_args[1] = (int *)procpool;
            cluster_args[2] = (int *)CHD;
            cluster_args[3] = (int *)&status;
            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterFeatureExtraction, cluster_args));

            if (status)
            {

#ifdef PRINT_FEAT
                // TEST for checking all the features at once //
                for (int i = 0; i < N_CHANNEL_USED * NUM_FEATURES; i++)
                {
                    if((i%NUM_FEATURES) == 0)
                        printf("\n");
                    printf("f[%d]:\t%f\n", i, fx_xtof(features_eeg.features_all[i], N_DEC_ENTR));
                    
                }
#endif
                
                for (int8_t i = 0; i < N_FEAT_USED_RF; i++)
                {
                    features_used[i] = features_eeg.features_all[ranking[i] - 1] - fx_ftox(mean_baseline[i], N_DEC_PSD);
                    #ifdef PRINT_FEAT_USED
                    printf("%f\t", fx_xtof(features_used[i], N_DEC_PSD));
                    #endif
                }

                cluster_args[0] = (int *)features_used;
                cluster_args[1] = (int *)&out;
                pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, ClusterdecisionTreeFun, cluster_args));
                
#ifdef PRINT_CLASSIFICATION
                printf("\nClassification result: %d\n", out);
#endif
            }

            start_full_index += WINDOW_LENGTH; // Incremented by one full window

            /* Goes until the last sample, then it starts all over again */
            if (start_full_index >= LENGTH)
            {
                start_full_index = 0;
            }
#ifdef APPLY_BLINK_REMOVAL
        }
#endif

        i++;
        if (i >= num_half_windows)
        {
            i = 0;
        }

        cnt++;
        // Stops when N_HALFWIND_TO_PROCESS has been processed
        if (cnt >= N_HALFWIND_TO_PROCESS)
        {
            break;
        }
    }

    // Free memories
#ifdef APPLY_BLINK_REMOVAL
    endRelEnModule();
#endif
    
    endPowerModule();
    endStatsModule((uint8_t)N_BATCHES, (uint8_t)N_BINS);

    // Power down the cluster
    pi_cluster_close(&cluster_dev);

    #if defined(GAPUINO) || defined(GAP9)
    gpio_disable();
    #endif

    #ifdef PROFILING_ON
    profile_stop();
    #endif
    
    #ifdef PRINT_INFO
    printf("\n-----------------\n|    THE END    |\n-----------------\n\n");
    #endif
    
    return;

}

/* Program Entry. */
int main(void)
{
   
    #ifdef GAP9
    setup_frequency_voltage(240000000, 650);
    printf("Geia\n");       // Need this as a delay to get correct measurements (correct voltage/frequency setting)
    #endif
    #if defined(GAP9) || defined(GAPUINO)
    gpio_ini();
    #endif

    #ifdef PRINT_INFO
    printf("\n\n\t *** Cognitive Workload Monitoring fixed-point ***\n\n");
    #endif

    // *** SETUP THE CLUSTER CONFIG AND ENABLE IT ***
    pi_cluster_conf_init(&cl_conf);
    cl_conf.id = 0;                /* Set cluster ID. */
                       // Enable the special icache for the master core
    #ifdef GAP9
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

    #ifdef GAPUINO
    pi_freq_set(PI_FREQ_DOMAIN_CL, 87000000);
    #elif defined(GAP9)
    pi_freq_set(PI_FREQ_DOMAIN_CL, 240000000);
    #endif

    eGlass();

    return 0;
}
