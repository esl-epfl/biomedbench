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



#ifndef FASTLOMB_HPP_
#define FASTLOMB_HPP_

#include "fft.hpp"

void FastLomb_initialize();

fixed_t FastLomb_timeSpan(const fixed_t* t, int N);

int FastLomb_numFreqs(fixed_t maxFreq, fixed_t timeSpan, fixed_t ofac);

fixed_t FastLomb_deltaFreq(fixed_t timeSpan, fixed_t ofac);


template<bool VALS_PART>
void FastLomb_spread(fixed_t x, fixed_t y);

inline void FastLomb_spread_vals(fixed_t x, fixed_t y);


inline void FastLomb_spread_time(fixed_t x, fixed_t y);

void FastLomb_(
    const fixed_t* x,
    const fixed_t* t,
    int len,
    fixed_t timeSpan,
    fixed_t maxFreq,
    fixed_t* output,
    fixed_t ofac,
    int id = 0
);

void FastLomb(
    const fixed_t* x,
    const fixed_t* t,
    int len,
    fixed_t maxFreq,
    fixed_t* output,
    fixed_t ofac
);

#endif // FASTLOMB_HPP_

