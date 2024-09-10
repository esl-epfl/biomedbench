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
                                                                //
// Date:            February 2024                                                                       //  
//////////////////////////////////////////////////////////////////////////////////////////////////////////



#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "bindi_knn.h"
#include "defines.h"
#include "preprocessing.h"

#include "input_signals.h"

#include "timer_sdk.h"

int main (void) {
    #ifdef PRINT_CYCLES
        timer_init();
        timer_start();
    #endif
    // Note: In this implementation, we run the complete inference (all 10 batches) at once
    // there is no DMA transfer of the input data - input data are in .h files
    int count = 0;
    int temp;
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
            printf("Batch %d --> ERROR: Could not classify because value out of normalization range\n", i + 1);
        else if (temp == 0)
            printf("Batch %d --> NO FEAR\n", i + 1);
        else
            printf("Batch %d --> FEAR\n", i + 1);
        #endif
    }
    #ifdef PRINT_CYCLES
        uint32_t cycles=timer_stop();
        printf("Cycles: %d\n",cycles);
    #endif
    #ifdef PRINTING_RESULT
    printf("*** CLASSIFICATION REPORT *** \n");
    
    if (count > THRESHOLD)
        printf("Result: FEAR!\n");
    else
        printf("Result: NO FEAR!\n");
    
    printf("Total batches: %d\nThreshold: %d\nFear batches:%d\n", WINDOWS, THRESHOLD, count);
    printf("***************************** \n");
    #endif

    return 0;
}
