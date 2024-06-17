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



#ifndef EDR_HPP_
#define EDR_HPP_

#include <stdint.h>
#include <cmath>
#include "global_config.hpp"
#include "utils.hpp"

template<int WIN, typename arr_t, typename filt_t>
void MeanFilt(const arr_t* arr, int arrSize, filt_t* out) {
    filt_t sum = 0;
    int i = 0;
    for (; i < WIN; ++i) {
        sum += arr[i];
    }
    for (; i < arrSize; ++i) {
        out[i - WIN] = sum/WIN;
        sum += arr[i] - arr[i - WIN];
    }
    for (i -= WIN; i < arrSize; ++i) {
        out[i] = sum/(arrSize-i);
        sum -= arr[i];
    }
    // TODO this does not work :(
    // assert((sum > 0? sum : -sum) < 0.1);
}

template<int WIN, typename arr_t>
void MeanFilt(arr_t* arr, int arrSize) {
    arr_t sum = 0;
    int i = 0;
    for (; i < WIN; ++i) {
        sum += arr[i];
    }
    for (; i < arrSize; ++i) {
        arr_t aux = arr[i - WIN];
        arr[i - WIN] = sum/WIN;
        sum += arr[i] - aux;
    }
    for (i -= WIN; i < arrSize; ++i) {
        arr_t aux = arr[i];
        arr[i] = sum/(arrSize-i);
        sum -= aux;
    }
    // TODO this does not work :(
    // assert((sum > 0? sum : -sum) < 0.1);
}

template<int ECG_FREQ, int MAX_ECG_SIZE, typename ecg_t, typename rpk_t>
void EcgDerivedRespiration(
    const ecg_t* ecg,
    int ecgSize,
    const rpk_t* rPeak,
    int rPeakSize,
    ecg_t* edr
) {
    static constexpr int WIN0 = std::ceil(0.2*ECG_FREQ);
    static constexpr int WIN1 = std::ceil(0.6*ECG_FREQ);
    static constexpr int INTEGRAL_RADIUS = std::ceil(0.1*ECG_FREQ);

    //static ecg_t woBaseline[MAX_ECG_SIZE];
    ecg_t *woBaseline = (ecg_t *) pvPortMalloc(sizeof(ecg_t) * (MAX_ECG_SIZE));

    // Double filter the signal...
    MeanFilt<WIN0>(ecg, ecgSize, woBaseline);
    MeanFilt<WIN1>(woBaseline, ecgSize);
    // ... and substract it from original sequence
    //TODO probably we wanted average around and not next :(
    for (int i = 0; i < ecgSize; ++i) {
        woBaseline[i] = ecg[i] - woBaseline[i];
    }

    for (int i = 0; i < rPeakSize; ++i) {
        int center = rPeak[i];
        int l = max(center - INTEGRAL_RADIUS, 0);
        int r = min(center + 1 + INTEGRAL_RADIUS, ecgSize);
        ecg_t integral = 0;
        for (int j = l; j < r; ++j) {
            integral += std::abs(woBaseline[j]);
        }
        edr[i] = integral / (r-l);
    }
    
    vPortFree(woBaseline);
}


#endif  // EDR_HPP_
