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



#include "fft.hpp"
#include "fft_look_up_table.hpp"

extern "C"	{
	#include "Pico_management/multicore.h"
	#include <stdio.h>
}

void Fft (cn pol[], const int n, int id)	{
    static constexpr int rSize = FLOMB_MAX_SIZE>>1;
    static constexpr float PI = acos((float) -1);
    
    // Sequential dependencies do not allow parallelism or it is too complicated to find a conflict free way
    // Only way I see is to have the j indexes precomputed, but there are n indexes (2048) and there is lack of space
    if (id == 0)	{
	for (int i = 1, j = 0; i < n; ++i) {
	    int bit = n >> 1;
	    for (; j & bit; bit >>= 1) {
		j ^= bit;
	    }
	    j ^= bit;
	    if (i < j)	{
		swap(pol[i], pol[j]);
	    }
	}
    }
    pico_barrier(id);
    
    int hlen = 1, dis = rSize;
    // Interestingly the 2nd loop is perfectly parallelizable and free of sequential dependencies
    // However if we statically assign workload the symmetricity breaks after hlen >= n/8
    // So lets find another way to parallelize after hlen = n/8
    for (; hlen < n/8; hlen <<= 1, dis >>= 1) {
		for (int i = id * 2 * hlen; i < n; i += 2*hlen*NUMBER_OF_CORES-hlen) {
	    	for (int j = i+hlen, idx = 0; idx < rSize; ++i, ++j, idx += dis) {
				cn odd = fastmul(pol[j], roots[idx]);
				pol[j] = pol[i] - odd;
				pol[i] = pol[i] + odd;
	    	}
		}
		pico_barrier(id);
    }
    
    // continue with another partition
    for (; hlen < n; hlen <<= 1, dis >>= 1) {
		for (int i = id; i < n; i += hlen) {
	    	for (int j = i+hlen, idx = id * dis; idx < rSize; i+=NUMBER_OF_CORES, j+=NUMBER_OF_CORES, idx += NUMBER_OF_CORES * dis) {
				cn odd = fastmul(pol[j], roots[idx]);
				pol[j] = pol[i] - odd;
				pol[i] = pol[i] + odd;
	    	}
		}
		pico_barrier(id);
    }
    
    pico_barrier(id);
}
