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



#include "fft.hpp"
#include "fft_look_up_table.hpp"

void Fft (cn pol[], const int n)	{
    static constexpr int rSize = FLOMB_MAX_SIZE >> 1;
    
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
    
    for (int hlen = 1, dis = rSize; hlen < n; hlen <<= 1, dis >>= 1) {
		for (int i = 0; i < n; i += hlen) {
	    	for (int j = i+hlen, id = 0; id < rSize; ++i, ++j, id += dis) {
				cn odd = fastmul(pol[j], roots[id]);
				pol[j] = pol[i] - odd;
				pol[i] = pol[i] + odd;
	    	}
		}
    }
}
