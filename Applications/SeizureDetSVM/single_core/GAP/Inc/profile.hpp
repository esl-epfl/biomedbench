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



#ifndef PROFILE_HPP_
#define PROFILE_HPP_

#include "global_config.hpp"

void fProfileStart();

void fProfileStop();

static inline void StartModuleProfile() {
    if (!PRINT_MODULES_TIME) return;
    fProfileStart();
}

static inline void StopModuleProfile() {
    if (!PRINT_MODULES_TIME) return 0;
    fProfileStop();
}


#endif  // PROFILE_HPP_

