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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author:          Dimitrios Samakovlis                                                                        //
// Date:            November 2023                                                                               //
// Description:     Batch normalization layer for training                                                      //
// Training:        y = gamma * (x - mean) / sqrt(variance + epsilon) + beta                                    //
// Moving average:  mean_moving = momentum * mean_moving + (1 - momentum) * mean                                //
// Moving variance: var_moving = momentum * var_moving + (1 - momentum) * var                                   //
// Inference:       y = gamma * (x - mean_moving) / sqrt(variance_moving + epsilon) + beta                      //
// Dimensions:      gamma, beta, (moving_)avg, (moving_)var all are vectors of size channels                    //
// Initialization:  gamma = moving_avg = 1, beta = moving_mean = 0, epsilon / moomentum user initialized        //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _TRAINING_BATCH_NORM_H_
#define _TRAINING_BATCH_NORM_H_

#include "defines.h"


// Global variables needed for batch normalization
typedef struct batch_norm_parameters {
    my_type *mean, *var, *mean_moving, *var_moving, *gamma, *beta;
    int channels, samples;
    my_type momentum, epsilon;
} batch_norm_params_t;


// MUST CALL THIS TO INSTANTIATE A BATCH NORM LAYER
// Allocates and initialize vectors / params according to description on top
void bn_init(batch_norm_params_t *batch_norm_params, int channels, int samples, my_type momentum, my_type epsilon);

// MUST CALL THIS TO DELETE MEMORY AFTER BATCH NORM LAYER IS NO LONGER NEEDED
// Note: Not needed if bn_init is not called (i.e. )
void bn_delete_memory(batch_norm_params_t *batch_norm_params);

// Call this to instantiate a batch norm layer with pre-trained parameters (Instead of bn_init)
void bn_init_pretrained(batch_norm_params_t *batch_norm_params, int channels, int samples, my_type *gamma, my_type *beta, my_type *mean, my_type *var, my_type epsilon);

// Per channel: y = gamma * (x - mean) / sqrt(variance + epsilon) + beta
// Also updates moving mean and moving variance
void bn_training_forward(batch_norm_params_t *batch_norm_params, my_type *x, my_type *y);


// Per channel: y = gamma * (x - mean_moving) / sqrt(var_moving + epsilon) + beta
void bn_inference_forward(batch_norm_params_t *batch_norm_params, my_type *x, my_type *y);


// Calculate dydgamma for column i (gamma_i)
// dydgamma has dimension (samples x channels) x channels
// dydgamma_i_bn has dimension samples x channels but we only use part of the column (non-zero elements)
// Memory optimization: dydgamma_i_bn has dimension samples - only store the non-zero elements
void dydgamma_i_bn(batch_norm_params_t *batch_norm_params, my_type *x, my_type *dydgamma, int column_index);


// Calculate dydbeta for column i (gamma_i)
// dydgamma has dimension (samples x channels) x channels
// dydgamma_i_bn has dimension samples x channels but we only use part of the column (non-zero elements)
// Memory optimization: dydgamma_i_bn has dimension samples - only store the non-zero elements
void dydbeta_i_bn(batch_norm_params_t *batch_norm_params, my_type *dydbeta, int column_index);


// Calculate dydx for column i
// dydx has dimension (samples x channels) x (samples x channels)
// Theoretically dydx_i_bn has dimension samples x channels but we only use part of the column (non-zero elements)
// Memory optimization: dydx_i_bn has dimension samples - only store the non-zero elements
void dydx_i_bn(batch_norm_params_t *batch_norm_params, my_type *x, my_type *dydx, int column_index);


// Calculate dydx_ii for column i (if bn layer Frozen)
// Return value is a scalar
// The complete gradient dydx_bn is a matrix with dimensions (samples x channels) but we only return the diagonal elements of each column
my_type dydx_i_bn_frozen(batch_norm_params_t *batch_norm_params, int column_index);

#endif
