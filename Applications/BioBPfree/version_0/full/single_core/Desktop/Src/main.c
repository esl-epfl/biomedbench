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


//////////////////////////////////////////
// Author:  Dimitrios Samakovlis        //
// Date:    February 2024               //
//////////////////////////////////////////


// C Libraries
#include <stdbool.h>
#include <stdio.h>

// Training library headers
#include "utils.h"
#include "training_conv.h"

// SeizDetCNN headers and fixed inputs/parameters
#include "SeizDetCNN.h"
#include "inputs.h"
#include "layer_1.h"
#include "layer_2.h"
#include "layer_3.h"
#include "layer_4.h"
#include "layer_5.h"

int main(void)  {


    // *** SeizDetCNN training with BioBPfree ***
    #ifdef PRINT_PROGRESS
    printf("\n************ Training SeizDetCNN with BioBPfree **************\n");
    #endif
    SeizDetCNN_params_t parameters_SeizDetCNN;
    batch_norm_params_t bn1, bn2, bn3;

    #ifdef PRINT_PROGRESS
    printf ("--- Parameter initialization ---\n");
    #endif
    // Set the parameters of the SeizDetCNN model
    set_input_params(&parameters_SeizDetCNN, 1024, 18);
    // Conv block 1
    bn_init_pretrained(&bn1, 128, 1024, gamma_l1, beta_l1, mean_l1, var_l1, 1e-5);
    set_conv_block(&parameters_SeizDetCNN, 128, 3, 1, 1, filters_l1, bias_l1, 0.1, &bn1, 4, 1);
    // Conv block 2
    bn_init_pretrained(&bn2, 128, 256, gamma_l2, beta_l2, mean_l2, var_l2, 1e-5);
    set_conv_block(&parameters_SeizDetCNN, 128, 3, 1, 1, filters_l2, bias_l2, 0.1, &bn2, 4, 2);
    // Conv block 3
    bn_init_pretrained(&bn3, 128, 64, gamma_l3, beta_l3, mean_l3, var_l3, 1e-5);
    set_conv_block(&parameters_SeizDetCNN, 128, 3, 1, 1, filters_l3, bias_l3, 0.1, &bn3, 4, 3);
    // Dense layer 4
    set_dense_layer(&parameters_SeizDetCNN, 100, weights_l4, bias_l4, 0.3, 4);
    // Dense layer 5
    set_dense_layer(&parameters_SeizDetCNN, 2, weights_l5, bias_l5, 0.0, 5);

    training_SeizDetCNN(&parameters_SeizDetCNN, x_subset, 5);

    // ****************************************************************************************************


    return 0;
}