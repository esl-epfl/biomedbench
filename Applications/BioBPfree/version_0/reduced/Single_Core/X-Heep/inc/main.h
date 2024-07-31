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
// Title:   BioBPfree training on a Convolution block layer                                                             //
// Author:  Dimitrios Samakovlis                                                                                        //
// Date:    12 March 2024                                                                                               //
// Description:     We use BioBPfree to train a convolution block layer                                                 //
// Architecture:    Conv1D + BatchNorm + ReLU + MaxPool                                                                 //
// Loss function:   QOID - Quadruple output intraclass distance (check training_loss_QOID.h)                            //
// Trainable parameters: conv weights and biases                                                                        //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CONV_BLOCK_BIOBPFREE_H_
#define _CONV_BLOCK_BIOBPFREE_H_

#include <stdbool.h>

// *** Print options ***
#define PRINT_INFO
#define PRINT_CYCLES
#define DEBUG_PRINTS

// *** Training options ***
#define TRAINING_EPOCHS 1
#define INPUT_LEN 64
#define INPUT_CHANNELS 32
#define NO_FILTERS 64
#define FILTER_LEN 3
#define STRIDE 1
#define PADDING 1   // 1 zero to the left and 1 zero to the right
#define POOL_SIZE 4

int main();

#endif