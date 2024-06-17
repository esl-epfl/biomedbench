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



#include "edr_features.hpp"
#include "fastlomb.hpp"

extern "C"  {
    #include "Pico_management/multicore.h"
    #include <stdio.h>
}

int global_sync2 = NUMBER_OF_CORES;
int nfs;
int64_t sumf = 0;

extern fixed_t *power;
extern fixed_t maxFreq; // Hz
extern fixed_t timeSpan; // s
extern fixed_t ofac;

void ExtractEdrFreqFeatures(const int32_t **arg, int id) {
    int edrSize = * ( (int *) arg[3]);
    fixed_t* edr = (fixed_t *) arg[5];
    fixed_t* feature = (fixed_t *) arg[8];
    const fixed_t* edrTime = (fixed_t *) arg[10];
    fixed_t* feat = (fixed_t *) arg[11];
    int featSize = EDR_NUM_FREQ_FEATS;
    
    // DIMITRIOS SAMAKOVLIS - PARALLELIZATION
    int chunk = edrSize / NUMBER_OF_CORES;
    int start_index = id * chunk;
    if (id == NUMBER_OF_CORES - 1)
	    chunk += edrSize % NUMBER_OF_CORES;
    
    for (int i = start_index; i < start_index + chunk; ++i) {
        edr[i] = fx_xtox(edr[i], ECG_FRAC, FLOMB_FRAC);
    }
    
    if (id == 0)	{
	    maxFreq = fx_itox(1, FLOMB_FRAC); // Hz
	    timeSpan = FastLomb_timeSpan(edrTime, edrSize); // s
	    ofac = fx_itox(2, FLOMB_FRAC);
	    nfs = FastLomb_numFreqs(maxFreq, timeSpan, ofac);
	    power = (fixed_t *) malloc(sizeof(fixed_t) * (FLOMB_MAX_SIZE/2));
    }

    pico_barrier(id);
    FastLomb_(edr, edrTime, edrSize, timeSpan, maxFreq, power, ofac, id);
    pico_barrier(id);
    
    // AGAIN STATIC PARALLELIZATION
    chunk = nfs / NUMBER_OF_CORES;
    start_index = id * chunk;
    if (id == NUMBER_OF_CORES - 1)
	    chunk += nfs % NUMBER_OF_CORES;
    
    int sum = 0;
    for (int i = start_index; i < start_index + chunk; ++i) {
        sum += power[i];
    }
    
    pico_critical_enter();
    sumf += sum;
    pico_critical_exit();
    
    pico_barrier(id);
    
    fx_rdiv_t isumf;
    fx_invx(sumf>>5, FLOMB_FRAC, &isumf);
    
    // DYNAMIC PARALLELIZATION
    int i = id;
    
    while (1) {
	    int l = i * nfs / featSize;
        int r = (i+1) * nfs / featSize;
        feat[i] = 0;
        for (int j = l; j < r; ++j) {
            feat[i] += power[j];
        }
	    feat[i] = fx_rdivx(feat[i], &isumf)>>5;
	    pico_critical_enter();
	    i = global_sync2++;
	    pico_critical_exit();
	    if (i >= featSize)
	        break;
    }

    pico_barrier(id);
    
    if (id == 0)	{
	    for (int i = 0; i < EDR_NUM_FREQ_FEATS; ++i) {
	        feature[15+i] = fx_xtox(feat[i], FLOMB_FRAC, SVM_FRAC);
	    }
	    //free space
	    free((void *) power);
    }
    pico_barrier(id);

    return;
}
