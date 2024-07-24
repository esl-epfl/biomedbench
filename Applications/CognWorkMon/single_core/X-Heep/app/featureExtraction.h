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

// Porting to X-Heep : Francesco Poluzzi

#ifndef	FEATURE_EXTRACTION_H
#define	FEATURE_EXTRACTION_H

#include <stdio.h>
#include <main.h>

#include <filter_signal_param.h>
#include <window_definitions.h>

#include <model_18.h>

#define NUM_FEATURES    17 

typedef enum workload{shannon_en_sig=0, renyi_en_sig, tsallis_en_sig,
			std, p_theta, p_alfa, p_beta, p_theta_rel, p_alfa_rel, p_beta_rel,
			p_alpha_beta, p_theta_alpha,
			skewness, kurtosis, hA, hM, hC
		} _workload_features_list_t; //17 features used on workload

//features for one window, both channels
typedef struct{
	unsigned char label; //ground truth for the evaluated window
	union{//allow feature access for each channel separately or both at the same time
		my_int features_all[N_CHANNEL_USED*NUM_FEATURES];
		struct{
			my_int feature_chA[NUM_FEATURES];//features extracted
			my_int feature_chB[NUM_FEATURES];//features extracted
			my_int feature_chC[NUM_FEATURES];//features extracted
			my_int feature_chD[NUM_FEATURES];//features extracted
		};
	};
} _features_t;

unsigned int FeatureExtraction(my_int *features, my_int *sig, uint8_t channel);

int decisionTreeFun(my_int x[]);

#endif
