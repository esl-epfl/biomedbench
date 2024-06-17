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

// Standard libraries
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Training library includes
#include "utils.h"
#include "training_conv.h"
#include "Adam_optimizer.h"
#include "training_loss_QOID.h"
#include "training_batch_norm.h"

// Load parameters
#include "parameters.h"
#include "main.h"

// Conv Block structure: Conv1D -> Batch Normalization -> ReLU -> Max pool
int main()   {

    // Convolution layer variables
    int input_len = INPUT_LEN, input_depth = INPUT_CHANNELS;
    int no_filters_ = NO_FILTERS, filter_len = FILTER_LEN, filter_size = no_filters_ * filter_len * input_depth, padding = PADDING, stride = STRIDE, bias_size;
    int conv_output_len = 64, output_depth = no_filters_, conv_output_size = conv_output_len * output_depth;
    bool bias_sharing = true;

    // Calculate bias_ size based on sharing or not
    if (bias_sharing)
        bias_size = output_depth;
    else
        bias_size = conv_output_size;

    // Batch normalization parameters
    batch_norm_params_t bn_params;
    bn_init_pretrained(&bn_params, output_depth, input_len, gamma_, beta_, mean_, var_, 1e-5); 

    // Max pool parameters
    int pool_size = 4, output_len = conv_output_len / pool_size, output_size = conv_output_size / pool_size; 

    // Adam optimizer variables
    my_type beta1 = 0.9, beta2 = 0.999, alpha = 0.001, epsilon = 1e-8;
    Adam_parameters Adam_filter, Adam_bias;


    // --- MEMORY ALLOCATION ---

    // Allocate memory for outputs y, y_relu (4 times to account for the 4 inputs) and derivatives dLdw, dLdb
    my_type *y_seiz1 = (my_type *) malloc(output_size  * sizeof(my_type));
    my_type *y_seiz2 = (my_type *) malloc(output_size * sizeof(my_type));
    my_type *y_nonseiz1 = (my_type *) malloc(output_size * sizeof(my_type));
    my_type *y_nonseiz2 = (my_type *) malloc(output_size * sizeof(my_type));

    my_type *dLdw = (my_type *) malloc(filter_size * sizeof(my_type));
    my_type *dLdb = (my_type *) malloc(bias_size * sizeof(my_type));

    // Used for storing intermediate results of forward propagation + relu output to be used for gradient calculation later
    my_type *y_relu_seiz1;
    my_type *y_relu_seiz2;
    my_type *y_relu_nonseiz1;
    my_type *y_relu_nonseiz2;

    // Initialize Adam parameters
    Adam_optimizer_init(&Adam_filter, beta1, beta2, alpha, epsilon, filter_size);
    Adam_optimizer_init(&Adam_bias, beta1, beta2, alpha, epsilon, bias_size);

    // ------------------------

    #ifdef PRINT_INFO
    printf("\n\n*** BioBPfree on a convolutional block ***\n" 
            "-Description: Conv1D block (Conv1D + BatchNorm + ReLU + MaxPool) training with BioBPfree\n");
    #endif

    // --- TRAINING ---

    int epochs_counter = 0;

    while (1)   {

        y_relu_seiz1 = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_seiz2 = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_nonseiz1 = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_nonseiz2 = (my_type *) malloc(conv_output_size * sizeof(my_type));

        #ifdef PRINT_INFO
        printf("\n --- EPOCH %d --- \n", epochs_counter + 1);
        #endif

        // --- FORWARD PROPAGATION ---

        // Seizure 1
        conv1D(x_set[0], filters_, bias_, y_relu_seiz1,
                input_len, input_depth, no_filters_, filter_len, stride, padding, bias_sharing);
        bn_inference_forward(&bn_params, y_relu_seiz1, y_relu_seiz1);
        relu(y_relu_seiz1, y_relu_seiz1, conv_output_size);
        max_pool1D(y_relu_seiz1, y_seiz1, pool_size, conv_output_size, output_size);

        // Seizure 2
        conv1D(x_set[1], filters_, bias_, y_relu_seiz2,
                input_len, input_depth, no_filters_, filter_len, stride, padding, bias_sharing);
        bn_inference_forward(&bn_params, y_relu_seiz2, y_relu_seiz2);
        relu(y_relu_seiz2, y_relu_seiz2, conv_output_size);
        max_pool1D(y_relu_seiz2, y_seiz2, pool_size, conv_output_size, output_size);

        // Non-seizure 1
        conv1D(x_set[2], filters_, bias_, y_relu_nonseiz1,
                input_len, input_depth, no_filters_, filter_len, stride, padding, bias_sharing);
        bn_inference_forward(&bn_params, y_relu_nonseiz1, y_relu_nonseiz1);
        relu(y_relu_nonseiz1, y_relu_nonseiz1, conv_output_size);
        max_pool1D(y_relu_nonseiz1, y_nonseiz1, pool_size, conv_output_size, output_size);

        // Non-seizure 2
        conv1D(x_set[3], filters_, bias_, y_relu_nonseiz2,
                input_len, input_depth, no_filters_, filter_len, stride, padding, bias_sharing);
        bn_inference_forward(&bn_params, y_relu_nonseiz2, y_relu_nonseiz2);
        relu(y_relu_nonseiz2, y_relu_nonseiz2, conv_output_size);
        max_pool1D(y_relu_nonseiz2, y_nonseiz2, pool_size, conv_output_size, output_size);

        // --------------------------


        // --- CALCULATE DERIVATIVES ---

        // Update distances - important for correct calculation of derivatives and Loss function
        update_loss_QOID_Norm1(y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, output_size);
        #ifdef PRINT_INFO
        my_type Loss = loss_QOID_Norm1();
        printf("Loss: %f\n", Loss);
        #endif

        memset(dLdw,'\0', filter_size * sizeof(my_type));
        memset(dLdb,'\0', bias_size   * sizeof(my_type));

        // Seizure 1 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(x_set[0], y_relu_seiz1, y_seiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            no_filters_, filter_len, stride, padding, bias_sharing,
                                            conv_output_len, output_depth,
                                            &bn_params, pool_size, output_len, 0);

        free(y_relu_seiz1);

        // Seizure 2 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(x_set[1], y_relu_seiz2, y_seiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            no_filters_, filter_len, stride, padding, bias_sharing,
                                            conv_output_len, output_depth,
                                            &bn_params, pool_size, output_len, 1);

        free(y_relu_seiz2);

        // Non-seizure 1 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(x_set[2], y_relu_nonseiz1, y_nonseiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            no_filters_, filter_len, stride, padding, bias_sharing,
                                            conv_output_len, output_depth,
                                            &bn_params, pool_size, output_len, 2);
                                            
        free(y_relu_nonseiz1);

        // Non-seizure 2 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(x_set[3], y_relu_nonseiz2, y_nonseiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            no_filters_, filter_len, stride, padding, bias_sharing,
                                            conv_output_len, output_depth,
                                            &bn_params, pool_size, output_len, 3);
                                            
        free(y_relu_nonseiz2);

        // ----------------------------


        // --- ADAM UPDATE STEP ---

        Adam_optimizer_step(&Adam_filter, filters_, dLdw);
        Adam_optimizer_step(&Adam_bias, bias_, dLdb);

        // ------------------------

        epochs_counter++;

        #ifdef PRINT_INFO
        printf(" ----------------- \n\n");
        #endif

        // Stop if epochs limit reached
        if (epochs_counter == TRAINING_EPOCHS) {
            break;
        }

    }

    // ----------------------------


    // --- MEMORY FREE ---

    // Outputs y
    free(y_seiz1);
    free(y_seiz2);
    free(y_nonseiz1);
    free(y_nonseiz2);

    // Derivatives dLdw, dLdb
    free(dLdw);
    free(dLdb);

    // Adam structs
    Adam_optimizer_free(&Adam_filter);
    Adam_optimizer_free(&Adam_bias);


    // ------------------

    #ifdef PRINT_INFO
    printf("\n*** END OF TENSORFLOW TEST ***\n\n");
    #endif

    return 0;

}