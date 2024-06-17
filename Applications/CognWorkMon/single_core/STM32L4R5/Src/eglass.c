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

#include "eglass.h"

#ifdef DATA_ACQUISITION
#include "main.h"
#endif

/* Import file for testing hardcoded data */
#include <W14_1_mod_s1d1_f.h>


extern _features_t features_eeg;

#ifndef DATA_ACQUISITION
int eglass()
{

    uint8_t status = 0; // for the features extraction
    uint8_t out = 0;    // for the classification

    // Ranking indexes of the final features
    int32_t ranking[NUM_FEATURES * N_CHANNEL_USED] = {1, 16, 12, 35, 7, 66, 17, 50, 14, 15, 45, 56, 57, 60, 38, 54, 29, 32, 2, 3, 5, 37, 13, 22, 4, 11, 26, 63, 6, 10, 41, 62, 19, 53, 34, 67, 23, 65, 36, 44, 43, 49, 21, 58, 24, 68, 51, 55, 39, 59, 27, 61, 9, 52, 18, 28, 33, 48, 20, 31, 8, 40, 47, 64, 25, 30, 42, 46};

#ifndef ONLY_CLASSIFICATION
    size_t len = sizeof(Signals_raw_test.signal->signal_chA) / sizeof(Signals_raw_test.signal->signal_chA[0]);
    int32_t num_windows = (int32_t)len / WINDOW_LENGTH;
    int32_t num_half_windows = num_windows * 2;

    uint16_t size_proc_buf = WINDOW_LENGTH / 2; // number of samples to be used for RelEn and blink removal
    my_int *procpool = (my_int *)pvPortMalloc(WINDOW_LENGTH * sizeof(my_int));

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

    uint8_t npeaks;

    PowerFeatureExtractionInit(WINDOW_LENGTH, SAMPLING_FREQ, N_BATCHES, N_CHANNEL_USED);

    // Init statistical feature extraction
    if (StatisticalFeatures_Init(N_BATCHES, 4) == 0)
    {
        printf("ERROR STATS INIT!\n");
    }

    // Init statistical feature histogram

    my_int max = fx_ftox(50.0, N_DEC_STAT);
    my_int min = fx_ftox(-50.0, N_DEC_STAT);
    StatisticalFeatures_InitHistogram(20, min, max);

#ifdef APPLY_BLINK_REMOVAL
    relEn_Init(WINDOW_LENGTH); // Initialize the relative energy module
#endif
    int i = 0;
    int cnt = 0;

#ifdef PRINT_INFO
    printf("NUM_HALF_WIND:\t%d\n", num_half_windows);
#endif

    uint16_t start_half_index = 0; // start index of the half_window used for relEn
    uint16_t start_full_index = 0; // start index of the full_window to be processed when the relEn coeffs are available


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

        /* Harcoded float data from signal_chb08.h*/
        change_bit_depth(&Signals_raw_test.signal->signal_chA[start_half_index], procpool, size_proc_buf, N_DEC_BIQ, N_DEC_REL);

        int st = relEn(procpool, size_proc_buf);
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
            vect_copy(&Signals_raw_test.signal->signal_chA[start_full_index], procpool, WINDOW_LENGTH);

#ifdef APPLY_BLINK_REMOVAL
            change_bit_depth(relEN_coeff, relEN_coeff, WINDOW_LENGTH, N_DEC_REL, N_DEC_BLINK);

            // find peaks
            npeaks = PreProc_FindPeaks(relEN_coeff, WINDOW_LENGTH);

            // blink removal
            PreProc_BlinkRemoval(procpool, npeaks); // NO DEVI PRIMA CONVERTIRE TUTTO CON LO STESSO NUMERO DI BITS!!!

            biquad_filter(&S[CHA], procpool, procpool, WINDOW_LENGTH);

            relEn_SetStatus(REL_EN_START); // restart relEn calculation
#endif
            status = FeatureExtraction(features_eeg.feature_chA, procpool, CHA);

            /* CHANNEL B */
            vect_copy(&Signals_raw_test.signal->signal_chB[start_full_index], procpool, WINDOW_LENGTH);
#ifdef APPLY_BLINK_REMOVAL
            PreProc_BlinkRemoval(procpool, npeaks);
            biquad_filter(&S[CHB], procpool, procpool, WINDOW_LENGTH);
#endif
            status = FeatureExtraction(features_eeg.feature_chB, procpool, CHB);

            // // /* CHANNEL C */
            vect_copy(&Signals_raw_test.signal->signal_chC[start_full_index], procpool, WINDOW_LENGTH);
#ifdef APPLY_BLINK_REMOVAL
            PreProc_BlinkRemoval(procpool, npeaks);
            biquad_filter(&S[CHC], procpool, procpool, WINDOW_LENGTH);
#endif
            status = FeatureExtraction(features_eeg.feature_chC, procpool, CHC);

            // // /* CHANNEL D */
            vect_copy(&Signals_raw_test.signal->signal_chD[start_full_index], procpool, WINDOW_LENGTH);

#ifdef APPLY_BLINK_REMOVAL
            PreProc_BlinkRemoval(procpool, npeaks);
            biquad_filter(&S[CHD], procpool, procpool, WINDOW_LENGTH);
#endif
            status = FeatureExtraction(features_eeg.feature_chD, procpool, CHD);

            if (status)
            {

#ifdef PRINT_FEAT
            	float *features_temp = (my_int *)pvPortMalloc(N_CHANNEL_USED * NUM_FEATURES * sizeof(float));

                for (int i = 0; i < N_CHANNEL_USED * NUM_FEATURES; i++)
                {
                    if ((i % NUM_FEATURES) == 0)
                        printf("\n");
                    printf("f[%d]:\t%f\n", i, fx_xtof(features_eeg.features_all[i], N_DEC_ENTR));
                    features_temp[i] = fx_xtof(features_eeg.features_all[i], N_DEC_ENTR);
                }
#endif
                my_int *features_used = (my_int *)pvPortMalloc(N_FEAT_USED_RF * sizeof(my_int));

                my_int mean, std;
                for (int8_t i = 0; i < N_FEAT_USED_RF; i++)
                {
                    features_used[i] = features_eeg.features_all[ranking[i] - 1] - fx_ftox(mean_baseline[i], N_DEC_PSD);
                }

                out = decisionTreeFun(features_used);

			#ifdef PRINT_CLASSIFICATION
                printf("\nClassification result: %d\n", out);
            #endif

                vPortFree(features_used);
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
    vPortFree(procpool);
#ifdef APPLY_BLINK_REMOVAL
    endRelEnModule();
#endif
    endPowerModule();
    endStatsModule();

#endif

#ifdef PRINT_INFO
    printf("\n-----------------\n|    THE END    |\n-----------------\n\n");
#endif

    return 1;
}
#else
my_int *procpool;
filter_instance S[CH_TO_STORE];
void eglass_init()
{

#ifndef ONLY_CLASSIFICATION
    procpool = (my_int *)pvPortMalloc(WINDOW_LENGTH * sizeof(my_int));

#ifdef FILTER_ACTIVE
    size_t sos_size = sizeof(sos_filter_arm_biquad) / sizeof(sos_filter_arm_biquad[0]);
    my_int new_sos_filter_second[sos_size];
    convert_to_x(sos_filter_arm_biquad_second, new_sos_filter_second, sos_size, N_DEC_BIQ);

    fixed_t fstate_ch[CH_TO_STORE][SOS_N_SAMPLES_NEEDED];

    // Initialize the filter instances

    // for all the channels
    for (uint8_t i = 0; i < CH_TO_STORE; i++)
    {
        init_filter(&S[i], SOS_N_BIQUAD, new_sos_filter_second, fstate_ch[i]); // second one
    }
#endif

    PowerFeatureExtractionInit(WINDOW_LENGTH, SAMPLING_FREQ, N_BATCHES, N_CHANNEL_USED);

    // Init statistical feature extraction
    if (StatisticalFeatures_Init(N_BATCHES, 4) == 0)
    {
        printf("ERROR STATS INIT!\n");
    }

    // Init statistical feature histogram
    my_int max = fx_ftox(50.0, N_DEC_STAT);
    my_int min = fx_ftox(-50.0, N_DEC_STAT);
    StatisticalFeatures_InitHistogram(20, min, max);

#ifdef APPLY_BLINK_REMOVAL
    relEn_Init(WINDOW_LENGTH); // Initialize the relative energy module
#endif

#endif

#ifdef PRINT_INFO
    printf("\n-----------------\n|    THE END    |\n-----------------\n\n");
#endif

}


// Process the 4 second window received
int eglass_process()	{
	static int cnt = -1;
	uint8_t npeaks;
    uint8_t status = 0; // for the features extraction
    uint8_t out = -1;    // for the classification
    // Ranking indexes of the final features
    int32_t ranking[NUM_FEATURES * N_CHANNEL_USED] = {1, 16, 12, 35, 7, 66, 17, 50, 14, 15, 45, 56, 57, 60, 38, 54, 29, 32, 2, 3, 5, 37, 13, 22, 4, 11, 26, 63, 6, 10, 41, 62, 19, 53, 34, 67, 23, 65, 36, 44, 43, 49, 21, 58, 24, 68, 51, 55, 39, 59, 27, 61, 9, 52, 18, 28, 33, 48, 20, 31, 8, 40, 47, 64, 25, 30, 42, 46};
    uint16_t size_proc_buf = WINDOW_LENGTH / 2; // number of samples to be used for RelEn and blink removal

	cnt++;
	for (int index = 0; index < 2; index++)	{
	int start_half_index = size_proc_buf * index;


	#ifdef PRINT_INFO
	        printf("-----------------%d--------------------\n", cnt);
	#endif

	#ifdef APPLY_BLINK_REMOVAL
	#ifdef PRINT_INFO
	        printf("[%d - %d)\n", start_half_index, start_half_index + size_proc_buf);
	#endif
	        /* Channel A */

	        /* Harcoded float data from signal_chb08.h*/
	        change_bit_depth(&channelA[start_half_index], procpool, size_proc_buf, N_DEC_BIQ, N_DEC_REL);

	        int st = relEn(procpool, size_proc_buf);
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
	            vect_copy(channelA, procpool, WINDOW_LENGTH);

	#ifdef APPLY_BLINK_REMOVAL
	            change_bit_depth(relEN_coeff, relEN_coeff, WINDOW_LENGTH, N_DEC_REL, N_DEC_BLINK);

	            // find peaks
	            npeaks = PreProc_FindPeaks(relEN_coeff, WINDOW_LENGTH);

	            // blink removal
	            PreProc_BlinkRemoval(procpool, npeaks); // NO DEVI PRIMA CONVERTIRE TUTTO CON LO STESSO NUMERO DI BITS!!!

	            biquad_filter(&S[CHA], procpool, procpool, WINDOW_LENGTH);

	            relEn_SetStatus(REL_EN_START); // restart relEn calculation
	#endif
	            status = FeatureExtraction(features_eeg.feature_chA, procpool, CHA);

	            /* CHANNEL B */
	            vect_copy(channelB, procpool, WINDOW_LENGTH);

	#ifdef APPLY_BLINK_REMOVAL
	            PreProc_BlinkRemoval(procpool, npeaks);
	            biquad_filter(&S[CHB], procpool, procpool, WINDOW_LENGTH);
	#endif
	            status = FeatureExtraction(features_eeg.feature_chB, procpool, CHB);

	            // // /* CHANNEL C */
	            vect_copy(channelC, procpool, WINDOW_LENGTH);

	#ifdef APPLY_BLINK_REMOVAL
	            PreProc_BlinkRemoval(procpool, npeaks);
	            biquad_filter(&S[CHC], procpool, procpool, WINDOW_LENGTH);
	#endif
	            status = FeatureExtraction(features_eeg.feature_chC, procpool, CHC);

	            // // /* CHANNEL D */
	            vect_copy(channelD, procpool, WINDOW_LENGTH);

	#ifdef APPLY_BLINK_REMOVAL
	            PreProc_BlinkRemoval(procpool, npeaks);
	            biquad_filter(&S[CHD], procpool, procpool, WINDOW_LENGTH);
	#endif
	            status = FeatureExtraction(features_eeg.feature_chD, procpool, CHD);

	            if (status)
	            {

	#ifdef PRINT_FEAT
	            	float *features_temp = (my_int *)pvPortMalloc(N_CHANNEL_USED * NUM_FEATURES * sizeof(float));

	                for (int i = 0; i < N_CHANNEL_USED * NUM_FEATURES; i++)
	                {
	                    if ((i % NUM_FEATURES) == 0)
	                        printf("\n");
	                    printf("f[%d]:\t%f\n", i, fx_xtof(features_eeg.features_all[i], N_DEC_ENTR));
	                    features_temp[i] = fx_xtof(features_eeg.features_all[i], N_DEC_ENTR);
	                }
	#endif
	                my_int *features_used = (my_int *)pvPortMalloc(N_FEAT_USED_RF * sizeof(my_int));

	                my_int mean, std;
	                for (int8_t i = 0; i < N_FEAT_USED_RF; i++)
	                {
	                    features_used[i] = features_eeg.features_all[ranking[i] - 1] - fx_ftox(mean_baseline[i], N_DEC_PSD);
	                }

	                out = decisionTreeFun(features_used);

				#ifdef PRINT_CLASSIFICATION
	                printf("\nClassification result: %d\n", out);
	            #endif

	                vPortFree(features_used);
	            }

	#ifdef APPLY_BLINK_REMOVAL
	        }
	#endif
}
	if (cnt >= 13)
	 {
	      cnt = -1;
	 }

	return out;
}

#endif
