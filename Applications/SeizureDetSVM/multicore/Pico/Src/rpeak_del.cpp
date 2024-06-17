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



#ifndef RPEAK_DEL_HPP_
#define RPEAK_DEL_HPP_


#include <cmath>
#include "morph_filt.hpp"
#include "global_config.hpp"

extern "C"  {
    #include <stdint.h>
    //#include "pico/multicore.h"
    #include "Pico_management/multicore.h"
}

// DIMITRIOS SAMAKOVLIS - PARALLELIZATION

// temporary storage for rpeaks
// used to separate rpeak indexes of cores 
// make simple pass at the end and get non-zero values
// this way we avoid sorting
int16_t rPeak_temp[MAX_RPEAKS_PER_CORE * NUMBER_OF_CORES] = {0};
/**
 *
 * @brief Finds the indices of the r-peaks in an ecg.
 *
 * Stores the result in rPeak and updates rPeakSize accordingly.
 * If the capacity is not enough to hold all R peaks then
 * the function returns -1;
 *
 * @param threshold Minimal (max-min) ecg difference in an r-peak neighborhood.
 */

int DelineateRpeaks(
    const int32_t **arg, int id
) {
    const int32_t* ecg = arg[0];
    int ecgSize = (int)*arg[1];
    int16_t* rPeak = (int16_t *)arg[2];
    int* rPeakSize_main = (int *)arg[3];
    int16_t* rri = (int16_t *)arg[4];
    int32_t threshold = *arg[5];
    
    const int INT_LEN = std::ceil(0.2*ECG_FREQ);
	
    // DIMITRIOS SAMAKOVLIS - PARALLELIZATION
    int chunk = ecgSize / NUMBER_OF_CORES;
    int start_index = id * chunk - INT_LEN + 1;
    if (id == NUMBER_OF_CORES - 1)
	chunk += ecgSize % NUMBER_OF_CORES;
    else if (id == 0)
	start_index = 0;
	
    int tmp_index = id*MAX_RPEAKS_PER_CORE;
    
    ecg += start_index;
    
    // For big INT_LEN, it is better to use a complex structure.
    if (INT_LEN >= 10) {
        // These structures calculate the min and max
        // over intervals of size INT_LEN
        ArgEroder<INT_LEN, int32_t> eroder(ecg);
        ArgDilator<INT_LEN, int32_t> dilator(ecg);

        // First, we need to push INT_LEN-1 values,
        // all but the last of the first window.
        int i = 0;
        for (; i < INT_LEN-1; ++i) {
            eroder.process(i);
            dilator.process(i);
        }
        // Then, we start to get the minimum and maximum of each window.
        for (; i < chunk + INT_LEN - 1; ++i) {
            uint16_t argmin = eroder.process(i);
            uint16_t argmax = dilator.process(i);
            // If the difference between them is enough, ...
            if (ecg[argmax] - ecg[argmin] > threshold) {
                // ... store the new R peak ...
		rPeak_temp[tmp_index++] = (int16_t) (argmax + start_index);
                // ... and skip the next INT_LEN-1 windows.
                for (int r = 0; r < INT_LEN-1; ++r) {
                    ++i;
                    eroder.process(i);
                    dilator.process(i);
                }
            }
        }
    } else {
        for (int i = 0; i+INT_LEN <= chunk; ++i) {
            int32_t max = ecg[i];
            int argmax = i;
            int32_t min = ecg[i];
            for(int j = i; j < i + INT_LEN; ++j) {
                if (ecg[j] > max) {
                    max = ecg[j];
                    argmax = j;
                }
                if (ecg[j] < min) {
                    min = ecg[j];
                }
            }
            if(max - min > threshold) {
		rPeak_temp[tmp_index++] = (int16_t) (argmax + start_index);
                i += (INT_LEN-1);
            }
        }
    }

    pico_barrier(id);
    
    // EXTRA STEP FOR PARALLELIZATION
    // NEED TO PASS THROUGH RPEAKS_TEMP 
    // ELEMENTS ARE ALREADY SORTED DUE TO THE STATIC PARTITION STRATEGY
    // THEY ARE JUST SEPARATED BY 0s
    if (id == 0)	{
	    tmp_index = 0;
	    for (int i = 0; i < MAX_RPEAKS_PER_CORE * NUMBER_OF_CORES; i++)	{
	        if (rPeak_temp[i] != 0)	{
		        // same peak found from different thread
		        if (tmp_index-1 > 0 && rPeak[tmp_index-1] + INT_LEN - 2 > rPeak_temp[i])	{
		            continue;
		        }
		        else	{
		            if (tmp_index == RPEAK_CAPACITY) {
			            return -1;
		        }
		        rPeak[tmp_index++] = rPeak_temp[i];
		    }
	    }
	}
	*rPeakSize_main = tmp_index;
	
	    // NOTE this can be parallelized but it is trivial
	    // This was orginally part of procedure.cpp
	    for (int i = 0; i < tmp_index - 1; ++i) {
	        rri[i] = rPeak[i+1] - rPeak[i];
	    }
    }

    end:
    pico_barrier(id);
    
    return 0;
}

#endif  // RPEAK_DEL_HPP_

