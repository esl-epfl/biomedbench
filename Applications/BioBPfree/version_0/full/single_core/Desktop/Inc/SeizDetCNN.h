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
// Title:   BioBPfree training on SeizDetCNN network                                                                    //
// Author:  Dimitrios Samakovlis                                                                                        //
// Date:    February 2024                                                                                               //
// Description: We use BioBPfree to retrain/fine-tune the full SeizDetCNN network per-layer                             //
//              The network is pretrained on the CHB-MIT EEG Seizure Detection dataset                                  //
//              We use BioBPfree to personalize on the test patient                                                     //
// Architecture: 3 conv blocks + 2 dense layers + softmax                                                               //
//               Conv block: Conv1D + BatchNorm + ReLU + MaxPool                                                        //
// Loss functions:                                                                                                      //
//      1. Categorical Cross-Entropy for final layer                                                                    //
//      2. QOID for intermediate layers (distances between outputs of same and different classes)                       //
// Trainable parameters: conv weights and biases, dense weights and biases                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _SEIZDETCNN_H_
#define _SEIZDETCNN_H_

#include "defines.h"
#include "training_batch_norm.h"


typedef struct SeizDetCNN_params {
    unsigned int in_len, in_depth;                                      // Input

    unsigned int no_filters_l1, filter_len_l1, stride_l1, padding_l1;   // Layer 1 - Conv1D
    my_type *filters_l1, *bias_l1;                                      // Layer 1 - Conv1D
    my_type dropout_l1;                                                 // Layer 1 - Dropout
    batch_norm_params_t *bn_l1;                                         // Layer 1 - BatchNorm
    unsigned int pool_size_l1;                                          // Layer 1 - MaxPool

    unsigned int no_filters_l2, filter_len_l2, stride_l2, padding_l2;   // Layer 2 - Conv1D
    my_type *filters_l2, *bias_l2;                                      // Layer 2 - Conv1D
    my_type dropout_l2;                                                 // Layer 2 - Dropout
    batch_norm_params_t *bn_l2;                                         // Layer 2 - BatchNorm
    unsigned int pool_size_l2;                                          // Layer 2 - MaxPool

    unsigned int no_filters_l3, filter_len_l3, stride_l3, padding_l3;   // Layer 3 - Conv1D
    my_type *filters_l3, *bias_l3;                                      // Layer 3 - Conv1D
    my_type dropout_l3;                                                 // Layer 3 - Dropout
    batch_norm_params_t *bn_l3;                                         // Layer 3 - BatchNorm
    unsigned int pool_size_l3;                                          // Layer 3 - MaxPool

    unsigned int no_neurons_l4;                                         // Layer 4 - Dense
    my_type *weights_l4, *bias_l4;                                      // Layer 4 - Dense
    my_type dropout_l4;                                                 // Layer 4 - Dropout

    unsigned int no_neurons_l5;                                         // Layer 5 - Dense
    my_type *weights_l5, *bias_l5;                                      // Layer 5 - Dense

} SeizDetCNN_params_t;


// Set input parameters of the SeizDetCNN network
int set_input_params(SeizDetCNN_params_t *params, unsigned int in_len, unsigned int in_depth);


// Set all the necessary params of a conv layer at once
// 0 on success, -1 on failure
int set_conv_block(SeizDetCNN_params_t *params, 
                    unsigned int no_filters, unsigned int filter_len, unsigned int stride, unsigned int padding, my_type *filters, my_type *bias,
                    my_type dropout,
                    batch_norm_params_t *bn,
                    unsigned int pool_size,
                    unsigned int layer_no);

// Set all the necessary params of a dense layer at once
// 0 on success, -1 on failure
int set_dense_layer(SeizDetCNN_params_t *params,
                    unsigned int no_neurons, my_type *weights, my_type *bias,
                    my_type dropout,
                    unsigned int layer_no);


// Train the SeizDetCNN network using BioBPfree
// Input: params - the parameters of the network
//        x - 1 subset of training data (4 samples: 2 healthy - 2 unhealthy) 
void training_SeizDetCNN(SeizDetCNN_params_t *params, const my_type *x[4], unsigned int epochs);

#endif // _SEIZDETCNN_H_