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



#ifndef RRI_FEATURES_HPP_
#define RRI_FEATURES_HPP_

#include <stdint.h>
extern "C" {
    #include "fixmath.h"
}
#include "global_config.hpp"

// ECG Frequency: Used for normalization to
// translate rri from index difference to seconds
const int RRI_FEAT_ECG_FREQ = ECG_FREQ;

// Statistical and Lorenz Plot Features ───────────────────────────────────────

// Variation in rri threshold (s) beyond which it is an irregularity
constexpr float RRI_FEAT_IRR_DIFF = 0.05f;

struct RriStatisticalFeats {
    fixed_t mean;
    fixed_t stdDev;
    fixed_t variance;
    fixed_t sqrtOrder2Derivative;
    fixed_t percIrregularities;
};

struct RriLorenzFeats {
    fixed_t std1;
    fixed_t std2;
    fixed_t T;
    fixed_t L;
    fixed_t csi;
    fixed_t modCsi;
    fixed_t cvi;
};

struct RriFeats {
    RriStatisticalFeats stats;
    RriLorenzFeats lorenz;
};

RriFeats ExtractRriFeatures(const int16_t* rri, int rriSize);

// Frequency Features ─────────────────────────────────────────────────────────

// The borders of the frequency spectrum used to evaluate the fastLomb output.
const fixed_t RRI_FREQ_LF_BORDER_LOW  = fx_ftox(0.040, FLOMB_FRAC);
const fixed_t RRI_FREQ_HF_BORDER_LOW  = fx_ftox(0.150, FLOMB_FRAC);
const fixed_t RRI_FREQ_HF_BORDER_HIGH = fx_ftox(0.400, FLOMB_FRAC);

struct RriFreqFeats {
    fixed_t totPow;
    fixed_t lowFreqRatio;
    fixed_t highFreqRatio;
    fixed_t lowToHighFreqRatio;
};

RriFreqFeats ExtractRriFreqFeatures(
    const fixed_t* hrv,
    const fixed_t* rriTime,
    int rriSize
);

#endif  // RRI_FEATURES_HPP_

