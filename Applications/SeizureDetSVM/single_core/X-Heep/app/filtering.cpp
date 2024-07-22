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



#include "filtering.hpp"
#include "global_config.hpp"
#include <cstdint>

extern "C"  {
    #include "fixmath.h"
}

// Remove the moving average of window "window_length" from the current signal
void remove_moving_average(int32_t *data, int length, int32_t window_length)   {

    int32_t *new_data = (int32_t *) malloc(length * sizeof(int32_t));

    // Calculate moving average
    int32_t sum = 0;
    for (int i = 0; i < length; i++)  {
        sum += data[i];
        if (i - window_length >= 0)
            sum -= data[i - window_length];
        
        new_data[i] = sum / window_length;
    }

    // Copy values back to original array
    for (int i = 0; i < length; i++)    {
        data[i] = data[i] - new_data[i];
    }

    free(new_data);
}