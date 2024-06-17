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




#include <cmath>
#include "global_config.hpp"
#include "utils.hpp"

extern "C"  {
#include <stdint.h>
#include "fixmath.h"
}

// DIMITRIOS SAMAKOVLIS PARALLELIZATION	
PI_L1 int32_t *woBaseline;
PI_L1 int rPeak_sync; 			// used for dynamic scheduling sync

void EcgDerivedRespiration(
    const int32_t **arg
) {
    int32_t *ecg = arg[0];
    int ecgSize = (int)*arg[1];
    const int16_t *rPeak = (int16_t *)arg[2];
    int *rPeakSize = (int *)arg[3];
    fixed_t* edr = (fixed_t *)arg[5];
    
    constexpr int WIN0 = std::ceil(0.2*ECG_FREQ);
    constexpr int WIN1 = std::ceil(0.6*ECG_FREQ);
    constexpr int INTEGRAL_RADIUS = std::ceil(0.1*ECG_FREQ);
    
    int id = pi_core_id();
    
    // *******************************************************************
    // IMPORTANT TO DYNAMICALLY ALLOCATE THIS MATRIX BECAUSE OF SIZE
    if (id == 0)	{
	woBaseline = (int32_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int32_t) * WIN_SIZE);
    }
    pi_cl_team_barrier();
    
    // Dimitrios Samakovlis Parallelization
    // Merge all steps in 1 function
    int32_t sum = 0;
    int i = 0;
    
    // classic static scheduling partition
    register int chunk = ecgSize / NUM_CORES;
    register int start_index = id * chunk;
    if (id == NUM_CORES - 1)
	chunk += ecgSize % NUM_CORES;
    
    int32_t *arr = ecg + (int32_t)start_index;
    
    // filt_1
    for (; i < WIN0; ++i) {
        sum += arr[i];
    }
    if (id != NUM_CORES - 1)	{
	for (; i < chunk + WIN0; ++i) {
	    woBaseline[i + start_index - WIN0] = sum/WIN0;
	    sum += arr[i] - arr[i - WIN0];
	}
    }
    else	{
	for (; i < chunk; ++i) {
	    woBaseline[i + start_index - WIN0] = sum/WIN0;
	    sum += arr[i] - arr[i - WIN0];
	}
	for (i -= WIN0; i < chunk; ++i) {
	    woBaseline[i + start_index] = sum/(ecgSize-i-start_index);
	    sum -= arr[i];
	}
    }

    pi_cl_team_barrier();
    
    //filt_2
    // write back on woBaseline_tmp because of sequential dependencies
    i = start_index;
    sum = 0;
    for (; i < start_index + WIN1; ++i) {
        sum += woBaseline[i];
    }
    
    if (id != NUM_CORES - 1)	{
	for (; i < chunk + start_index + WIN1; ++i) {
	    int32_t aux = woBaseline[i - WIN1];
	    ecg[i - WIN1] -= sum/WIN1;
	    sum += woBaseline[i] - aux;
	}
    }
    else {
	for (; i < chunk + start_index; ++i) {
	    int32_t aux = woBaseline[i - WIN1];
	    ecg[i - WIN1] -= sum/WIN1;
	    sum += woBaseline[i] - aux;
	}

	for (i -= WIN1; i < chunk + start_index; ++i) {
	    int32_t aux = woBaseline[i];
	    ecg[i] -= sum/(ecgSize-i);
	    sum -= aux;
	}
    }

    pi_cl_team_barrier();
    
    for (i = start_index; i < start_index + chunk; ++i) {
        woBaseline[i] = ecg[i];
    }
    
    rPeak_sync = NUM_CORES;
    pi_cl_team_barrier();
    
    // custom dynamic scheduling clause
    i = id;
    
    while (i < *rPeakSize) {
        int center = (int) rPeak[i];
        int l = max(center - INTEGRAL_RADIUS, 0);
        int r = min(center + 1 + INTEGRAL_RADIUS, ecgSize);
        int32_t integral = 0;
        for (int j = l; j < r; ++j) {
            integral += std::abs(woBaseline[j]);
        }
        edr[i] = (fixed_t)integral / (r-l);
	
	pi_cl_team_critical_enter();
	i = rPeak_sync++;
	pi_cl_team_critical_exit();
    }
    pi_cl_team_barrier();
    // free space
    if (id == 0)
	pi_cl_l1_free(&cluster_dev, (void *) woBaseline, sizeof(int32_t) * WIN_SIZE);
}

