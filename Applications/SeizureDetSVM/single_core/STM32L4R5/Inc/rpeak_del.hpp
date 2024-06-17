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

#include <stdint.h>
#include <cmath>
#include "morph_filt.hpp"

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
template<int ECG_FREQ, typename rpk_t, typename ecg_t>
int DelineateRpeaks(
    const ecg_t* ecg,
    int ecgSize,
    rpk_t* rPeak,
    int* rPeakSize,
    int rPeakCapacity,
    ecg_t threshold = 1500
) {
    const int INT_LEN = std::ceil(0.2*ECG_FREQ);
    // We process only the first lead

    *rPeakSize = 0;

    // For big INT_LEN, it is better to use a complex structure.
    if (INT_LEN >= 10) {
        // These structures calculate the min and max
        // over intervals of size INT_LEN
        ArgEroder<INT_LEN, ecg_t> eroder(ecg);
        ArgDilator<INT_LEN, ecg_t> dilator(ecg);

        // First, we need to push INT_LEN-1 values,
        // all but the last of the first window.
        int i = 0;
        for (; i < INT_LEN-1; ++i) {
            eroder.process(i);
            dilator.process(i);
        }
        // Then, we start to get the minimum and maximum of each window.
        for (; i < ecgSize; ++i) {
            uint16_t argmin = eroder.process(i);
            uint16_t argmax = dilator.process(i);
            // If the difference between them is enough, ...
            if (ecg[argmax] - ecg[argmin] > threshold) {
                if (*rPeakSize >= rPeakCapacity) {
                    return -1;
                }
                // ... store the new R peak ...
                rPeak[*rPeakSize] = argmax;
                *rPeakSize += 1;
                // ... and skip the next INT_LEN-1 windows.
                for (int r = 0; r < INT_LEN-1; ++r) {
                    ++i;
                    eroder.process(i);
                    dilator.process(i);
                }
            }
        }
    } else {
        for (int i = 0; i+INT_LEN <= ecgSize; ++i) {
            ecg_t max = ecg[i];
            int argmax = i;
            ecg_t min = ecg [i];
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
                if (*rPeakSize >= rPeakCapacity) {
                    return -1;
                }
                rPeak[*rPeakSize] = argmax;
                *rPeakSize += 1;
                i += (INT_LEN-1);
            }
        }
    }
    return 0;
}

#endif  // RPEAK_DEL_HPP_

