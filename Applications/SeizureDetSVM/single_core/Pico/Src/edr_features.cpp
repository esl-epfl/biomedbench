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
#include "lib/fastlomb.hpp"
#include "edr.hpp"

extern "C"  {
    #include "FreeRTOS.h"
}

extern fixed_t *power;

void ExtractEdrFreqFeatures(
    const fixed_t* edr,
    const fixed_t* edrTime,
    int edrSize,
    fixed_t* feat,
    int featSize
) {
    const fixed_t maxFreq = fx_itox(1, FLOMB_FRAC); // Hz
    const fixed_t timeSpan = FastLomb_timeSpan(edrTime, edrSize); // s
    const fixed_t ofac = fx_itox(2, FLOMB_FRAC);
    int nfs = FastLomb_numFreqs(maxFreq, timeSpan, ofac);

    FastLomb_(edr, edrTime, edrSize, timeSpan, maxFreq, power, ofac);

    int64_t sumf = 0;
    for (int i = 0; i < nfs; ++i) {
        sumf += power[i];
    }
    int l = 0;
    for (int i = 0; i < featSize; ++i) {
        int r = (i+1)*nfs/featSize;
        feat[i] = 0;
        for (int j = l; j < r; ++j) {
            feat[i] += power[j];
        }
        l = r;
    }
    
    vPortFree((void *) power);
    
    fx_rdiv_t isumf;

    fx_invx(sumf>>5, FLOMB_FRAC, &isumf);
    for (int i = 0; i < featSize; ++i) {
        feat[i] = fx_rdivx(feat[i], &isumf)>>5;
    }
}
