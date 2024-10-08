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



#ifndef SVM_HPP_
#define SVM_HPP_

#include "rri_features.hpp"
#include "edr_features.hpp"

/**
 * Predicts the seizure with a support vector machine,
 * based on all the features extracted.
 * If the result is bigger than 0, then the prediction is positive.
 */
fixed_t predict(fixed_t* feature);

#endif  // SVM_HPP_

