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


//////////////////////////////////////////////////////////////////////////
// Title:   Convolution 1D and common layers training                   //
// Author:  Dimitrios Samakovlis                                        //
// Date:    December 2023                                               //
// Description:                                                         //
// Implemented fully-connected + softmax + cross-entropy for:           //
//      1. Forward propagation                                          //
//      2. Gradients with respect to input / filters-bias               //
// Examples for Backward Propagation for common layers                  //
// Focus is on memory saving for resource-constrained devices           //
//////////////////////////////////////////////////////////////////////////

#ifndef _TRAINING_FULLY_CONNECTED_H_
#define _TRAINING_FULLY_CONNECTED_H_

#include "defines.h"



// *** FORWARD PROPAGATION ***

// Forward propagation for fully connected layer
// Operation: W*x + b
// 1d input x (input size x 1)
// 2d weights (output_size x input_size) (row i -> weights for output neuron i)
// 1d bias (output_size x 1)
// 1d output y (output_size x 1)
void fully_connected(my_type *x, my_type *weights, my_type *bias, my_type *y,
                    int input_size, int output_size);


// Forward propagation for Softmax layer
// Input size = output size
// softmax(x_i) = e^x_i / sum(e^x_i)
void softmax(my_type *x, my_type *y, int input_size); 


// Categorical cross entropy for a single sample
my_type categorical_cross_entropy(int *y_true, my_type *y_pred, int no_classes);

// ************************************************************




// *** GRADIENTS ***

// Gradient of fully connected layer with respect to its weights
// 1d input (input_size x 1)
// 2d output (output_size x #weights)
// ATTENTION: Assumes that dydw has by default 0 elements
void dydw_fully_connected(my_type *x, my_type *dydw, int input_size, int output_size);

// Gradient of Categorical crossentropy with respect to Softmax input
void dLdx_softmax_categorical_crossentropy(int *y_true, my_type *y_pred, my_type *dLdx, int output_size, int batch_size);

// Gradient of softmax layer with respect to input per column (avoid stroring whole matrix)
void dydx_column_i_softmax_(my_type *y, my_type *dydx, int size, int column_index);

// ************************************************************




// *** EXAMPLES - BACKWARD PROPAGATION FOR COMMON FULLY-CONNECTED LAYERS ***

// This function calculates the derivative of the Norm1 Loss function with respect to the weights and biases (trainable parameters)
// Network: 1 fully-connected layer
// Loss: QOID (Quad Output Intra Distances with Norm1 distance metric)
// ATTENTION: Adds up the new derivative to dLdw, dLdb (does not assign - this is useful for doing batches without memory overhead)
void dLdw_dLdb_fully_connected_QOID_Norm1(my_type *x, my_type *y1, my_type *y2, my_type *y3, my_type *y4,
                                            my_type *dLdw, my_type *dLdb, int input_size, int output_size,  int input_index);


// This function calculates the derivative of the Norm1 Loss function with respect to the weights and biases (trainable parameters)
// Network: 1 fully-connected layer + softmax
// Loss: CCE (Categorical Cross Entropy)
// ATTENTION: Adds up the new derivative to dLdw, dLdb (does not assign - this is useful for doing batches without memory overhead)
void dLdw_dLdb_fully_connected_softmax_categorical_crossentropy(my_type *x, my_type *y, int *y_true, my_type *dLdw, my_type *dLdb, 
                                                                int input_size, int output_size,  int batch_size);

// ************************************************************

#endif