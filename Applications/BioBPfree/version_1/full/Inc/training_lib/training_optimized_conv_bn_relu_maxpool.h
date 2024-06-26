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


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author:      Daniel Panero                                                                           //
// Date:        June 2024                                                                               //
// Desciption:  Aggressive optimizations on forward+back pass on conv1D + batchnorm + ReLU + MaxPool    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _TRAINING_OPTIMIZED_CONV_BN_RELU_MAXPOOL_H_
#define _TRAINING_OPTIMIZED_CONV_BN_RELU_MAXPOOL_H_

#include <stdint.h>

#include "training_batch_norm.h"
#include "Adam_optimizer.h"
#include "utils.h"

struct dLdy_maxpool_dy_maxpool_dy_conv_t
{
    uint16_t dy_maxpool_dy_conv : 16;

    // Binary bools array to store which element is the greatest and if they are equal
    uint8_t gts : 8;
    uint8_t eqs : 8;
};

my_type forward_dLdy_conv1d_bn_relu_maxpool_4(const my_type *x[4], struct dLdy_maxpool_dy_maxpool_dy_conv_t *dLdy, my_type *filters, my_type *biases, batch_norm_params_t *batch_norm_params,
                                                int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding, bool bias_sharing, 
                                                int maxpool_len, int output_size);
void forward_output_conv1d_bn_relu_maxpool_4(const my_type *x[4], my_type *y[4], my_type *filters, my_type *biases, batch_norm_params_t *batch_norm_params, 
                                                int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding, bool bias_sharing, 
                                                int maxpool_len, int output_size);
void backward_optimized_conv1d_bn_relu_maxpool(const my_type *x[4], my_type *dLdw, my_type *dLdb, struct dLdy_maxpool_dy_maxpool_dy_conv_t *dLdy, batch_norm_params_t *batch_norm_params,
                                                int input_len, int input_depth, 
                                                int no_filters, int filter_len, int stride, int padding, bool bias_sharing, 
                                                int maxpool_len, int conv_output_len, int output_len);

void Adam_step_optimized(my_type *filters, my_type *biases, my_type *dLdw, my_type *dLdb, Adam_parameters *Adam_filter, Adam_parameters *Adam_bias, int filter_size, int bias_size);
#endif
