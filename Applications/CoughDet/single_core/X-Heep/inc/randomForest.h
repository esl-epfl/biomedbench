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



#ifndef _RANDOMFOREST_H_
#define _RANDOMFOREST_H_

#include <inttypes.h>

#define N_AUDIO_FEAT_RF     26
#define N_IMU_FEAT_RF       12
#define N_BIO_FEAT_RF       2

#define TOT_FEATURES_RF    (N_AUDIO_FEAT_RF + N_IMU_FEAT_RF + N_BIO_FEAT_RF)  // This ML model was trained for 40 features

#define MAX_NODES 461
#define N_TREES 100
#define N_NODES 355
#define CLASS_0 -1
#define CLASS_1 -2

extern const float feat_means[TOT_FEATURES_RF];
extern const float feat_stds[TOT_FEATURES_RF];

extern const int16_t children[N_TREES][N_NODES][2];

extern float __attribute__((section(".xheep_data_flash_only"))) __attribute__ ((aligned (16))) weights[N_TREES][N_NODES] ;

extern const int16_t node_features[N_TREES][N_NODES];

void predict_c(float *feat, float *probs);


#endif
