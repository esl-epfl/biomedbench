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
//////// Mapping to X-Heep: Francesco Poluzzi  \\\\


//////////////////////////////////////////////////////////////////
// Title:   Convolution 1D and common layers training           //
// Author:  Dimitrios Samakovlis                                //
// Date:    February 2024                                       //
// Description:                                                 //
// Implemented some basic Conv1D layers for:                    //
//      1. Forward propagation                                  //
//      2. Gradients with respect to input / filters-bias       //
// Examples for Backward Propagation for common layers          //
// Focus is on memory saving for resource-constrained devices   //
//////////////////////////////////////////////////////////////////


#ifndef _TRAINING_CONV_H_
#define _TRAINING_CONV_H_

#include "training_batch_norm.h"

// *** HELPERS FOR MEMORY REFERENCING ***
#define mem2d(data,data_len,j,i)   data[((j)*(data_len))+(i)]
#define mem3d(filter,filter_len,filter_depth,k,j,i)   filter[((k)*(filter_depth)+(j))*(filter_len)+(i)]


// *** FORWARD PROPAGATION - COMMON CONV LAYERS ***

// Description: Simple 1D convolution on 2D input map
// Input:   2D vector -> input_len x input_depth
// Filters: 3D vector -> no_filters x filter_len x input_depth
// Output:  2D vector -> no_filters x (input_len / stride) for zero padding
// Note: Can be 1D if input_depth = 1
void conv1D(const my_type *x, my_type *filter, my_type *bias, my_type *output,
            int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding, bool bias_sharing);


// Description: ReLu layer
// Input:   1D vector -> size
// Output:  1D vector -> size
// Note: Input can be higher dimension, as long as it is continuous in memory
void relu(my_type *x, my_type *y, int size);


// Description: Max pooling layer
// Input:   1D vector -> x_len
// Output:  1D vector -> y_len
// Note: Cannot deal with higher dimensions (i.e. 2D pooling)
void max_pool1D(my_type *x, my_type *y, int pool_size, int x_len, int y_len);


// Description: Simple 1D convolution on 2D input map with ReLu
// Input:   2D vector -> input_len x input_depth
// Filters: 3D vector -> no_filters x filter_len x input_depth
// Output:  2D vector -> no_filters x (input_len / stride) for zero padding
// Note: Can be 1D if input_depth = 1
void conv1D_relu(const my_type *x, my_type *filter, my_type *bias, my_type *output,
            int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding, bool bias_sharing);


// Description: Normalization layer where the input vector is scaled down by its Euclidean norm
// Input:   1D vector -> size
// Output:  1D vector -> size
my_type normalize(my_type *x, my_type *y, int size);


// Description: Dropout layer
// Input:   1D vector -> size
// Output:  1D vector -> size
// Note: Randomly sets elements to 0 with probability dropout_rate
void dropout(my_type *x, my_type *y, int size, my_type dropout_rate);




// *** GRADIENTS ***

// This function calculates the derivative of the conv1D output with respect to a specific weight (i.e. w_1 for weight_index = 1)
// 0 for the output elements that this weight was not involved
// x_ij , if the weight was multiplied with x_ij during convolution
// x:       2D vector -> input_len  x input_depth
// dydw_i:  1D vector -> output_size x 1  (output_size = output_len * output_depth)
void dydw_column_i_conv1D(const my_type *x, my_type *dydw_i,
                          int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding,
                          int output_len, int output_depth, int weight_index);


// This function calculates the derivative of the output (after conv1D -> ReLu) with respect to a specific weight (i.e. w_1 for weight_index = 1)
// 0 for the output elements that this weight was not involved or that the output is 0 (ReLu)
// x_ij / normalization_factor, if the weight was multiplied with x_ij during convolution and the output is >0
// x:       2D vector -> input_len  x input_depth
// dydw_i:  1D vector -> output_size x 1  (output_size = output_len * output_depth)
void dydw_column_i_conv1D_relu(const my_type *x, my_type *y, my_type *dydw_i,
                                int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding,
                                int output_len, int output_depth, int weight_index);


// This function calculates the derivative of the ReLu layer
// Input:   1D vector -> size
// Output:  1D vector -> size
// Note: The output is the diagonal of a size x size matrix
// Note: The derivative is either 1 or 0
void dydx_relu(const my_type *y, my_type *output, int size);


// This function calculates the derivative of the normalization layer (y_i = x_i / norm(x))
// Input:   1D vector -> size
// Output:  1D vector -> size
// Returns the Jacobian matrix of the normalization layer
void dydx_normalization(const my_type *x, my_type *dydx, int size, my_type normalization_factor);

// Derivative of normalization layer per column (avoid storing the whole matrix)
void dydx_column_i_normalization(const my_type *x, my_type *dydx, int size, my_type normalization_factor, int column_index);
// ************************************************************


// *** EXAMPLES - BACKWARD PROPAGATION FOR COMMON CONV LAYERS ***

// This function calculates the derivative of the Euclidean Loss function with respect to the weights (filter parameters)
// Network: 1 layer of Conv1D
// Output:  2D vector -> no_filters x (filter_len * input_depth) = #filter_weights
void dLdw_conv1D_QOID_Euclidean(const my_type *x, my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *dLdw,
                                int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding,
                                int output_len, int output_depth, int input_index);


// This function calculates the derivative of the Euclidean Loss function with respect to the bias parameters
// Network: 1 layer of Conv1D
// Output:  2D vector -> no_biases x (bias_size) = #biases
void dLdb_conv1D_QOID_Euclidean(const my_type *x, my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *dLdb,
                                int output_len, int output_depth, int input_index);


// This function calculates the derivative of the Norm1 Loss function with respect to the weights and biases (trainable parameters)
// Network: 1 layer of Conv1D + ReLU + Normalization
// ATTENTION: Adds up the new derivative to dLdw, dLdb (does not assign - this is useful for doing batches without memory overhead)
void dLdw_dLdb_conv1D_relu_QOID_Norm1(const my_type *x, my_type *x_norm, my_type *y, my_type *y1, my_type *y2, my_type *y3, my_type *y4,
                                        my_type *dLdw, my_type *dLdb,
                                        my_type normalization_factor, int input_len, int input_depth,
                                        int no_filters, int filter_len, int stride, int padding,
                                        int output_len, int output_depth, int input_index, bool bias_sharing);


// This function calculates the derivative of the Norm1 Loss function with respect to the weights and biases (trainable parameters)
// Note: BN layer is frozen
// Network: 1 layer of Conv1D + Batch Normalization + ReLU + Max pooling
// ATTENTION: Adds up the new derivative to dLdw, dLdb (does not assign - this is useful for doing batches without memory overhead)
void dLdw_dLdb_conv1D_block_QOID_Norm1(const my_type *x, my_type *y_relu, my_type *y, my_type *y1, my_type *y2, my_type *y3, my_type *y4,
                                        my_type *dLdw, my_type *dLdb,
                                        int input_len, int input_depth,
                                        int no_filters, int filter_len, int stride, int padding, bool bias_sharing,
                                        int conv_output_len, int output_depth,
                                        batch_norm_params_t *batch_norm_params,
                                        int pool_size, int output_len,
                                        int input_index, uint32_t *cycle_count); 


#endif