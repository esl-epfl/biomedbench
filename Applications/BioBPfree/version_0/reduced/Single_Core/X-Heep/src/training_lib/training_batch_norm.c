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



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author:          Dimitrios Samakovlis                                                                        //
// Date:            November 2023                                                                               //
// Description:     Batch normalization layer for training                                                      //
// Training:        y = gamma * (x - mean) / sqrt(variance + epsilon) + beta                                    //
// Moving average:  mean_moving = momentum * mean_moving + (1 - momentum) * mean                                //
// Moving variance: var_moving = momentum * var_moving + (1 - momentum) * var                                   //
// Inference:       y = gamma * (x - mean_moving) / sqrt(variance_moving + epsilon) + beta                      //
// Dimensions:      gamma, beta, (moving_)avg, (moving_)var all are vectors of size channels                    //
// Initialization:  gamma = moving_avg = 1, beta = moving_mean = 0, epsilon / moomentum user initialized        //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Library includes
#include <math.h>
#include <stdlib.h>

// Local includes
#include "training_batch_norm.h"
#include "defines.h"



// MUST CALL THIS TO INSTANTIATE A BATCH NORM LAYER
void bn_init(batch_norm_params_t *batch_norm_params, int channels, int samples, my_type momentum, my_type epsilon)   {
    // Allocate memory for the parameters
    batch_norm_params->mean = (my_type *) malloc(channels * sizeof(my_type));
    batch_norm_params->var = (my_type *) malloc(channels * sizeof(my_type));
    batch_norm_params->mean_moving = (my_type *) malloc(channels * sizeof(my_type));
    batch_norm_params->var_moving = (my_type *) malloc(channels * sizeof(my_type));
    batch_norm_params->gamma = (my_type *) malloc(channels * sizeof(my_type));
    batch_norm_params->beta = (my_type *) malloc(channels * sizeof(my_type));
    // Initialize the parameters
    batch_norm_params->channels = channels;
    batch_norm_params->samples = samples;
    batch_norm_params->momentum = momentum;
    batch_norm_params->epsilon = epsilon;

    // Initialize the parameters
    // Default initialize gamma to 1 and beta to 0
    for (int i = 0; i < channels; i++)  {
        batch_norm_params->gamma[i] = batch_norm_params->var_moving[i] = 1.0;
        batch_norm_params->beta[i] = batch_norm_params->mean_moving[i] = 0.0;
    }
}

void bn_delete_memory(batch_norm_params_t *batch_norm_params)   {
    free(batch_norm_params->mean);
    free(batch_norm_params->var);
    free(batch_norm_params->mean_moving);
    free(batch_norm_params->var_moving);
    free(batch_norm_params->gamma);
    free(batch_norm_params->beta);
}

void bn_init_pretrained(batch_norm_params_t *batch_norm_params, int channels, int samples, my_type *gamma, my_type *beta, my_type *mean, my_type *var, my_type epsilon)   {
    // Initialize the parameters
    batch_norm_params->channels = channels;
    batch_norm_params->samples = samples;
    batch_norm_params->epsilon = epsilon;
    // Set pretrained parameters 
    batch_norm_params->gamma = gamma;
    batch_norm_params->beta = beta;
    batch_norm_params->mean_moving = mean;
    batch_norm_params->var_moving = var;
}

// Per channel: y = gamma * (x - mean) / sqrt(variance + epsilon) + beta
// Also updates moving mean and moving variance
void bn_training_forward(batch_norm_params_t *batch_norm_params, my_type *x, my_type *y)   {
    my_type mean, var, sum;
    int samples = batch_norm_params->samples;

    for (int i = 0; i < batch_norm_params->channels; i++)   {
        // Step 1: Calculate mean for channel i
        sum = 0;
        for (int j = 0; j < samples; j++)   {
            sum += x[i*samples + j];
        }
        mean = sum / samples;
        batch_norm_params->mean[i] = mean;
        // Update moving mean for channel i
        batch_norm_params->mean_moving[i] = batch_norm_params->momentum * batch_norm_params->mean_moving[i] + (1 - batch_norm_params->momentum) * mean;

        // Step 2: Calculate variance per channel
        sum = 0;
        for (int j = 0; j < samples; j++)   {
            sum += (x[i*samples + j] - mean)*(x[i*samples + j] - mean);
        }
        var = sum / samples;
        batch_norm_params->var[i] = var;
        // Update moving mean for channel i
        batch_norm_params->var_moving[i] = batch_norm_params->momentum * batch_norm_params->var_moving[i] + (1 - batch_norm_params->momentum) * var;

        // Step 3: Calculate the normalized output (y = gamma * (x - mean) / sqrt(variance + epsilon) + beta)
        for (int j = 0; j < samples; j++)   {
            #ifdef FLOATS
            y[i*samples + j] = batch_norm_params->gamma[i] * (x[i*samples + j] - mean) / sqrtf(var + batch_norm_params->epsilon) + batch_norm_params->beta[i];
            #else
            y[i*samples + j] = batch_norm_params->gamma[i] * (x[i*samples + j] - mean) / sqrt(var + batch_norm_params->epsilon) + batch_norm_params->beta[i];
            #endif
        }
    }

}

// Per channel: y = gamma * (x - mean_moving) / sqrt(var_moving + epsilon) + beta
void bn_inference_forward(batch_norm_params_t *batch_norm_params, my_type *x, my_type *y)   {
    int samples = batch_norm_params->samples;

    for (int i = 0; i < batch_norm_params->channels; i++)   {
         my_type mean = batch_norm_params->mean_moving[i];
         my_type var = batch_norm_params->var_moving[i];
        // Calculate the normalized output (y = gamma * (x - mean_moving) / sqrt(var_moving + epsilon) + beta)
        for (int j = 0; j < samples; j++)   {
            #ifdef FLOATS
            y[i*samples + j] = batch_norm_params->gamma[i] * (x[i*samples + j] - mean) / sqrtf(var + batch_norm_params->epsilon) + batch_norm_params->beta[i];
            #else
            y[i*samples + j] = batch_norm_params->gamma[i] * (x[i*samples + j] - mean) / sqrt(var + batch_norm_params->epsilon) + batch_norm_params->beta[i];
            #endif
        }

    }

}

// Calculate dydgamma for column i (gamma_i)
// dydgamma has dimension (samples x channels) x channels
// dydgamma_i_bn has dimension samples x channels but we only use part of the column (non-zero elements)
// Memory optimization: dydgamma_i_bn has dimension samples - only store the non-zero elements
void dydgamma_i_bn(batch_norm_params_t *batch_norm_params, my_type *x, my_type *dydgamma, int column_index)    {
    int output_index = 0;
    // Traverse x[i] positions of interest
    for (int i = column_index * batch_norm_params->samples; i < (column_index + 1) * batch_norm_params->samples; i++)   {
        #ifdef FLOATS
        dydgamma[output_index++] = (x[i] - batch_norm_params->mean[column_index]) / sqrtf(batch_norm_params->var[column_index] + batch_norm_params->epsilon);
        #else
        dydgamma[output_index++] = (x[i] - batch_norm_params->mean[column_index]) / sqrt(batch_norm_params->var[column_index] + batch_norm_params->epsilon);
        #endif
    }

}

// Calculate dydbeta for column i (gamma_i)
// dydgamma has dimension (samples x channels) x channels
// dydgamma_i_bn has dimension samples x channels but we only use part of the column (non-zero elements)
// Memory optimization: dydgamma_i_bn has dimension samples - only store the non-zero elements
void dydbeta_i_bn(batch_norm_params_t *batch_norm_params, my_type *dydbeta, int column_index)    {

    for (int i = 0; i < batch_norm_params->samples; i++)   {
        dydbeta[i] = 1.0;
    }

}

// Calculate dydx for column i
// dydx has dimension (samples x channels) x (samples x channels)
// Theoretically dydx_i_bn has dimension samples x channels but we only use part of the column (non-zero elements)
// Memory optimization: dydx_i_bn has dimension samples - only store the non-zero elements
void dydx_i_bn(batch_norm_params_t *batch_norm_params, my_type *x, my_type *dydx, int column_index)    {
    int output_index = 0;
    // Extract parameters for readability
    int samples = batch_norm_params->samples;
    int channel_index = column_index / samples; // ATTENTION: Must find which channel we are referring to 
    my_type gamma = batch_norm_params->gamma[channel_index];
    my_type mean = batch_norm_params->mean[channel_index];
    my_type var = batch_norm_params->var[channel_index];
    my_type epsilon = batch_norm_params->epsilon;

    // Common multiplier in all cases
    #ifdef FLOATS
    my_type multiplier = gamma / ( samples * (var + epsilon) * sqrtf(var + epsilon) );
    #else
    my_type multiplier = gamma / ( samples * (var + epsilon) * sqrt(var + epsilon) );
    #endif
    

    // Find the limits of the channel we are interested in
    int start_index = channel_index * samples;

    for (int i = start_index; i < start_index + samples; i++)   {
        if (i == column_index)
            dydx[output_index++] = multiplier * ( (var + epsilon) * (samples - 1)  - (x[i] - mean) * (x[i] - mean) );
        else
            dydx[output_index++] = -multiplier * ( (var + epsilon) + (x[i] - mean) * (x[column_index] - mean) );
    }

}

// Calculate dydx_ii for column i (if bn layer Frozen)
// Return value is a scalar
// The complete gradient dydx_bn is a matrix with dimensions (samples x channels) but we only return the diagonal elements of each column
my_type dydx_i_bn_frozen(batch_norm_params_t *batch_norm_params, int column_index)    {
    // Extract parameters for readability
    int channel_index = column_index / batch_norm_params->samples;                 // Find which channel we are referring to 

    // Common multiplier in all cases
    // dydx_ii_bn_frozen = gamma / sqrt(var + epsilon)
    #ifdef FLOATS
    return batch_norm_params->gamma[channel_index] / sqrtf(batch_norm_params->var_moving[channel_index] + batch_norm_params->epsilon);
    #else
    return batch_norm_params->gamma[channel_index] / sqrt(batch_norm_params->var_moving[channel_index] + batch_norm_params->epsilon);
    #endif

}


