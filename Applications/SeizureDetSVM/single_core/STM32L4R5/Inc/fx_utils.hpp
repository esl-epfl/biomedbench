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



#ifndef FX_UTILS_HPP_
#define FX_UTILS_HPP_

extern "C" {
    #include "fixmath.h"
}

namespace fixpnt_32b {

inline fixed_t Mean(const fixed_t* input, int len, int frac) {
    int64_t mean = 0;
    for (int i = 0; i < len; ++i) {
        mean += input[i];
    }
    return mean/len;
}

inline fixed_t Variance(const fixed_t* input, int len, fixed_t mean, int frac) {
    int64_t var = 0;
    for (int i = 0; i < len; ++i) {
        var += ((int64_t)input[i]*input[i]) >> frac;
    }
    var = (var - len*(((int64_t)mean*mean) >> frac))/(len-1);
    return var;
}

inline fixed_t RemMod(fixed_t a, fixed_t b, int frac) {
    fixed_t fdiv = fx_divx(a, b, frac);
    fixed_t div = fx_itox(fx_floorx(fdiv, frac), frac);
    return a - fx_mulx(div, b, frac);
}

}  // namespace fixpnt_32b

#endif  // FX_UTILS_HPP_

