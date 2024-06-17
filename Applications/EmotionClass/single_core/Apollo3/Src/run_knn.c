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

#include "am_util.h"

#include "run_knn.h"
#include "bindi_knn.h"
#include "defines.h"
#include "preprocessing.h"

#ifndef DATA_ACQUISITION
#include "input_signals.h"
#else
int32_t input_buffer[BUFFER_SIZE];
#endif


int run_knn (void) {

    // Note: In this implementation, we run the complete inference (all 10 batches) at once
    // there is no DMA transfer of the input data - input data are in .h file
    #ifndef DATA_ACQUISITION
    int count = 0;
    int temp;
    for (int i=0; i<WINDOWS; i++)   {
        if (preprocess_input(bvp_sensor[i], gsr_sensor[i], temp_sensor[i])) {
            temp = runKNN();
            count += temp;
        }
        else    {
            temp = -1;
        }

        #ifdef PRINTING_DETAILS
        if (temp == -1)
            am_util_stdio_printf("Batch %d --> ERROR: Could not classify because value out of normalization range\n\n", i);
        else if (temp == 0)
            am_util_stdio_printf("Batch %d --> NO FEAR\n\n", i);
        else
            am_util_stdio_printf("Batch %d --> FEAR\n\n", i);
        #endif
    }

    #ifdef PRINTING_RESULT
    am_util_stdio_printf("*** CLASSIFICATION REPORT *** \n");
    
    if (count > THRESHOLD)
        am_util_stdio_printf("Result: FEAR!\n");
    else
        am_util_stdio_printf("Result: NO FEAR!\n");
    
    am_util_stdio_printf("Total batches: %d\nThreshold: %d\nFear batches:%d\n", WINDOWS, THRESHOLD, count);
    am_util_stdio_printf("***************************** \n");
    #endif

    // ToDO: Implement real-time SPI receival of the input data and run in batches every ~4 seconds
    #else

    #endif

    return 0;
}
