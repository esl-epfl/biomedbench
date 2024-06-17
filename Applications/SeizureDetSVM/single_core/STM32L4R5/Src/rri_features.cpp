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



#include "rri_features.hpp"
extern "C" {
    #include <math.h>
}
#include "fastlomb.hpp"

// Statistical and Lorenz Plot Features ───────────────────────────────────────

RriFeats ExtractRriFeatures(const int16_t* rri, int size) {
    // Handy alias
    const int FREQ = RRI_FEAT_ECG_FREQ;
    const int FREQ_SQ = FREQ*FREQ;

    RriFeats fts;

    // We try to work with integers for precision and speed.
    // Recall that rri contains very small numbers.

    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += rri[i];
    }
    fts.stats.mean = fx_divx(sum, size * FREQ, RRI_FRAC);

    int dsize = size - 1;

    int sqSum = 0;
    for (int i = 0; i < size; ++i) {
        sqSum += rri[i]*rri[i];
    }
    fts.stats.variance = fx_divx(
        size*sqSum - sum*sum, size*dsize*FREQ_SQ, RRI_FRAC
    );
    fts.stats.stdDev = fx_sqrtx(fts.stats.variance, RRI_FRAC);

    int prodSum = 0;
    for (int i = 0; i < size-1; ++i) {
        prodSum += rri[i+1]*rri[i];
    }
    int diffSqSum =
        2*(sqSum - prodSum) - (rri[0]*rri[0] + rri[dsize]*rri[dsize]);
    int sumSqSum =
        2*(sqSum + prodSum) - (rri[0]*rri[0] + rri[dsize]*rri[dsize]);
    fixed_t o2Derivative = fx_divx(diffSqSum, dsize*FREQ_SQ, RRI_FRAC);
    fts.stats.sqrtOrder2Derivative = fx_sqrtx(o2Derivative, RRI_FRAC);

    int irregs = 0;
    for (int i = 0; i < size-1; ++i) {
        constexpr int threshold = (RRI_FEAT_IRR_DIFF*FREQ);
        if (abs(rri[i+1] - rri[i]) > threshold) {
            irregs += 1;
        }
    }
    fts.stats.percIrregularities = fx_divx(irregs, dsize, RRI_FRAC);

    // Lorenz stats are computed considering
    // data1 = rri[i] - rr[i+1], i in [0...rriSize-1)
    // data2 = rri[i] + rr[i+1], i in [0...rriSize-1)
    // var1 = variance(data1)
    // var2 = variance(data2)

    int sum1 = rri[0] - rri[dsize];
    int sum2 = 2*sum - rri[0] - rri[dsize];
    fixed_t var1 = fx_divx(
        dsize*diffSqSum - sum1*sum1, dsize*(dsize - 1), RRI_FRAC
    );
    fixed_t var2 = fx_divx(
        dsize*sumSqSum - sum2*sum2, dsize*(dsize - 1), RRI_FRAC
    );

    fts.lorenz.std1 = fx_sqrtx(var1/2, RRI_FRAC) / FREQ;
    fts.lorenz.std2 = fx_sqrtx(var2/2, RRI_FRAC) / FREQ;
    fts.lorenz.T = 4*fts.lorenz.std1;
    fts.lorenz.L = 4*fts.lorenz.std2;
    fts.lorenz.csi = fx_divx(fts.lorenz.std2, fts.lorenz.std1, RRI_FRAC);
    fts.lorenz.modCsi = fx_mulx(fts.lorenz.L, fts.lorenz.csi, RRI_FRAC);
    fixed_t mul = fx_mulx(fts.lorenz.L, fts.lorenz.T, RRI_FRAC);
    fts.lorenz.cvi = fx_log10x(mul, RRI_FRAC);
    return fts;
}

// Frequency Features ─────────────────────────────────────────────────────────

//extern FastLombType<FLOMB_MAX_SIZE, FLOMB_FRAC> *flomb;
fixed_t *power;

RriFreqFeats ExtractRriFreqFeatures(
    const fixed_t* hrv,
    const fixed_t* rriTime,
    int rriSize
) {
    const fixed_t maxFreq = fx_itox(1, FLOMB_FRAC); // Hz
    const fixed_t timeSpan = FastLomb_timeSpan(rriTime, rriSize); // s
    // const fixed_t timeSpan = 60; // s
    const fixed_t ofac = fx_itox(2, FLOMB_FRAC);

    power = (fixed_t *) pvPortMalloc(sizeof(fixed_t) * (FLOMB_MAX_SIZE/2));

    // Computing square integral as total power
    RriFreqFeats feats;
    feats.totPow = 0;
    for (int i = 0; i < rriSize; ++i) {
        feats.totPow += fx_mulx(hrv[i], hrv[i], FLOMB_FRAC);
    }
    feats.totPow = fx_divx(feats.totPow, timeSpan, FLOMB_FRAC);

    int nfs = FastLomb_numFreqs(maxFreq, timeSpan, ofac);
    FastLomb_(hrv, rriTime, rriSize, timeSpan, maxFreq, power, ofac);
    
    fixed_t deltaFreq = FastLomb_deltaFreq(timeSpan, ofac);

    // Sum frequencies

    fixed_t sumf = 0;
    fixed_t lf = 0;
    fixed_t hf = 0;
    int i = 0;
    while (deltaFreq*(i+1) < RRI_FREQ_LF_BORDER_LOW) {
        sumf += power[i];
        ++i;
    }
    while (deltaFreq*(i+1) < RRI_FREQ_HF_BORDER_LOW) {
        lf += power[i];
        ++i;
    }
    while (deltaFreq*(i+1) < RRI_FREQ_HF_BORDER_HIGH) {
        hf += power[i];
        ++i;
    }
    sumf += lf + hf;
    while (i < nfs) {
        sumf += power[i];
        ++i;
    }

    feats.lowFreqRatio = fx_divx(lf, sumf, FLOMB_FRAC);
    feats.highFreqRatio = fx_divx(hf, sumf, FLOMB_FRAC);
    feats.lowToHighFreqRatio = fx_divx(lf, hf, FLOMB_FRAC);

    return feats;
}

