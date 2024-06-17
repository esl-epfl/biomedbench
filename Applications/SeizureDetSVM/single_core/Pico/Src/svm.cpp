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



#include "svm.hpp"
extern "C" {
    #include "math.h"
}
#include "global_config.hpp"

#include "svm.inc"

fixed_t predict(fixed_t* feat) {
    // Normalize

    for (int i = 0; i < NUM_FEATURES; ++i) {
        feat[i] -= meanFeat[i];
        feat[i] = fx_mulx(feat[i], scaleFeat[i], SVM_FRAC);
    }

    // Find compound decision

    fixed_t sum = bias;
    for (int i = 0; i < SVM_SIZE; ++i) {
        fixed_t norm = 0;
        for (int j = 0; j < NUM_FEATURES; ++j) {
            fixed_t aux = feat[j] - svmVect[i][j];
            norm += fx_mulx(aux, aux, SVM_FRAC);
        }
        fixed_t exp = fx_expx(-norm, SVM_FRAC);
        sum += fx_mulx(alpha[i], exp, SVM_FRAC);
    }

    return sum;
}

