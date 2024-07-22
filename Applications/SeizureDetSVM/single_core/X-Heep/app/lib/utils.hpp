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



#ifndef UTILS_HPP_
#define UTILS_HPP_

template<typename T>
T min(T lhs, T rhs) {
    return (lhs < rhs? lhs : rhs);
}

template<typename T>
T max(T lhs, T rhs) {
    return (lhs > rhs? lhs : rhs);
}

template<typename T>
inline T abs(T x) {
    return x > 0? x : -x;
}

// template<typename T>
// T Meanf(const T* input, int len) {
//     float mean = 0;
//     for (int i = 0; i < len; ++i) {
//         mean += input[i];
//     }
//     mean /= len;
//     return mean;
// }

// template<typename T>
// T CenteredVariancef(const T* input, int len) {
//     float var = 0;
//     for (int i = 0; i < len; ++i) {
//         var += input[i]*input[i];
//     }
//     var /= len - 1;
//     return var;
// }

// template<typename T>
// T Variancef(const T* input, int len, T mean) {
//     float var = 0;
//     for (int i = 0; i < len; ++i) {
//         var += (input[i] - mean)*(input[i] - mean);
//     }
//     var /= len - 1;
//     return var;
// }

template<typename T>
T RemMod(T a, T b) {
    int div = a/b;
    return a - div*b;
}

template<typename T>
T Sign(T x) {
    return x < 0? -1 : 1;
}

#endif  // UTILS_HPP_

