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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Title:   Training with Quad-Output Intra-Distances (QOID) loss function                                              //
// Author:  Dimitrios Samakovlis                                                                                        //
// Date:    October 2023                                                                                                //
// Description: The goal of this loss function is to                                                                    //
//      1. Minimize the same-class distances (y1, y2) and (y3, y4)                                                      //
//      2. Maximize the different-class distances (y1, y3), (y1, y4), (y2, y3), (y2, y4)                                //
//      Given 4 outputs of neural layer y1 + y2 (Class 0), y3 + y4 (Class 1)                                            //
//      loss_QOID = dist(y1, y2) + dist(y3, y4) + 1/dist(y1, y3) + 1/dist(y1, y4) + 1/dist(y2, y3) + 1/dist(y2, y4)     //
//      where dist() cam be defined as Euclidean or Abs distance (Norm1)                                                //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _TRAINING_LOSS_QOID_H_
#define _TRAINING_LOSS_QOID_H_

#include "defines.h"


// *** QOID Loss using sum of Abs distance (Norm1) ***

// Calculate the Loss value for 1D vectors
my_type loss_QOID_Norm1();

// Update the distances of the output vectors y
void update_loss_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, int output_size);


// *** GRADIENTS ***
// These functions calculates the derivative of the Loss function with respect to the layer outputs (Loss function inputs)
// Output: Row vector (1 x output_size)
// Note: 4 similar functions depending on the input considered - y1, y2 healthy samples and y3, y4 unhealthy samples
void dLdy1_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size);
void dLdy2_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size);
void dLdy3_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size);
void dLdy4_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size);

// ********************************************************************************************************************


// *** QOID Loss using Euclidean distance ***

// Calculate the Loss value for 1D vectors
my_type loss_QOID_Euclidean();

// Update the distances of the output vectors y
void update_loss_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, int output_size);


// *** GRADIENTS ***
// These functions calculates the derivative of the Loss function with respect to the layer outputs (Loss function inputs)
// Output: Row vector (1 x output_size)
// Note: 4 similar functions depending on the input considered - y1, y2 healthy samples and y3, y4 unhealthy samples
void dLdy1_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size);
void dLdy2_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size);
void dLdy3_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size);
void dLdy4_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size);

// ********************************************************************************************************************


#endif