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


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Original app:    Bindi from Jose A. Miranda Calero,  Universidad Carlos III de Madrid                //
// Modified by:     Dimitrios Samakovlis, Embedded Systems Laboratory (ESL), EPFL                       //
// Date:            February 2024                                                                       //  
//////////////////////////////////////////////////////////////////////////////////////////////////////////



#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "bindi_knn.h"
#include "defines.h"
#include "preprocessing.h"

#ifdef GAPUINO
#include "GAPuino/gpio_mgmt.h"
#elif defined(GAP9)
#include "GAP9/peripherals_sleep.h"
#endif

#ifndef DATA_ACQUISITION
#include "input_signals.h"
#else
PI_L2 int32_t input_buffer[BUFFER_SIZE];
#endif

unsigned int L2_accesses = 0;

int main (void) {

    #ifdef GAP9
    setup_frequency_voltage(240000000, 650);
    printf("Geia\n");       // Need this as a delay to get correct measurements (correct voltage/frequency setting)
    #endif
    #if defined(GAPUINO) || defined(GAP9)
    gpio_ini();
    gpio_enable(); //enable GPIO signal for accurate measurements
    #endif

    // Note: In this implementation, we run the complete inference (all 10 batches) at once
    // there is no DMA transfer of the input data - input data are in .h file
    #ifndef DATA_ACQUISITION
    int count = 0;
    int temp;

    #ifdef PROFILE_ALL
    profile_start();
    #endif

    for (int i=0; i<WINDOWS; i++)   {
        if (preprocess_input(bvp_sensor[i], gsr_sensor[i], temp_sensor[i])) {
            #ifdef PRINTING_DETAILS
            printf("\n*** Batch %d ***\n", i + 1);
            #endif
            temp = runKNN();
            count += temp;
        }
        else    {
            temp = -1;
        }

        #ifdef PRINTING_DETAILS
        if (temp == -1)
            printf("Batch %d --> ERROR: Could not classify because value out of normalization range\n\n", i + 1);
        else if (temp == 0)
            printf("Batch %d --> NO FEAR\n\n", i + 1);
        else
            printf("Batch %d --> FEAR\n\n", i + 1);
        #endif
    }

    #ifdef PROFILE_ALL
    profile_stop();
    #endif

    #ifdef PRINTING_RESULT
    printf("*** CLASSIFICATION REPORT *** \n");
    
    if (count > THRESHOLD)
        printf("Result: FEAR!\n");
    else
        printf("Result: NO FEAR!\n");
    
    printf("Total batches: %d\nThreshold: %d\nFear batches:%d\n", WINDOWS, THRESHOLD, count);
    printf("***************************** \n");
    #ifdef COUNT_L2_ACCESS
    printf("L2 loads: %d\n", L2_accesses);
    #endif
    #endif

    // ToDO: Implement real-time SPI receival of the input data and run in batches every ~4 seconds
    #else

    #endif

    #if defined(GAPUINO) || defined(GAP9)
    gpio_disable(); // disable GPIO to end measurements
    #endif

    return 0;
}
