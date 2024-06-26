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


// Libraries
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Local includes
#include "SeizDetCNN.h"

// Training library headers
#include "Adam_optimizer.h"
#include "training_conv.h"
#include "training_batch_norm.h"
#include "training_fully_connected.h"
#include "training_loss_QOID.h"


// Set input parameters of the SeizDetCNN network
int set_input_params(SeizDetCNN_params_t *params, unsigned int in_len, unsigned int in_depth)   {
    
    if (params == NULL || in_len == 0 || in_depth == 0)  {
        return -1;
    }
    
    params->in_len = in_len;
    params->in_depth = in_depth;

    return 0;

}


int set_conv_block(SeizDetCNN_params_t *params, 
                    unsigned int no_filters, unsigned int filter_len, unsigned int stride, unsigned int padding, my_type *filters, my_type *bias,
                    my_type dropout,
                    batch_norm_params_t *bn,
                    unsigned int pool_size,
                    unsigned int layer_no)
{
    if (params == NULL || no_filters == 0 || filter_len == 0 || stride == 0 || filters == NULL || bias == NULL || pool_size == 0 || layer_no < 1 || layer_no > 3)  {
        return -1;
    }

    if (layer_no == 1)  {
        params->no_filters_l1 = no_filters;
        params->filter_len_l1 = filter_len;
        params->stride_l1 = stride;
        params->padding_l1 = padding;
        params->filters_l1 = filters;
        params->bias_l1 = bias;
        params->dropout_l1 = dropout;
        params->bn_l1 = bn;
        params->pool_size_l1 = pool_size;
    }
    else if (layer_no == 2)  {
        params->no_filters_l2 = no_filters;
        params->filter_len_l2 = filter_len;
        params->stride_l2 = stride;
        params->padding_l2 = padding;
        params->filters_l2 = filters;
        params->bias_l2 = bias;
        params->dropout_l2 = dropout;
        params->bn_l2 = bn;
        params->pool_size_l2 = pool_size;
    }
    else if (layer_no == 3)  {
        params->no_filters_l3 = no_filters;
        params->filter_len_l3 = filter_len;
        params->stride_l3 = stride;
        params->padding_l3 = padding;
        params->filters_l3 = filters;
        params->bias_l3 = bias;
        params->dropout_l3 = dropout;
        params->bn_l3 = bn;
        params->pool_size_l3 = pool_size;
    }

    return 0;

}


int set_dense_layer(SeizDetCNN_params_t *params,
                     unsigned int no_neurons, my_type *weights, my_type *bias,
                     my_type dropout,
                     unsigned int layer_no)
{
    if (params == NULL || no_neurons == 0 || weights == NULL || bias == NULL || layer_no < 4 || layer_no > 5)  {
        return -1;
    }

    if (layer_no == 4)  {
        params->no_neurons_l4 = no_neurons;
        params->weights_l4 = weights;
        params->bias_l4 = bias;
        params->dropout_l4 = dropout;
    }
    else if (layer_no == 5)  {
        params->no_neurons_l5 = no_neurons;
        params->weights_l5 = weights;
        params->bias_l5 = bias;
    }

    return 0;
}                     




// *** MAIN PROGRAM - BioBPfree on complete SeizDetCNN ***
void training_SeizDetCNN(SeizDetCNN_params_t *params, const my_type *x[4], unsigned int epochs)  {
    my_type loss;
    unsigned int conv_output_len, conv_output_size, input_len, input_depth, input_size, output_len, output_size, filter_size, bias_size;    // helper variables for conv layers
    bool training;  // flag for doing smart jumps in the code

    // Intermediate arrays needed for the forward and backward propagation
    my_type *in_seiz1, *in_seiz2, *in_nonseiz1, *in_nonseiz2;                   // input map for each layer
    my_type *y_seiz1, *y_seiz2, *y_nonseiz1, *y_nonseiz2;                       // output map for each layer
    my_type *y_relu_seiz1, *y_relu_seiz2, *y_relu_nonseiz1, *y_relu_nonseiz2;   // ReLU output for conv layer (needed for gradients)
    my_type *dLdw, *dLdb;                                                       // gradients for each layer (weights and biases)
    
    // --- Adam optimizer for all layers ---
    my_type beta1 = 0.9, beta2 = 0.999, alpha = 1e-6, epsilon = 1e-8;
    
    Adam_parameters Adam_filter_l1, Adam_bias_l1;
    Adam_parameters Adam_filter_l2, Adam_bias_l2;
    Adam_parameters Adam_filter_l3, Adam_bias_l3;
    Adam_parameters Adam_weights_l4, Adam_bias_l4;
    Adam_parameters Adam_weights_l5, Adam_bias_l5;

    // Initialize Adam optimizer for all layers
    #ifdef PRINT_PROGRESS
    printf("--- Adam optimizer initialization ---\n");
    #endif
    Adam_optimizer_init(&Adam_filter_l1, beta1, beta2, alpha, epsilon, params->no_filters_l1 * params->filter_len_l1 * params->in_depth);
    Adam_optimizer_init(&Adam_bias_l1, beta1, beta2, alpha, epsilon, params->no_filters_l1);
    Adam_optimizer_init(&Adam_filter_l2, beta1, beta2, alpha, epsilon, params->no_filters_l2 * params->filter_len_l2 * params->no_filters_l1);
    Adam_optimizer_init(&Adam_bias_l2, beta1, beta2, alpha, epsilon, params->no_filters_l2);
    Adam_optimizer_init(&Adam_filter_l3, beta1, beta2, alpha, epsilon, params->no_filters_l3 * params->filter_len_l3 * params->no_filters_l2);
    Adam_optimizer_init(&Adam_bias_l3, beta1, beta2, alpha, epsilon, params->no_filters_l3);
    Adam_optimizer_init(&Adam_weights_l4, beta1, beta2, alpha, epsilon, params->no_neurons_l4 * params->no_filters_l3 * 16);
    Adam_optimizer_init(&Adam_bias_l4, beta1, beta2, alpha, epsilon, params->no_neurons_l4);
    Adam_optimizer_init(&Adam_weights_l5, beta1, beta2, alpha, epsilon, params->no_neurons_l5 * params->no_neurons_l4);
    Adam_optimizer_init(&Adam_bias_l5, beta1, beta2, alpha, epsilon, params->no_neurons_l5);
    // ---------------------------------------


    // --- Training loop ---
    int pos_class[2] = {0, 1};
    int neg_class[2] = {1, 0};
    for (unsigned int epoch = 0; epoch < epochs; epoch++)  {
        #ifdef PRINT_PROGRESS
        printf("\n*** Epoch %d ***\n", epoch + 1);
        #endif
        // *************************** LAYER 1 ************************************
        #ifdef PRINT_PROGRESS
        printf("---> Layer 1\n");
        #endif
        training         = true;
        conv_output_len  = (params->in_len - params->filter_len_l1 + 2 * params->padding_l1) / params->stride_l1 + 1;
        conv_output_size = params->no_filters_l1 * conv_output_len;
        output_len       = conv_output_len / params->pool_size_l1;
        output_size      = output_len * params->no_filters_l1;
        filter_size      = params->no_filters_l1 * params->filter_len_l1 * params->in_depth;
        bias_size        = params->no_filters_l1;

        // --- MEMORY ALLOCATION ---
        y_seiz1          = (my_type *) malloc(output_size * sizeof(my_type));
        y_seiz2          = (my_type *) malloc(output_size * sizeof(my_type));
        y_nonseiz1       = (my_type *) malloc(output_size * sizeof(my_type));
        y_nonseiz2       = (my_type *) malloc(output_size * sizeof(my_type));

        dLdw             = (my_type *) malloc(filter_size * sizeof(my_type));
        dLdb             = (my_type *) malloc(bias_size * sizeof(my_type));

        y_relu_seiz1     = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_seiz2     = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_nonseiz1  = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_nonseiz2  = (my_type *) malloc(conv_output_size * sizeof(my_type));

        // --- FORWARD PROPAGATION ---
        fw_layer1: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS
        printf("\t---> Forward Pass\n");
        #endif

        // Seizure 1
        conv1D(x[0], params->filters_l1, params->bias_l1, y_relu_seiz1,
                params->in_len, params->in_depth, params->no_filters_l1, params->filter_len_l1, params->stride_l1, params->padding_l1, true);
        // dropout(y_relu_seiz1, y_relu_seiz1, conv_output_size, params->dropout_l1);
        bn_inference_forward(params->bn_l1, y_relu_seiz1, y_relu_seiz1);
        relu(y_relu_seiz1, y_relu_seiz1, conv_output_size);
        max_pool1D(y_relu_seiz1, y_seiz1, params->pool_size_l1, conv_output_size, output_size);

        // Seizure 2
        conv1D(x[1], params->filters_l1, params->bias_l1, y_relu_seiz2,
                params->in_len, params->in_depth, params->no_filters_l1, params->filter_len_l1, params->stride_l1, params->padding_l1, true);
        // dropout(y_relu_seiz2, y_relu_seiz2, conv_output_size, params->dropout_l1);
        bn_inference_forward(params->bn_l1, y_relu_seiz2, y_relu_seiz2);
        relu(y_relu_seiz2, y_relu_seiz2, conv_output_size);
        max_pool1D(y_relu_seiz2, y_seiz2, params->pool_size_l1, conv_output_size, output_size);

        // Non-seizure 1
        conv1D(x[2], params->filters_l1, params->bias_l1, y_relu_nonseiz1,
                params->in_len, params->in_depth, params->no_filters_l1, params->filter_len_l1, params->stride_l1, params->padding_l1, true);
        // dropout(y_relu_nonseiz1, y_relu_nonseiz1, conv_output_size, params->dropout_l1);
        bn_inference_forward(params->bn_l1, y_relu_nonseiz1, y_relu_nonseiz1);
        relu(y_relu_nonseiz1, y_relu_nonseiz1, conv_output_size);
        max_pool1D(y_relu_nonseiz1, y_nonseiz1, params->pool_size_l1, conv_output_size, output_size);

        // Non-seizure 2
        conv1D(x[3], params->filters_l1, params->bias_l1, y_relu_nonseiz2,
                params->in_len, params->in_depth, params->no_filters_l1, params->filter_len_l1, params->stride_l1, params->padding_l1, true);
        // dropout(y_relu_nonseiz2, y_relu_nonseiz2, conv_output_size, params->dropout_l1);
        bn_inference_forward(params->bn_l1, y_relu_nonseiz2, y_relu_nonseiz2);
        relu(y_relu_nonseiz2, y_relu_nonseiz2, conv_output_size);
        max_pool1D(y_relu_nonseiz2, y_nonseiz2, params->pool_size_l1, conv_output_size, output_size);

        // Do not train - just propagate the output
        if (!training)  {
            free(y_relu_seiz1);     free(y_relu_seiz2);
            free(y_relu_nonseiz1);  free(y_relu_nonseiz2);
            goto layer2;
        }

        // --- BACKWARD PROPAGATION ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Backward Pass\n");
        #endif
        // Loss function: QOID
        update_loss_QOID_Norm1(y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, output_size);
        loss = loss_QOID_Norm1();

        // Gradients
        // Set to 0 the gradients to do cumulative updates per input and avoid storing 4 times the gradients
        memset(dLdw,'\0', filter_size * sizeof(my_type));
        memset(dLdb,'\0', bias_size * sizeof(my_type));

        // Seizure 1 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(x[0], y_relu_seiz1,y_seiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            params->in_len, params->in_depth,
                                            params->no_filters_l1, params->filter_len_l1, params->stride_l1, params->padding_l1, true,
                                            conv_output_len, params->no_filters_l1,
                                            params->bn_l1, params->pool_size_l1, output_len, 0);                               

        // Seizure 2 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(x[1], y_relu_seiz2, y_seiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            params->in_len, params->in_depth,
                                            params->no_filters_l1, params->filter_len_l1, params->stride_l1, params->padding_l1, true,
                                            conv_output_len, params->no_filters_l1,
                                            params->bn_l1, params->pool_size_l1, output_len, 1);
        
        // Non-seizure 1 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(x[2], y_relu_nonseiz1, y_nonseiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            params->in_len, params->in_depth,
                                            params->no_filters_l1, params->filter_len_l1, params->stride_l1, params->padding_l1, true,
                                            conv_output_len, params->no_filters_l1,
                                            params->bn_l1, params->pool_size_l1, output_len, 2);
        
        // Non-seizure 2 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(x[3], y_relu_nonseiz2, y_nonseiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            params->in_len, params->in_depth,
                                            params->no_filters_l1, params->filter_len_l1, params->stride_l1, params->padding_l1, true,
                                            conv_output_len, params->no_filters_l1,
                                            params->bn_l1, params->pool_size_l1, output_len, 3);                                                                                

        // --- ADAM UPDATE STEP ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Adam Update\n");
        #endif
        Adam_optimizer_step(&Adam_filter_l1, params->filters_l1, dLdw);
        Adam_optimizer_step(&Adam_bias_l1, params->bias_l1, dLdb);

        // --- MEMORY CLEANUP BEFORE NEXT LAYER ---
        free(dLdw);         free(dLdb);

        // --- UPDATED INFERENCE TO PROPAGATE ---
        training = false;
        goto fw_layer1;
        // ************************* END OF LAYER 1 *******************************



        // *************************** LAYER 2 ************************************
        layer2: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS
        printf("---> Layer 2\n");
        #endif
        training         = true;
        input_len        = output_len;
        input_depth      = params->no_filters_l1;
        conv_output_len  = (input_len - params->filter_len_l2 + 2 * params->padding_l2) / params->stride_l2 + 1;
        conv_output_size = params->no_filters_l2 * conv_output_len;
        output_len       = conv_output_len / params->pool_size_l2;
        output_size      = output_len * params->no_filters_l2;
        filter_size      = params->no_filters_l2 * params->filter_len_l2 * input_depth;
        bias_size        = params->no_filters_l2;

        // --- PROPAGATE OUTPUT FROM PREVIOUS LAYER ---
        in_seiz1    = y_seiz1;
        in_seiz2    = y_seiz2;
        in_nonseiz1 = y_nonseiz1;
        in_nonseiz2 = y_nonseiz2;

        // --- MEMORY ALLOCATION ---
        y_seiz1         = (my_type *) malloc(output_size * sizeof(my_type));
        y_seiz2         = (my_type *) malloc(output_size * sizeof(my_type));
        y_nonseiz1      = (my_type *) malloc(output_size * sizeof(my_type));
        y_nonseiz2      = (my_type *) malloc(output_size * sizeof(my_type));

        dLdw            = (my_type *) malloc(filter_size * sizeof(my_type));
        dLdb            = (my_type *) malloc(bias_size * sizeof(my_type));

        y_relu_seiz1    = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_seiz2    = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_nonseiz1 = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_nonseiz2 = (my_type *) malloc(conv_output_size * sizeof(my_type));


        // --- FORWARD PROPAGATION ---
        fw_layer2: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS
        printf("\t---> Forward Pass\n");
        #endif
        // Seizure 1
        conv1D(in_seiz1, params->filters_l2, params->bias_l2, y_relu_seiz1,
                input_len, input_depth, params->no_filters_l2, params->filter_len_l2, params->stride_l2, params->padding_l2, true);
        // dropout(y_relu_seiz1, y_relu_seiz1, conv_output_size, params->dropout_l2);
        bn_inference_forward(params->bn_l2, y_relu_seiz1, y_relu_seiz1);
        relu(y_relu_seiz1, y_relu_seiz1, conv_output_size);
        max_pool1D(y_relu_seiz1, y_seiz1, params->pool_size_l2, conv_output_size, output_size);

        // Seizure 2
        conv1D(in_seiz2, params->filters_l2, params->bias_l2, y_relu_seiz2,
                input_len, input_depth, params->no_filters_l2, params->filter_len_l2, params->stride_l2, params->padding_l2, true);
        // dropout(y_relu_seiz2, y_relu_seiz2, conv_output_size, params->dropout_l2);
        bn_inference_forward(params->bn_l2, y_relu_seiz2, y_relu_seiz2);
        relu(y_relu_seiz2, y_relu_seiz2, conv_output_size);
        max_pool1D(y_relu_seiz2, y_seiz2, params->pool_size_l2, conv_output_size, output_size);

        // Non-seizure 1
        conv1D(in_nonseiz1, params->filters_l2, params->bias_l2, y_relu_nonseiz1,
                input_len, input_depth, params->no_filters_l2, params->filter_len_l2, params->stride_l2, params->padding_l2, true);
        // dropout(y_relu_nonseiz1, y_relu_nonseiz1, conv_output_size, params->dropout_l2);
        bn_inference_forward(params->bn_l2, y_relu_nonseiz1, y_relu_nonseiz1);
        relu(y_relu_nonseiz1, y_relu_nonseiz1, conv_output_size);
        max_pool1D(y_relu_nonseiz1, y_nonseiz1, params->pool_size_l2, conv_output_size, output_size);

        // Non-seizure 2
        conv1D(in_nonseiz2, params->filters_l2, params->bias_l2, y_relu_nonseiz2,
                input_len, input_depth, params->no_filters_l2, params->filter_len_l2, params->stride_l2, params->padding_l2, true);
        // dropout(y_relu_nonseiz2, y_relu_nonseiz2, conv_output_size, params->dropout_l2);
        bn_inference_forward(params->bn_l2, y_relu_nonseiz2, y_relu_nonseiz2);
        relu(y_relu_nonseiz2, y_relu_nonseiz2, conv_output_size);
        max_pool1D(y_relu_nonseiz2, y_nonseiz2, params->pool_size_l2, conv_output_size, output_size);

        // Do not train - just propagate the output
        if (!training)  {
            free(in_seiz1);         free(in_seiz2);
            free(in_nonseiz1);      free(in_nonseiz2);
            free(y_relu_seiz1);     free(y_relu_seiz2);
            free(y_relu_nonseiz1);  free(y_relu_nonseiz2);
            goto layer3;
        }

        // --- BACKWARD PROPAGATION ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Backward Pass\n");
        #endif
        // Loss function: QOID
        update_loss_QOID_Norm1(y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, output_size);
        loss = loss_QOID_Norm1();

        // Gradients
        // Set to 0 the gradients to do cumulative updates per input and avoid storing 4 times the gradients
        memset(dLdw,'\0', filter_size * sizeof(my_type));
        memset(dLdb,'\0', bias_size * sizeof(my_type));

        // Seizure 1 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(in_seiz1, y_relu_seiz1, y_seiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            params->no_filters_l2, params->filter_len_l2, params->stride_l2, params->padding_l2, true,
                                            conv_output_len, params->no_filters_l2,
                                            params->bn_l2, params->pool_size_l2, output_len, 0);

        // Seizure 2 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(in_seiz2, y_relu_seiz2, y_seiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            params->no_filters_l2, params->filter_len_l2, params->stride_l2, params->padding_l2, true,
                                            conv_output_len, params->no_filters_l2,
                                            params->bn_l2, params->pool_size_l2, output_len, 1);
    
        // Non-seizure 1 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(in_nonseiz1, y_relu_nonseiz1, y_nonseiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            params->no_filters_l2, params->filter_len_l2, params->stride_l2, params->padding_l2, true,
                                            conv_output_len, params->no_filters_l2,
                                            params->bn_l2, params->pool_size_l2, output_len, 2);
        
        // Non-seizure 2 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(in_nonseiz2, y_relu_nonseiz2, y_nonseiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            params->no_filters_l2, params->filter_len_l2, params->stride_l2, params->padding_l2, true,
                                            conv_output_len, params->no_filters_l2,
                                            params->bn_l2, params->pool_size_l2, output_len, 3);


        // --- ADAM UPDATE STEP ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Adam Update\n");
        #endif
        Adam_optimizer_step(&Adam_filter_l2, params->filters_l2, dLdw);
        Adam_optimizer_step(&Adam_bias_l2, params->bias_l2, dLdb);

        // --- MEMORY CLEANUP BEFORE NEXT LAYER ---
        free(dLdw); free(dLdb);

        // --- UPDATED INFERENCE TO PROPAGATE ---
        training = false;
        goto fw_layer2;

        // ************************* END OF LAYER 2 *******************************



        // *************************** LAYER 3 ************************************
        layer3: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS
        printf("---> Layer 3\n");
        #endif
        training         = true;
        input_len        = output_len;
        input_depth      = params->no_filters_l2;
        conv_output_len  = (input_len - params->filter_len_l3 + 2 * params->padding_l3) / params->stride_l3 + 1;
        conv_output_size = params->no_filters_l3 * conv_output_len;
        output_len       = conv_output_len / params->pool_size_l3;
        output_size      = output_len * params->no_filters_l3;
        filter_size      = params->no_filters_l3 * params->filter_len_l3 * input_depth;
        bias_size        = params->no_filters_l3;

        // --- PROPAGATE OUTPUT FROM PREVIOUS LAYER ---
        in_seiz1         = y_seiz1;
        in_seiz2         = y_seiz2;
        in_nonseiz1      = y_nonseiz1;
        in_nonseiz2      = y_nonseiz2;

        // --- MEMORY ALLOCATION ---
        y_seiz1          = (my_type *) malloc(output_size * sizeof(my_type));
        y_seiz2          = (my_type *) malloc(output_size * sizeof(my_type));
        y_nonseiz1       = (my_type *) malloc(output_size * sizeof(my_type));
        y_nonseiz2       = (my_type *) malloc(output_size * sizeof(my_type));

        dLdw             = (my_type *) malloc(filter_size * sizeof(my_type));
        dLdb             = (my_type *) malloc(bias_size   * sizeof(my_type));

        y_relu_seiz1     = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_seiz2     = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_nonseiz1  = (my_type *) malloc(conv_output_size * sizeof(my_type));
        y_relu_nonseiz2  = (my_type *) malloc(conv_output_size * sizeof(my_type));


        // --- FORWARD PROPAGATION ---
        fw_layer3: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS 
        printf("\t---> Forward Pass\n");
        #endif
        // Seizure 1
        conv1D(in_seiz1, params->filters_l3, params->bias_l3, y_relu_seiz1,
                input_len, input_depth, params->no_filters_l3, params->filter_len_l3, params->stride_l3, params->padding_l3, true);
        // dropout(y_relu_seiz1, y_relu_seiz1, conv_output_size, params->dropout_l3);
        bn_inference_forward(params->bn_l3, y_relu_seiz1, y_relu_seiz1);
        relu(y_relu_seiz1, y_relu_seiz1, conv_output_size);
        max_pool1D(y_relu_seiz1, y_seiz1, params->pool_size_l3, conv_output_size, output_size);

        // Seizure 2
        conv1D(in_seiz2, params->filters_l3, params->bias_l3, y_relu_seiz2,
                input_len, input_depth, params->no_filters_l3, params->filter_len_l3, params->stride_l3, params->padding_l3, true);
        // dropout(y_relu_seiz2, y_relu_seiz2, conv_output_size, params->dropout_l3);
        bn_inference_forward(params->bn_l3, y_relu_seiz2, y_relu_seiz2);
        relu(y_relu_seiz2, y_relu_seiz2, conv_output_size);
        max_pool1D(y_relu_seiz2, y_seiz2, params->pool_size_l3, conv_output_size, output_size);

        // Non-seizure 1
        conv1D(in_nonseiz1, params->filters_l3, params->bias_l3, y_relu_nonseiz1,
                input_len, input_depth, params->no_filters_l3, params->filter_len_l3, params->stride_l3, params->padding_l3, true);
        // dropout(y_relu_nonseiz1, y_relu_nonseiz1, conv_output_size, params->dropout_l3);
        bn_inference_forward(params->bn_l3, y_relu_nonseiz1, y_relu_nonseiz1);
        relu(y_relu_nonseiz1, y_relu_nonseiz1, conv_output_size);
        max_pool1D(y_relu_nonseiz1, y_nonseiz1, params->pool_size_l3, conv_output_size, output_size);

        // Non-seizure 2
        conv1D(in_nonseiz2, params->filters_l3, params->bias_l3, y_relu_nonseiz2,
                input_len, input_depth, params->no_filters_l3, params->filter_len_l3, params->stride_l3, params->padding_l3, true);
        // dropout(y_relu_nonseiz2, y_relu_nonseiz2, conv_output_size, params->dropout_l3);
        bn_inference_forward(params->bn_l3, y_relu_nonseiz2, y_relu_nonseiz2);
        relu(y_relu_nonseiz2, y_relu_nonseiz2, conv_output_size);
        max_pool1D(y_relu_nonseiz2, y_nonseiz2, params->pool_size_l3, conv_output_size, output_size);

        // Do not train - just propagate the output
        if (!training)  {
            free(in_seiz1);         free(in_seiz2);
            free(in_nonseiz1);      free(in_nonseiz2);
            free(y_relu_seiz1);     free(y_relu_seiz2);
            free(y_relu_nonseiz1);  free(y_relu_nonseiz2);
            goto layer4;
        }

        // --- BACKWARD PROPAGATION ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Backward Pass\n");
        #endif
        // Loss function: QOID
        update_loss_QOID_Norm1(y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, output_size);
        loss = loss_QOID_Norm1();

        // Gradients
        // Set to 0 the gradients to do cumulative updates per input and avoid storing 4 times the gradients
        memset(dLdw,'\0', filter_size * sizeof(my_type));
        memset(dLdb,'\0', bias_size   * sizeof(my_type));

        // Seizure 1 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(in_seiz1, y_relu_seiz1, y_seiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            params->no_filters_l3, params->filter_len_l3, params->stride_l3, params->padding_l3, true,
                                            conv_output_len, params->no_filters_l3,
                                            params->bn_l3, params->pool_size_l3, output_len, 0);

        // Seizure 2 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(in_seiz2, y_relu_seiz2, y_seiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            params->no_filters_l3, params->filter_len_l3, params->stride_l3, params->padding_l3, true,
                                            conv_output_len, params->no_filters_l3,
                                            params->bn_l3, params->pool_size_l3, output_len, 1);

        // Non-seizure 1 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(in_nonseiz1, y_relu_nonseiz1, y_nonseiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            params->no_filters_l3, params->filter_len_l3, params->stride_l3, params->padding_l3, true,
                                            conv_output_len, params->no_filters_l3,
                                            params->bn_l3, params->pool_size_l3, output_len, 2);

        // Non-seizure 2 - dLdw, dLdb
        dLdw_dLdb_conv1D_block_QOID_Norm1(in_nonseiz2, y_relu_nonseiz2, y_nonseiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2,
                                            dLdw, dLdb,
                                            input_len, input_depth,
                                            params->no_filters_l3, params->filter_len_l3, params->stride_l3, params->padding_l3, true,
                                            conv_output_len, params->no_filters_l3,
                                            params->bn_l3, params->pool_size_l3, output_len, 3);

        // --- ADAM UPDATE STEP ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Adam Update\n");
        #endif
        Adam_optimizer_step(&Adam_filter_l3, params->filters_l3, dLdw);
        Adam_optimizer_step(&Adam_bias_l3, params->bias_l3, dLdb);

        // --- MEMORY CLEANUP BEFORE NEXT LAYER ---
        free(dLdw); free(dLdb);
        // --- UPDATED INFERENCE TO PROPAGATE ---
        training = false;
        goto fw_layer3;

        // ************************* END OF LAYER 3 *******************************



        // *************************** LAYER 4 ************************************
        layer4: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS
        printf("---> Layer 4\n");
        #endif

        training    = true;
        input_size  = output_size;
        output_size = params->no_neurons_l4;
        filter_size = params->no_neurons_l4 * input_size;
        bias_size   = params->no_neurons_l4;

        // --- PROPAGATE OUTPUT FROM PREVIOUS LAYER ---
        in_seiz1    = y_seiz1;
        in_seiz2    = y_seiz2;
        in_nonseiz1 = y_nonseiz1;
        in_nonseiz2 = y_nonseiz2;

        // --- MEMORY ALLOCATION ---
        y_seiz1     = (my_type *) malloc(params->no_neurons_l4 * sizeof(my_type));
        y_seiz2     = (my_type *) malloc(params->no_neurons_l4 * sizeof(my_type));
        y_nonseiz1  = (my_type *) malloc(params->no_neurons_l4 * sizeof(my_type));
        y_nonseiz2  = (my_type *) malloc(params->no_neurons_l4 * sizeof(my_type));

        dLdw        = (my_type *) malloc(filter_size * sizeof(my_type));
        dLdb        = (my_type *) malloc(bias_size * sizeof(my_type));

        // --- FORWARD PROPAGATION ---
        fw_layer4: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS
        printf("\t---> Forward Pass\n");
        #endif
        // Seizure 1
        fully_connected(in_seiz1, params->weights_l4, params->bias_l4, y_seiz1, input_size, output_size);
        // dropout(y_seiz1, y_seiz1, output_size, params->dropout_l4);

        // Seizure 2
        fully_connected(in_seiz2, params->weights_l4, params->bias_l4, y_seiz2, input_size, output_size);
        // dropout(y_seiz2, y_seiz2, output_size, params->dropout_l4);

        // Non-seizure 1
        fully_connected(in_nonseiz1, params->weights_l4, params->bias_l4, y_nonseiz1, input_size, output_size);
        // dropout(y_nonseiz1, y_nonseiz1, output_size, params->dropout_l4);

        // Non-seizure 2
        fully_connected(in_nonseiz2, params->weights_l4, params->bias_l4, y_nonseiz2, input_size, output_size);
        // dropout(y_nonseiz2, y_nonseiz2, output_size, params->dropout_l4);

        // Do not train - just propagate the output
        if (!training)  {
            free(in_seiz1); free(in_seiz2);
            free(in_nonseiz1); free(in_nonseiz2);
            goto layer5;
        }

        // --- BACKWARD PROPAGATION ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Backward Pass\n");
        #endif
        // Loss function: QOID
        update_loss_QOID_Norm1(y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, output_size);
        loss = loss_QOID_Norm1();

        // Gradients
        // Set to 0 the gradients to do cumulative updates per input and avoid storing 4 times the gradients
        memset(dLdw,'\0', filter_size * sizeof(my_type));
        memset(dLdb,'\0', bias_size * sizeof(my_type));

        // Seizure 1 - dLdw, dLdb
        dLdw_dLdb_fully_connected_QOID_Norm1(in_seiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, dLdw, dLdb, input_size, output_size, 0);
        
        // Seizure 2 - dLdw, dLdb
        dLdw_dLdb_fully_connected_QOID_Norm1(in_seiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, dLdw, dLdb, input_size, output_size, 1);

        // Non-seizure 1 - dLdw, dLdb
        dLdw_dLdb_fully_connected_QOID_Norm1(in_nonseiz1, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, dLdw, dLdb, input_size, output_size, 2);

        // Non-seizure 2 - dLdw, dLdb
        dLdw_dLdb_fully_connected_QOID_Norm1(in_nonseiz2, y_seiz1, y_seiz2, y_nonseiz1, y_nonseiz2, dLdw, dLdb, input_size, output_size, 3);

        // --- ADAM UPDATE STEP ---
        Adam_optimizer_step(&Adam_weights_l4, params->weights_l4, dLdw);
        Adam_optimizer_step(&Adam_bias_l4, params->bias_l4, dLdb);

        // --- MEMORY CLEANUP BEFORE NEXT LAYER ---
        free(dLdw); free(dLdb);

        // --- UPDATED INFERENCE TO PROPAGATE ---
        training = false;
        goto fw_layer4;

        // ************************* END OF LAYER 4 *******************************



        // *************************** LAYER 5 ************************************
        layer5: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS
        printf("---> Layer 5\n");
        #endif
        training    = true;
        input_size  = params->no_neurons_l4;
        output_size = params->no_neurons_l5;
        filter_size = params->no_neurons_l5 * input_size;
        bias_size   = params->no_neurons_l5;
        
        // --- PROPAGATE OUTPUT FROM PREVIOUS LAYER ---
        in_seiz1    = y_seiz1;
        in_seiz2    = y_seiz2;
        in_nonseiz1 = y_nonseiz1;
        in_nonseiz2 = y_nonseiz2;

        // --- MEMORY ALLOCATION ---
        y_seiz1     = (my_type *) malloc(params->no_neurons_l5 * sizeof(my_type));
        y_seiz2     = (my_type *) malloc(params->no_neurons_l5 * sizeof(my_type));
        y_nonseiz1  = (my_type *) malloc(params->no_neurons_l5 * sizeof(my_type));
        y_nonseiz2  = (my_type *) malloc(params->no_neurons_l5 * sizeof(my_type));

        dLdw        = (my_type *) malloc(filter_size * sizeof(my_type));
        dLdb        = (my_type *) malloc(bias_size * sizeof(my_type));

        // --- FORWARD PROPAGATION ---
        fw_layer5: ;    // Used for goto statement
        #ifdef PRINT_PROGRESS
        printf("\t---> Forward Pass\n");
        #endif

        // Seizure 1
        fully_connected(in_seiz1, params->weights_l5, params->bias_l5, y_seiz1, input_size, output_size);
        softmax(y_seiz1, y_seiz1, output_size);

        // Seizure 2
        fully_connected(in_seiz2, params->weights_l5, params->bias_l5, y_seiz2, input_size, output_size);
        softmax(y_seiz2, y_seiz2, output_size);

        // Non-seizure 1
        fully_connected(in_nonseiz1, params->weights_l5, params->bias_l5, y_nonseiz1, input_size, output_size);
        softmax(y_nonseiz1, y_nonseiz1, output_size);

        // Non-seizure 2
        fully_connected(in_nonseiz2, params->weights_l5, params->bias_l5, y_nonseiz2, input_size, output_size);
        softmax(y_nonseiz2, y_nonseiz2, output_size);

        #ifdef PRINT_OUTPUT
        // Print the outputs
        if (!training)  {
            printf("\nSeizure 1: [%f, %f]", y_seiz1[0], y_seiz1[1]);
            printf(" - Loss seizure 1: %f\n", categorical_cross_entropy(pos_class, y_seiz1, output_size) );
            printf("Seizure 2: [%f, %f]", y_seiz2[0], y_seiz2[1]);
            printf(" - Loss seizure 2: %f\n", categorical_cross_entropy(pos_class, y_seiz2, output_size) );
            printf("Non-seizure 1: [%f, %f]", y_nonseiz1[0], y_nonseiz1[1]);
            printf(" - Loss non-seizure 1: %f\n", categorical_cross_entropy(neg_class, y_nonseiz1, output_size) );
            printf("Non-seizure 2: [%f, %f]", y_nonseiz2[0], y_nonseiz2[1]);
            printf(" - Loss non-seizure 2: %f\n", categorical_cross_entropy(neg_class, y_nonseiz2, output_size) );
        }
        #endif
        // Do not train - finish this epoch
        if (!training)  {
            #ifdef PRINT_LOSS
            // Update loss after new FW pass
            loss = (categorical_cross_entropy(pos_class, y_seiz1, output_size)      +
                    categorical_cross_entropy(pos_class, y_seiz2, output_size)      +
                    categorical_cross_entropy(neg_class, y_nonseiz1, output_size)   +
                    categorical_cross_entropy(neg_class, y_nonseiz2, output_size) ) / 4;
           
            printf("Loss after epoch %d: %f\n", epoch + 1, loss);
            #endif
            free(y_seiz1);              free(y_seiz2);
            free(y_nonseiz1);           free(y_nonseiz2);
            free(in_seiz1);             free(in_seiz2);
            free(in_nonseiz1);          free(in_nonseiz2);
            goto end;
        }
        

        // --- BACKWARD PROPAGATION ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Backward Pass\n");
        #endif
        // Loss function: Categoprical Cross-Entropy (CCE)
        // Average the loss of the 4 outputs 
        loss = (categorical_cross_entropy(pos_class, y_seiz1, output_size)      +
                categorical_cross_entropy(pos_class, y_seiz2, output_size)      +
                categorical_cross_entropy(neg_class, y_nonseiz1, output_size)   +
                categorical_cross_entropy(neg_class, y_nonseiz2, output_size) ) / 4;


        // Gradients
        // Set to 0 the gradients to do cumulative updates per input and avoid storing 4 times the gradients
        memset(dLdw,'\0', filter_size * sizeof(my_type));
        memset(dLdb,'\0', bias_size * sizeof(my_type));

        // Seizure 1 - dLdw, dLdb
        dLdw_dLdb_fully_connected_softmax_categorical_crossentropy(in_seiz1, y_seiz1, pos_class, dLdw, dLdb, input_size, output_size, 4);

        // Seizure 2 - dLdw, dLdb
        dLdw_dLdb_fully_connected_softmax_categorical_crossentropy(in_seiz2, y_seiz2, pos_class, dLdw, dLdb, input_size, output_size, 4);

        // Non-seizure 1 - dLdw, dLdb
        dLdw_dLdb_fully_connected_softmax_categorical_crossentropy(in_nonseiz1, y_nonseiz1, neg_class, dLdw, dLdb, input_size, output_size, 4);

        // Non-seizure 2 - dLdw, dLdb
        dLdw_dLdb_fully_connected_softmax_categorical_crossentropy(in_nonseiz2, y_nonseiz2, neg_class, dLdw, dLdb, input_size, output_size, 4);

        // --- ADAM UPDATE STEP ---
        #ifdef PRINT_PROGRESS
        printf("\t---> Adam Update\n");
        #endif
        Adam_optimizer_step(&Adam_weights_l5, params->weights_l5, dLdw);
        Adam_optimizer_step(&Adam_bias_l5, params->bias_l5, dLdb);

        // --- MEMORY CLEANUP ---
        free(dLdw); free(dLdb);

        // --- UPDATED INFERENCE FOR NEW LOSS ---
        training = false;
        goto fw_layer5;

        // ************************* END OF LAYER 5 *******************************

        end: ;    // Used for goto statement

        // Finish this epoch
        #ifdef PRINT_PROGRESS
        printf("******************************\n\n");
        #endif
    
    }

}