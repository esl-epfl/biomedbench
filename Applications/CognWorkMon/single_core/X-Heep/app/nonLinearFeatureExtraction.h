// Porting to X-Heep : Francesco Poluzzi
/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////////
// Author:          Stefano Albini                  //
// Contributions:   Dimitrios Samakovlis            //
// Date:            September 2023                  //
//////////////////////////////////////////////////////

#ifndef NONLINEAR_FEAT_EXTRACTION_H 
#define NONLINEAR_FEAT_EXTRACTION_H 

#include <featureExtraction.h>


void shannon_renyi_tsallis_entropy(my_int *data, my_int *sh_entropy, my_int *ren_entropy, my_int *ts_entropy, uint16_t num_bins, uint16_t value);
void EntropyFeaturesCalcWorkload(my_int *features, my_int *hist, uint8_t num_bins);

#endif
