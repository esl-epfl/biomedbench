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



#ifndef _FEATURE_EXTRACTION_H
#define _FEATURE_EXTRACTION_H

#include <stdlib.h>
#include <imu_features.h>

// Macro for printing the features
// #define PRINT_AUDIO_FEAT
#define PRINT_IMU_FEAT

void audio_features(const int8_t *features_selector, const float *sig, int16_t len, int16_t fs, float *feats);

void imu_features(const int8_t *features_selector, const float sig[][Num_IMU_signals], int16_t len, float *feats);

// void generate_features_names(char *names[]);

#endif