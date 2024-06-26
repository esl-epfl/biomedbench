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


///////////////////////////////////////////////////////////////////
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


// Library includes
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// Local includes
#include "utils.h"
#include "training_conv.h"
#include "training_loss_QOID.h"
#include "training_loss_QOID.h"
#include "training_batch_norm.h"


// *** FORWARD PROPAGATION - COMMON CONV LAYERS ***

// ToDo: Optimize this loop (Avoid if with loop reordering + use better indexing)
void conv1D(const my_type *x, my_type *filter, my_type *bias, my_type *output,
            int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding, bool bias_sharing)  {

    int output_index = 0;
    int bias_index = 0;

    // Multiple filters
    for (int w_n = 0; w_n < no_filters; w_n++) {

        // Input indexing -> Specifies where the filter is sitting at a moment
        // Since it is 1D conv we only traverse 1 dimension
        for (int start_index = -padding; start_index <= input_len + (padding - filter_len); start_index += stride) {

            my_type sum = 0;

            // Traverse through filters length - depth (depth of filter matches depth of input map)
            for (int w_i = 0; w_i < filter_len; w_i++) {
                for (int w_j = 0; w_j < input_depth; w_j++) {

                    if (start_index + w_i < input_len && start_index + w_i  > -1) {
                        my_type filter_weight = mem3d(filter, filter_len, input_depth, w_n, w_j, w_i);
                        my_type x_ij = mem2d(x, input_len, w_j, start_index + w_i);
			            sum += filter_weight * x_ij;
                    }

                }
            }

            // Write output after adding bias
            output[output_index] = sum + bias[bias_index];

            // Update indexes
            output_index++;
            if (!bias_sharing)
                bias_index++;
        }

        if (bias_sharing)
            bias_index++;

    }
}


void relu(my_type *x, my_type *y, int size)  {

    for (int i = 0; i < size; i++)  {
        if (x[i] >= 0)
            y[i] = x[i];
        else
            y[i] = 0;
    }

}


void max_pool1D(my_type *x, my_type *y, int pool_size, int x_len, int y_len) {

    int y_index = 0;

    for (int i = 0; i < x_len; i += pool_size) {
        my_type max = x[i];
        for (int j = 1; j < pool_size; j++) {
            if (x[i + j] > max)
                max = x[i + j];
        }
        y[y_index++] = max;
    }
}


void conv1D_relu(const my_type *x, my_type *filter, my_type *bias, my_type *output,
                 int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding, bool bias_sharing)  {

    int output_index = 0;
    int bias_index = 0;

    // Multiple filters
    for (int w_n = 0; w_n < no_filters; w_n++) {

        // Input indexing -> Specifies where the filter is sitting at a moment
        // Since it is 1D conv we only traverse 1 dimension
        for (int start_index = -padding; start_index <= input_len + (padding - filter_len); start_index += stride) {

            my_type sum = 0;

            // Traverse through filters length - depth (depth of filter matches depth of input map)
            for (int w_i = 0; w_i < filter_len; w_i++) {
                for (int w_j = 0; w_j < input_depth; w_j++) {

                    if (start_index + w_i < input_len && start_index + w_i  > -1) {
                        my_type filter_weight = mem3d(filter, filter_len, input_depth, w_n, w_j, w_i);
                        my_type x_ij = mem2d(x, input_len, w_j, start_index + w_i);
			            sum += filter_weight * x_ij;
                    }

                }
            }

            // Add bias
            my_type temp = sum + bias[bias_index];

            // Write output after ReLU
            if (temp >= 0)
                output[output_index] = temp;
            else
                output[output_index] = 0;

            // Update indexes
            output_index++;
            if (!bias_sharing)
                bias_index++;
        }

        if (bias_sharing)
            bias_index++;
    }
}


my_type normalize(my_type *x, my_type *y, int size) {

    my_type x_norm = Euclidean_norm(x, size);

    for (int i = 0; i < size; i++)  {
        y[i] = x[i] / x_norm;
    }

    return x_norm;
}


void dropout(my_type *x, my_type *y, int size, my_type dropout_rate) {
    srand(time(NULL));

    for (int i = 0; i < size; i++)  {
        if (rand() / (my_type) RAND_MAX > dropout_rate)
            y[i] = x[i];
        else
            y[i] = 0;
    }

}

// ************************************************************




// *** GRADIENTS ***

void dydw_column_i_conv1D( const my_type *x, my_type *dydw_column_i,
                            int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding,
                            int output_len, int output_depth, int weight_index) {

    // First find which is the filter index depending on the weight_index
    int filter_index = weight_index / (filter_len * input_depth);

    // Then the relative index of this weight within the filter
    int relative_index = weight_index % (filter_len * input_depth);

    // Then the i, j coordinates of the filter
    int filter_j = relative_index / filter_len;
    int filter_i = relative_index % filter_len;

    int output_index = 0;

    // Potential initial 0s because of other filter
    for (; output_index < filter_index * output_len; output_index++)   {
        dydw_column_i[output_index] = 0;
    }

    // Find which element of the 2D input map were multiplied by this weight
    // Progress by stride
    for (int start_index = -padding; start_index <= input_len + (padding - filter_len); output_index++, start_index += stride)   {

        int input_i = start_index + filter_i;

        // Check if x_ij is within limits - if not it was padding -> 0
        if (input_i >= 0 && input_i < input_len)
            dydw_column_i[output_index] = mem2d(x, input_len, filter_j, input_i);
        else
            dydw_column_i[output_index] = 0;

    }

    // Potential final 0s because of other filters
    for (; output_index < output_len * output_depth; output_index++)   {
        dydw_column_i[output_index] = 0;
    }

}


void dydw_column_i_conv1D_relu( const my_type *x, my_type *y, my_type *dydw_column_i,
                                int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding,
                                int output_len, int output_depth, int weight_index) {

    // First find which is the filter index depending on the weight_index
    int filter_index = weight_index / (filter_len * input_depth);

    // Then the relative index of this weight within the filter
    int relative_index = weight_index % (filter_len * input_depth);

    // Then the i, j coordinates of the filter
    int filter_j = relative_index / filter_len;
    int filter_i = relative_index % filter_len;

    // Output index
    int output_index;

    // Initialize to 0s
    for (output_index = 0; output_index < output_depth * output_len; output_index++)   {
        dydw_column_i[output_index] = 0;
    }

    // Find which element of the 2D input map were multiplied by this weight
    // Progress by stride
    output_index = filter_index * output_len;

    for (int start_index = -padding; start_index <= input_len + (padding - filter_len); output_index++, start_index += stride)   {

        int input_i = start_index + filter_i;
        // Only if y_j != 0 (relu layer) and not padded position
        if (input_i >= 0 && input_i < input_len && y[output_index] != 0.0)
            dydw_column_i[output_index] = mem2d(x, input_len, filter_j, input_i);

    }

}


// ToDo: Optimize by exploiting the fact that the matrix is symmetrical
void dydx_normalization(const my_type *x, my_type *dydx, int size, my_type normalization_factor)  {

    my_type squared_sum = vector_sum_squared(x, size);

    my_type denominator = normalization_factor * normalization_factor * normalization_factor;

    // Fill all matrix
    for (int i = 0; i < size; i++)  {
        for (int j = 0; j < size; j++)  {
            dydx[i * size + j] = - (x[i] * x[j]) / denominator;
        }
    }

    // Correct diagonal
    for (int i = 0; i < size; i++)  {
        dydx[i * size + i] = (squared_sum - x[i] * x[i]) / denominator;
    }

}


// Derivative of normalization layer per column (avoid storing the whole matrix)
void dydx_column_i_normalization(const my_type *x, my_type *dydx, int size, my_type normalization_factor, int column_index)  {

    // Initialize only once and reuse
    static my_type squared_sum, denominator;
    
    if (column_index == 0)  {
        squared_sum = vector_sum_squared(x, size);
        denominator = normalization_factor * normalization_factor * normalization_factor;
    }

    // Fill all matrix
    register my_type multiplier = x[column_index] / denominator;
    for (int i = 0; i < size; i++)  {
        dydx[i] = - x[i] * multiplier;
    }

    dydx[column_index] = (squared_sum - x[column_index] * x[column_index]) / denominator;
    
}

// ************************************************************




// *** EXAMPLES - BACKWARD PROPAGATION FOR COMMON CONV LAYERS ***

void dLdw_conv1D_QOID_Euclidean(const my_type *x, my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *dLdw,
                                int input_len, int input_depth, int no_filters, int filter_len, int stride, int padding,
                                int output_len, int output_depth, int input_index) {

    // Calculate #weight_parameters and size of output
    int weight_size = no_filters * filter_len * input_depth;
    int output_size = output_len * output_depth;

    // Allocate memory for the 2 derivative vectors (Chain rule)
    my_type *dLdy = (my_type *) malloc(sizeof(my_type) * output_size);
    my_type *dydw_column_i = (my_type *) malloc(sizeof(my_type) * output_size);

    // Depending on the index of the input calculate dLdy
    if (input_index == 0)
        dLdy1_QOID_Euclidean(y1, y2, y3, y4, dLdy, output_size);
    else if (input_index == 1)
        dLdy2_QOID_Euclidean(y1, y2, y3, y4, dLdy, output_size);
    else if (input_index == 2)
        dLdy3_QOID_Euclidean(y1, y2, y3, y4, dLdy, output_size);
    else
        dLdy4_QOID_Euclidean(y1, y2, y3, y4, dLdy, output_size);


    // Calculate derivative for each weight
    for (int weight_index = 0; weight_index < weight_size; weight_index++)   {

        dydw_column_i_conv1D( x, dydw_column_i,
                        input_len, input_depth, no_filters, filter_len, stride, padding,
                        output_len, output_depth, weight_index);
        dLdw[weight_index] = vector_by_vector_mul(dLdy, dydw_column_i, output_size);

    }

    // Free memory
    free(dLdy);
    free(dydw_column_i);

}


void dLdb_conv1D_QOID_Euclidean(const my_type *x, my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *dLdb,
                                int output_len, int output_depth, int input_index)  {

    // Calculate output_size = bias_size
    int output_size = output_len * output_depth;

    // Allocate memory for the 2 derivative vectors (Chain rule)
    my_type *dLdy = (my_type *) malloc(sizeof(my_type) * output_size);

    // Depending on the index of the input calculate dLdy
    if (input_index == 0)
        dLdy1_QOID_Euclidean(y1, y2, y3, y4, dLdy, output_size);
    else if (input_index == 1)
        dLdy2_QOID_Euclidean(y1, y2, y3, y4, dLdy, output_size);
    else if (input_index == 2)
        dLdy3_QOID_Euclidean(y1, y2, y3, y4, dLdy, output_size);
    else
        dLdy4_QOID_Euclidean(y1, y2, y3, y4, dLdy, output_size);


    // Calculate derivative for each weight
    for (int i = 0; i < output_size; i++)   {
        dLdb[i] = dLdy[i];
    }

    // Free memory
    free(dLdy);

}


void dLdw_dLdb_conv1D_relu_QOID_Norm1(const my_type *x, my_type *x_norm, my_type *y, my_type *y1, my_type *y2, my_type *y3, my_type *y4,
                                        my_type *dLdw, my_type *dLdb,
                                        my_type normalization_factor, int input_len, int input_depth,
                                        int no_filters, int filter_len, int stride, int padding,
                                        int output_len, int output_depth, int input_index, bool bias_sharing)  {

    // Calculate #weight_parameters and size of output
    int weight_size = no_filters * filter_len * input_depth;
    int output_size = output_len * output_depth;
    int filter_size = filter_len * input_depth;

    // Allocate memory for the 3 derivative vectors (Chain rule)
    my_type *dLdy = (my_type *) malloc(sizeof(my_type) * output_size);

    // dLdy - Derivative of Loss with respect to output y
    // Depending on which input is fed (y1? y2? y3? y4?)
    if (input_index == 0)
        dLdy1_QOID_Norm1(y1, y2, y3, y4, dLdy, output_size);
    else if (input_index == 1)
        dLdy2_QOID_Norm1(y1, y2, y3, y4, dLdy, output_size);
    else if (input_index == 2)
        dLdy3_QOID_Norm1(y1, y2, y3, y4, dLdy, output_size);
    else
        dLdy4_QOID_Norm1(y1, y2, y3, y4, dLdy, output_size);


    my_type *dLdy_relu = (my_type *) malloc(sizeof(my_type) * output_size);
    my_type *dydx_column_i = (my_type *) malloc(sizeof(my_type) * output_size);

    for (int column_index = 0; column_index < output_size; column_index++)   {
        // dydx_column_i - Derivative of normalization layer with respect to its input
        dydx_column_i_normalization(x_norm, dydx_column_i, output_size, normalization_factor, column_index);

        dLdy_relu[column_index] = vector_by_vector_mul(dLdy, dydx_column_i, output_size);

    }

    free(dLdy);
    free(dydx_column_i);

    my_type *dydw_column_i = (my_type *) malloc(sizeof(my_type) * output_size);

    // dLdw = dLdy * dydw
    // dydw - Derivative of (conv1d + ReLU) layer with respect to weights
    // Calculate column by column and do vector by vector multiplication (avoid vector by matrix multiplication)
    for (int weight_index = 0; weight_index < weight_size; weight_index++)   {

        dydw_column_i_conv1D_relu(x, y, dydw_column_i,
                        input_len, input_depth, no_filters, filter_len, stride, padding,
                        output_len, output_depth, weight_index);

        // dLdy * dydw_column_i = dLdw_i
        // No need to multiply whole vectors
        // Just multiply the non-zero elements
        dLdw[weight_index] += vector_by_vector_mul(dLdy_relu + (weight_index / filter_size) * output_len,
                                                dydw_column_i + (weight_index / filter_size) * output_len, output_len);

    }

    free(dydw_column_i);

    // dLdb
    // Avoid vector by matrix multiplication since dydb matrix is sparse with 1s and 0s
    if (!bias_sharing)   {

        // Calculate derivative for each bias parameter
        for (int i = 0; i < output_size; i++)   {
            if (y[i] != 0.0)
                dLdb[i] += dLdy_relu[i];
        }

    }
    else    {

        int output_index = 0;
        // Calculate derivative for each bias parameter
        // With bias sharing there is: output_depth bias parameters
        // And each bias contributes to: output_len elements of output y
        for (int i = 0; i < output_depth; i++)   {
            for (; output_index < (i + 1) * output_len; output_index++)
                if (y[output_index] > 0.0)
                    dLdb[i] += dLdy_relu[output_index];
        }

    }

    // Free memory
    free(dLdy_relu);

}


void dLdw_dLdb_conv1D_block_QOID_Norm1(const my_type *x, my_type *y_relu, my_type *y, my_type *y1, my_type *y2, my_type *y3, my_type *y4,
                                        my_type *dLdw, my_type *dLdb,
                                        int input_len, int input_depth,
                                        int no_filters, int filter_len, int stride, int padding, bool bias_sharing,
                                        int conv_output_len, int output_depth,
                                        batch_norm_params_t *batch_norm_params,
                                        int pool_size, int output_len,
                                        int input_index) {
    
    // Calculate #weight_parameters and size of output
    int weight_size = no_filters * filter_len * input_depth;
    int conv_output_size = conv_output_len * output_depth;
    int output_size = output_len * output_depth;

    // Allocate memory for the 3 derivative vectors (Chain rule)
    my_type *dLdy = (my_type *) malloc(sizeof(my_type) * output_size);

    // dLdy - Derivative of Loss with respect to output y
    // Depending on which input is fed (y1? y2? y3? y4?)
    if (input_index == 0)
        dLdy1_QOID_Norm1(y1, y2, y3, y4, dLdy, output_size);
    else if (input_index == 1)
        dLdy2_QOID_Norm1(y1, y2, y3, y4, dLdy, output_size);
    else if (input_index == 2)
        dLdy3_QOID_Norm1(y1, y2, y3, y4, dLdy, output_size);
    else
        dLdy4_QOID_Norm1(y1, y2, y3, y4, dLdy, output_size);

    my_type *dLdy_bn = (my_type *) malloc(sizeof(my_type) * conv_output_size);

    // dLdy_bn - Derivative of Loss with respect to output y after batch normalization
    // Multiply the derivative with the derivatives of pooling, ReLU layers
    // Exploit sparsity of such layers' derivatives to avoid matrix or vector multiplications 
    for (int index = 0; index < conv_output_size; index++)   {
        // The derivative propagates only if the neuron was selected during max pooling (max pooling derivative) and the neuron is not 0 (ReLU derivative)
        if ( y[index / pool_size] > 0 && y_relu[index] == y[index / pool_size] )
            dLdy_bn[index] = dLdy[index / pool_size];
        else
            dLdy_bn[index] = 0; 
    }

    free(dLdy);

    my_type *dLdy_conv = (my_type *) malloc(sizeof(my_type) * conv_output_size);

    // dLdy_conv - Derivative of Loss with respect to output y after convolution
    // dLdy_conv = dLdy_bn * dydx_bn
    // Multiply the derivative with the derivative of batch normalization layer
    // Do vector by column multiplication to avoid storing matrix for vector by matrix multiplication
    // ToDO: Multiply by element only since each column has only 1 non-zero element!
    for (int column_index = 0; column_index < conv_output_size; column_index++)   {
        dLdy_conv[column_index] = dLdy_bn[column_index] * dydx_i_bn_frozen(batch_norm_params, column_index);
    }

    free(dLdy_bn);

    my_type *dydw_column_i = (my_type *) malloc(sizeof(my_type) * conv_output_size);

    // dLdw = dLdy_conv * dydw
    // dydw - Derivative of (conv1d + ReLU) layer with respect to weights
    // Calculate column by column and do vector by vector multiplication (avoid vector by matrix multiplication)
    
    for (int weight_index = 0; weight_index < weight_size; weight_index++)   {

        dydw_column_i_conv1D(x, dydw_column_i,
                             input_len, input_depth, no_filters, filter_len, stride, padding,
                             conv_output_len, output_depth, weight_index);

        // dLdy * dydw_column_i = dLdw_i
        // No need to multiply whole vectors
        // Just multiply the non-zero elements
        dLdw[weight_index] += vector_by_vector_mul(dLdy_conv, dydw_column_i, conv_output_size);
    }
    
    free(dydw_column_i);

    // dLdb
    // Avoid vector by matrix multiplication since dydb matrix is sparse with 1s and 0s
    if (!bias_sharing)   {
        // Calculate derivative for each bias parameter
        for (int i = 0; i < conv_output_size; i++)   {
            dLdb[i] += dLdy_conv[i];
        }
    }
    else    {

        // Calculate derivative for each bias parameter
        // With bias sharing there is: output_depth bias parameters
        // And each bias contributes to: output_len elements of output y
        for (int i = 0; i < output_depth; i++)   {
            for (int grad_index = i * conv_output_len; grad_index < (i + 1) * conv_output_len; grad_index++)
                dLdb[i] += dLdy_conv[grad_index];
        }

    }
    free(dLdy_conv);                       
}

// ************************************************************