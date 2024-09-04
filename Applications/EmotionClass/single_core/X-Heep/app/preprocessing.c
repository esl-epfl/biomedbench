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

#include "bindi_knn.h"
#include "defines.h"


// Return the average of an array of uint32_t
float average_u32(const uint32_t *data, uint32_t length)  {
    int32_t sum = 0;

    for (int i=0; i<length; i++)   {
        sum += data[i];
    }

    return (float) (sum) /  (float) length;
}


// Return the average of an array of int16_t
float average_s16(const int16_t *data, int16_t length)  {
    // Note: Use int32_t to avoid overflow
    int32_t sum = 0;

    for (int i=0; i<length; i++)   {
        sum += (int32_t) data[i];
    }
    
    return (float) (sum) / (float) length;
}


// Return the average of an array of float
float average_f(const float *data, int16_t length)  {
    float sum = 0;

    for (int i=0; i<length; i++)   {
        sum += data[i];
    }

    return sum / (float) length;
}


// Normalize values and set global kNN test variable
// Return True if values fall within the expected range
// Return Flase if values fall out of the expected range
bool normalize(float  bvp, float gsr, float temp)
{
  /* 0-1 scaling: each variable in the data set is 
   * recalculated as (V - min V)/(max V - min V) */

  /* zerOne BVP */
  if(bvp > (float)MAX_BVP || bvp < (float)MIN_BVP)
    return false;
  
  knn_testing_dataset.field_1 = (bvp - (float)MIN_BVP)/((float)(MAX_BVP-MIN_BVP));

  /* zerOne GSR */
  if(gsr > (float)MAX_GSR || gsr < (float)MIN_GSR)
    return false;

  knn_testing_dataset.field_2 = (gsr - (float)MIN_GSR)/((float)(MAX_GSR-MIN_GSR));

  /* zerOne TEMP */
  if(temp > (float)MAX_TEMP || temp < (float)MIN_TEMP)
    return false;

  knn_testing_dataset.field_3 = (temp - (float)MIN_TEMP)/((float)(MAX_TEMP-MIN_TEMP));

  return true;
}


// Return true if the averages are withing the expected ranges for normalization
// Return false if the average is exceeding the limits
bool preprocess_input(const uint32_t *bvp, const int16_t *gsr, const float *temp)   {

    // Step 1: get the mean of the input series
    float mean_bvp = average_u32(bvp, BVP_SIZE);
    float mean_gsr = average_s16(gsr, GSR_SIZE);
    float mean_temp = average_f(temp, STEMP_SIZE);

    // Step 2: normalize data to (0, 1)
    bool flag = normalize(mean_bvp, mean_gsr, mean_temp);
        
    return flag;
}
