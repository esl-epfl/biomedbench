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



#include "fixpnt_lin_pred_est.hpp"
#include <cstdlib>
extern "C" {
    #include "fixmath.h"
    #include "stdio.h"
}

namespace fixpnt_32b {

void ComputeLinPredCoeffs(
    const int32_t* norm_corr,
    int order,
    int32_t* alpha,
    int frac
) {
    fixed_t e = fx_itox(1, frac);
    for (int i = 0; i < order; ++i) {
        fixed_t conv = 0;
        for (int j = 0; j < i; ++j) {
            conv += fx_mulx(alpha[j], norm_corr[i-1-j], frac);
        }
        fixed_t k = fx_divx(norm_corr[i] - conv, e, frac);
        for (int j = 0; j < i/2; ++j) {
            fixed_t aux = alpha[j];
            alpha[j] -= fx_mulx(k, alpha[i-1-j], frac);
            alpha[i-1-j] -= fx_mulx(k, aux, frac);
        }
        if (i%2 == 1) {
            alpha[i/2] -= fx_mulx(k, alpha[i/2], frac);
        }
        alpha[i] = k;
        fixed_t factor = fx_itox(1, frac) - fx_mulx(k, k, frac);
        e = fx_mulx(e, factor, frac);
    }
    for (int i = 0; i < order; ++i) {
        alpha[i] = -alpha[i];
    }
}

//TODO remove printfs

void ComputeLinPredCoeffs(
    const int32_t* seq,
    int seqSize,
    int order,
    int32_t* alpha,
    int frac
) {
    int32_t accShift = 1;
    while ((1<<accShift) < seqSize) {
        accShift += 1;
    }
    
    int minLeadingZeroes = 64;
    for (int i = 0; i < seqSize; ++i) {
        int lz = fx_clz(labs(seq[i]) | 1);
        if (lz < minLeadingZeroes) {
            minLeadingZeroes = lz;
        }
    }
    accShift -= 2*minLeadingZeroes;
    if (accShift < 0) accShift = 0;

    uint64_t corr0 = 0;
    for (int i = 0; i < seqSize; ++i) {
        corr0 += ((int64_t)seq[i]*seq[i]) >> accShift;
    }
    int32_t corr0shift = 33;
    while (corr0shift > 0 && (corr0 & (1LL << (30+corr0shift))) == 0) {
        corr0shift -= 1;
    }

    fixed_t norm_corr[order];
    for (int i = 1; i <= order; ++i) {
        int64_t corri = 0;
        for (int j = 0; j+i < seqSize; ++j) {
            corri += ((int64_t)seq[j]*seq[j+i]) >> accShift;
        }
        int32_t rcorr0 = corr0 >> corr0shift;
        int32_t rcorri = corri >> corr0shift;
        norm_corr[i-1] = fx_divx(rcorri, rcorr0, frac);
    }

    ComputeLinPredCoeffs(norm_corr, order, alpha, frac);
}

namespace acc_32b {

void ComputeLinPredCoeffs(
    const int32_t* seq,
    int seqSize,
    int order,
    int32_t* alpha,
    int frac
) {
    int32_t accShift = 1;
    while ((1<<accShift) < seqSize) {
        accShift += 1;
    }

    int minLeadingZeroes = 64;
    for (int i = 0; i < seqSize; ++i) {
        int lz = fx_clz(labs(seq[i]) | 1);
        if (lz < minLeadingZeroes) {
            minLeadingZeroes = lz;
        }
    }
    accShift -= 2*minLeadingZeroes;
    accShift += 33;
    if (accShift < 0) accShift = 0;

    int32_t corr0 = 0;
    for (int i = 0; i < seqSize; ++i) {
        corr0 += ((int64_t)seq[i]*seq[i]) >> accShift;
    }

    fixed_t norm_corr[order];
    for (int i = 1; i <= order; ++i) {
        int32_t corri = 0;
        for (int j = 0; j+i < seqSize; ++j) {
            corri += ((int64_t)seq[j]*seq[j+i]) >> accShift;
        }
        norm_corr[i-1] = fx_divx(corri, corr0, frac);
    }

    fixpnt_32b::ComputeLinPredCoeffs(norm_corr, order, alpha, frac);
}

}  // namespace acc_32b

}  // namespace fixpnt_32b
