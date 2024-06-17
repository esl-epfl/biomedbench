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
#ifdef GAP9
#include "fft_look_up_table.hpp"
#elif defined(GAPUINO)
PI_L1 cn *roots;
PI_L1 int initialize = 1; //used to initialize only the first time, only works for 2 calls during the whole program
#endif
void Fft (cn pol[], const int n, int id)	{
    static constexpr int rSize = FLOMB_MAX_SIZE>>1;
    static constexpr float PI = acos((float) -1);
    
	#ifdef GAPUINO
    // Lack of L1 space obliges us to dynamically allocate-deallocate memory
    // This means every new window requires a new allocation/initialization
    if (initialize)	{
		if (id == 0)	{
	    	roots = (cn *) pi_cl_l1_malloc(&cluster_dev, sizeof(cn) * rSize);
	    	if (roots == NULL) printf("**Not enough memory in L1 for Fft!**\n");
	    	#ifdef L1_memory_analysis
	    	printf("Address of roots (fft): %p\n", roots);
	    	#endif
	    	roots[0] = fx_itox(1, FFT_FRAC);
		}
		pi_cl_team_barrier();
	
		register int chunk = rSize / NUM_CORES;
		register int start_index = id * chunk;
		if (id == NUM_CORES - 1)
	    	chunk += rSize % NUM_CORES;
		else if (id == 0)	{
	    	start_index += 1;
	    	chunk -= 1;
		}
		for (int e = start_index; e < start_index + chunk; ++e) {
	    	fixed_t angle = e * fx_ftox(PI, FFT_FRAC) / rSize;
	    	roots[e] = cn(fx_cosx(angle, FFT_FRAC), fx_sinx(angle, FFT_FRAC));
		}
    }
	#endif
    
    pi_cl_team_barrier();
    
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
    pi_cl_team_barrier();
    
    int hlen = 1, dis = rSize;
    // Interestingly the 2nd loop is perfectly parallelizable and free of sequential dependencies
    // However if we statically assign workload the symmetricity breaks after hlen >= n/8
    // So lets find another way to parallelize after hlen = n/8
    for (; hlen < n/8; hlen <<= 1, dis >>= 1) {
		for (int i = id * 2 * hlen; i < n; i += 2*hlen*NUM_CORES-hlen) {
	    	for (int j = i+hlen, idx = 0; idx < rSize; ++i, ++j, idx += dis) {
				cn odd = fastmul(pol[j], roots[idx]);
				pol[j] = pol[i] - odd;
				pol[i] = pol[i] + odd;
	    	}
		}
		pi_cl_team_barrier();
    }
    
    // continue with another partition
    for (; hlen < n; hlen <<= 1, dis >>= 1) {
		for (int i = id; i < n; i += hlen) {
	    	for (int j = i+hlen, idx = id * dis; idx < rSize; i+=NUM_CORES, j+=NUM_CORES, idx += NUM_CORES * dis) {
				cn odd = fastmul(pol[j], roots[idx]);
				pol[j] = pol[i] - odd;
				pol[i] = pol[i] + odd;
	    	}
		}
		pi_cl_team_barrier();
    }
    
    pi_cl_team_barrier();
	#ifdef GAPUINO
    if (id == 0)	{
		if (!initialize)	{
	    	pi_cl_l1_free(&cluster_dev, (void *) roots, sizeof(cn) * rSize);
	    	initialize = 1;
		}
		else
	   		initialize = 0;
    }
	#endif
}
