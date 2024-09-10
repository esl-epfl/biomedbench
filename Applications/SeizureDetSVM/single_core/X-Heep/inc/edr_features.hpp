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



#ifndef EDR_FEATURES_HPP_
#define EDR_FEATURES_HPP_

extern "C" {
    #include <stdint.h>
    #include "lib/fixmath.h"
}
#include "global_config.hpp"

#define EDR_LOMBSIZE LOMBSIZE

void ExtractEdrFreqFeatures(
    const fixed_t* edr,
    const fixed_t* edrTime,
    int edrSize,
    fixed_t* feat,
    int featSize
);

#endif  // EDR_FEATURES_HPP_

