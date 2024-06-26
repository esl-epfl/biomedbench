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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author:      Daniel Panero                                                                               //
// Date:        June 2024                                                                                   //
// Desciption:  Aggressive optimizations on forward+backward pass on conv1D + batchnorm + ReLU + MaxPool    //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////



#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "training_optimized_conv_bn_relu_maxpool.h"

static my_type epsilon = 1e-7;
static my_type terms[6];

static uint8_t pack_bools_to_uint_8(bool bools[8])
{
    uint8_t uint8 = 0;
    for (int index = 0; index < 8; index++)
    {
        if (bools[index])
        {
            uint8 |= 0b1 << index;
        }
    }

    return uint8;
}

static void unpack_uint8_t_to_bools(uint8_t uint8, bool *bools)
{
    for (int index = 0; index < 8; index++)
    {
        bools[index] = (uint8 & 0b1 << index) >> index;
    }
}

static uint16_t pack_nibbles_to_uint_16(uint8_t *nibbles)
{
    uint16_t uint16 = 0;
    for (int index = 0; index < 4; index++)
    {
        uint16 |= nibbles[index] << 4 * index;
    }

    return uint16;
}

static void unpack_uint_16_to_nibbles(uint16_t uint16, uint8_t *nibbles)
{
    for (int index = 0; index < 4; index++)
    {
        nibbles[index] = (uint16 & 0b1111 << 4 * index) >> 4 * index;
    }
}


// Forward 1D + dLdy_maxpool with the loop of convolution
my_type forward_dLdy_conv1d_bn_relu_maxpool_4(const my_type *x[4], struct dLdy_maxpool_dy_maxpool_dy_conv_t *dLdy, my_type *filters, my_type *biases, batch_norm_params_t *batch_norm_params, int input_len, int input_depth,
                           int no_filters, int filter_len, int stride, int padding, bool bias_sharing, int maxpool_len, int output_size)
{
    int output_index = 0;
    int bias_index = 0;

    my_type sum_abs_yy[6] = {0};

    for (int filter_index = 0; filter_index < no_filters; filter_index++)
    {
        int maxpool_index = 0;
        uint8_t maxpool_selected_index[4] = {4, 4, 4, 4}; // 4 in the case, no input was selected (all of the items in the maxpool or relu were rejected)
        my_type maxpool_selected[4] = {0, 0, 0, 0};

        for (int column_index = 0; column_index < input_len; column_index++)
        {
            // 1. Convolution
            my_type y_current[4] = {0, 0, 0, 0};

            int h_index_start = -(column_index - padding);
            int h_index_end = input_len - (column_index - padding);

            if (h_index_start < 0)
                h_index_start = 0;

            if (h_index_end > filter_len)
                h_index_end = filter_len;

            // for (int h_index = 0; h_index < filter_len; h_index++)
            for (int depth_index = 0; depth_index < input_depth; depth_index++)
            {
                for (int h_index = h_index_start; h_index < h_index_end; h_index++)
                // for (int depth_index = 0; depth_index < input_depth; depth_index++)
                {
                    int relative_index = h_index * stride + column_index - padding;
                    my_type h_current = filters[filter_index * (filter_len * input_depth) + depth_index * (filter_len) + h_index];

                    for (int vector_index = 0; vector_index < 4; vector_index++)
                    {
                        my_type x_current = x[vector_index][depth_index * (input_len) + relative_index];
                        y_current[vector_index] += h_current * x_current;
                    }
                }
            }

            for (int vector_index = 0; vector_index < 4; vector_index++)
            {
                y_current[vector_index] += biases[bias_index];
                // 2. Batch normalization

#ifdef FLOATS
                y_current[vector_index] = batch_norm_params->gamma[filter_index] * (y_current[vector_index] - batch_norm_params->mean_moving[filter_index]) / sqrtf(batch_norm_params->var_moving[filter_index] + batch_norm_params->epsilon) + batch_norm_params->beta[filter_index];
#else
                y_current[vector_index] = batch_norm_params->gamma[filter_index] * (y_current[vector_index] - batch_norm_params->mean_moving[filter_index]) / sqrt(batch_norm_params->var_moving[filter_index] + batch_norm_params->epsilon) + batch_norm_params->beta[filter_index];
#endif

                // 3. ReLu
                if (y_current[vector_index] <= 0)
                    y_current[vector_index] = 0;

                // 4. Maxpool
                if (y_current[vector_index] > maxpool_selected[vector_index])
                {
                    maxpool_selected[vector_index] = y_current[vector_index];
                    maxpool_selected_index[vector_index] = maxpool_index;
                }
            }

            if (!bias_sharing)
                bias_index++;

            maxpool_index++;

            if (maxpool_index == maxpool_len)
            {
                // Filling dLdy
                uint8_t dy_maxpool_dy_conv[4];
                bool eqs[8] = {false};
                bool gts[8] = {false};
                int k = 0;
                for (int i = 0; i < 4; i++)
                {
                    dy_maxpool_dy_conv[i] = maxpool_selected_index[i];

                    for (int j = i + 1; j < 4; j++)
                    {
                        eqs[k] = maxpool_selected[i] == maxpool_selected[j];
                        gts[k] = maxpool_selected[i] > maxpool_selected[j];
                        sum_abs_yy[k] += ABS(maxpool_selected[i] - maxpool_selected[j]);
                        k++;
                    }
                }

                dLdy[output_index].dy_maxpool_dy_conv = pack_nibbles_to_uint_16(dy_maxpool_dy_conv);
                dLdy[output_index].eqs = pack_bools_to_uint_8(eqs);
                dLdy[output_index].gts = pack_bools_to_uint_8(gts);

                maxpool_index = 0;
                for (int vector_index = 0; vector_index < 4; vector_index++)
                {
                    maxpool_selected[vector_index] = 0;
                    maxpool_selected_index[vector_index] = 4;
                }
                output_index++;
            }
        }

        if (bias_sharing)
            bias_index++;
    }

    my_type dist_yy[6] = {0};
    for (int i = 0; i < 6; i++)
    {
        dist_yy[i] = sum_abs_yy[i] / output_size;
    }

    terms[0] = 1.0 / output_size;
    terms[1] = -1 / (output_size * (dist_yy[1] + epsilon) * (dist_yy[1] + epsilon));
    terms[2] = -1 / (output_size * (dist_yy[2] + epsilon) * (dist_yy[2] + epsilon));
    terms[3] = -1 / (output_size * (dist_yy[3] + epsilon) * (dist_yy[3] + epsilon));
    terms[4] = -1 / (output_size * (dist_yy[4] + epsilon) * (dist_yy[4] + epsilon));
    terms[5] = 1.0 / output_size;

    my_type loss = dist_yy[0] + dist_yy[5] + (1 / (dist_yy[1] + epsilon)) + (1 / (dist_yy[2] + epsilon)) + (1 / (dist_yy[3] + epsilon)) + (1 / (dist_yy[4] + epsilon));

    return loss;
}

// Forward 1D
void forward_output_conv1d_bn_relu_maxpool_4(const my_type *x[4], my_type *y[4], my_type *filters, my_type *biases, batch_norm_params_t *batch_norm_params, int input_len, int input_depth,
                               int no_filters, int filter_len, int stride, int padding, bool bias_sharing, int maxpool_len, int output_size)
{
    int output_index = 0;
    int bias_index = 0;

    for (int filter_index = 0; filter_index < no_filters; filter_index++)
    {
        int maxpool_index = 0;
        my_type maxpool_selected[4] = {0, 0, 0, 0};

        for (int column_index = 0; column_index < input_len; column_index++)
        {
            // 1. Convolution
            my_type y_current[4] = {0, 0, 0, 0};

            int h_index_start = -(column_index - padding);
            int h_index_end = input_len - (column_index - padding);

            if (h_index_start < 0)
                h_index_start = 0;

            if (h_index_end > filter_len)
                h_index_end = filter_len;

            // for (int h_index = 0; h_index < filter_len; h_index++)
            for (int depth_index = 0; depth_index < input_depth; depth_index++)
            {
                for (int h_index = h_index_start; h_index < h_index_end; h_index++)
                // for (int depth_index = 0; depth_index < input_depth; depth_index++)
                {
                    int relative_index = h_index * stride + column_index - padding;
                    my_type h_current = filters[filter_index * (filter_len * input_depth) + depth_index * (filter_len) + h_index];

                    for (int vector_index = 0; vector_index < 4; vector_index++)
                    {
                        my_type x_current = x[vector_index][depth_index * (input_len) + relative_index];
                        y_current[vector_index] += h_current * x_current;
                    }
                }
            }

            for (int vector_index = 0; vector_index < 4; vector_index++)
            {
                y_current[vector_index] += biases[bias_index];
                // 2. Batch normalization

#ifdef FLOATS
                y_current[vector_index] = batch_norm_params->gamma[filter_index] * (y_current[vector_index] - batch_norm_params->mean_moving[filter_index]) / sqrtf(batch_norm_params->var_moving[filter_index] + batch_norm_params->epsilon) + batch_norm_params->beta[filter_index];
#else
                y_current[vector_index] = batch_norm_params->gamma[filter_index] * (y_current[vector_index] - batch_norm_params->mean_moving[filter_index]) / sqrt(batch_norm_params->var_moving[filter_index] + batch_norm_params->epsilon) + batch_norm_params->beta[filter_index];
#endif

                // 3. ReLu
                if (y_current[vector_index] <= 0)
                    y_current[vector_index] = 0;

                // 4. Maxpool
                if (y_current[vector_index] > maxpool_selected[vector_index])
                {
                    maxpool_selected[vector_index] = y_current[vector_index];
                }
            }

            if (!bias_sharing)
                bias_index++;

            maxpool_index++;

            if (maxpool_index == maxpool_len)
            {
                for (int vector_index = 0; vector_index < 4; vector_index++)
                {
                    y[vector_index][output_index] = maxpool_selected[vector_index];
                }

                // Resetting maxpooling
                maxpool_index = 0;
                for (int vector_index = 0; vector_index < 4; vector_index++)
                {
                    maxpool_selected[vector_index] = 0;
                }
                output_index++;
            }
        }

        if (bias_sharing)
            bias_index++;
    }
}


static void dLdy_maxpool_QOID_Norm1(my_type dLdy_maxpool[4], bool eqs[8], bool gts[8])
{
    int k = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = i + 1; j < 4; j++)
        {
            if (!eqs[k])
            {
                if (gts[k])
                {
                    dLdy_maxpool[i] += terms[k];
                    dLdy_maxpool[j] -= terms[k];
                }
                else
                {
                    dLdy_maxpool[i] -= terms[k];
                    dLdy_maxpool[j] += terms[k];
                }
            }
            k++;
        }
    }
}


void backward_optimized_conv1d_bn_relu_maxpool(const my_type *x[4], my_type *dLdw, my_type *dLdb, struct dLdy_maxpool_dy_maxpool_dy_conv_t *dLdy, batch_norm_params_t *batch_norm_params,
                         int input_len, int input_depth,
                         int no_filters, int filter_len, int stride, int padding, bool bias_sharing, int maxpool_len, int conv_output_len, int output_len)
{
    for (int filter_index = 0; filter_index < no_filters; filter_index++)
    {
        for (int row_index = 0; row_index < output_len; row_index++)
        {
            struct dLdy_maxpool_dy_maxpool_dy_conv_t dLdy_maxpool_dy_maxpool_dy_conv_current = dLdy[filter_index * (output_len) + row_index];

            uint8_t dy_maxpool_dy_conv[4] = {0};
            bool eqs[8] = {false}, gts[8] = {false};

            unpack_uint_16_to_nibbles(dLdy_maxpool_dy_maxpool_dy_conv_current.dy_maxpool_dy_conv, dy_maxpool_dy_conv);
            unpack_uint8_t_to_bools(dLdy_maxpool_dy_maxpool_dy_conv_current.eqs, eqs);
            unpack_uint8_t_to_bools(dLdy_maxpool_dy_maxpool_dy_conv_current.gts, gts);

            my_type dLdy_maxpool[4] = {0};
            dLdy_maxpool_QOID_Norm1(dLdy_maxpool, eqs, gts);

            for (int vector_index = 0; vector_index < 4; vector_index++)
            {
                my_type dLdy_maxpool_current = dLdy_maxpool[vector_index];
                uint8_t dy_maxpool_dy_conv_index_current = dy_maxpool_dy_conv[vector_index];

                if (dy_maxpool_dy_conv_index_current == maxpool_len || dLdy_maxpool_current == 0)
                    continue;

                if (bias_sharing)
                {
                    dLdb[filter_index] += dLdy_maxpool_current;
                }
                else
                {
                    dLdb[filter_index * conv_output_len + row_index * maxpool_len + dy_maxpool_dy_conv_index_current] += dLdy_maxpool_current;
                }

                int column_start = -(row_index * maxpool_len + dy_maxpool_dy_conv_index_current - padding);
                int column_end = input_len - (row_index * maxpool_len + dy_maxpool_dy_conv_index_current - padding);

                if (column_start < 0)
                    column_start = 0;

                if (column_end > filter_len)
                    column_end = filter_len;

                // Sum of rows by summing all the elements multiplied by dLdy_maxpool
                for (int depth_index = 0; depth_index < input_depth; depth_index++)
                {
                    for (int column_index = column_start; column_index < column_end; column_index += stride)
                    {
                        int relative_index = row_index * maxpool_len + dy_maxpool_dy_conv_index_current + column_index - padding;

                        my_type dy_conv_dw_current = x[vector_index][depth_index * (input_len) + relative_index];
                        dLdw[filter_index * (filter_len * input_depth) + depth_index * (filter_len) + column_index] += dLdy_maxpool_current * dy_conv_dw_current;
                    }
                }
            }
        }
    }

    // Grouped multiplication by batch normalization coefficient
    for (int filter_index = 0; filter_index < no_filters; filter_index++)
    {
#ifdef FLOATS
        my_type bn_coefficient = batch_norm_params->gamma[filter_index] / sqrtf(batch_norm_params->var_moving[filter_index] + batch_norm_params->epsilon);
#else
        my_type bn_coefficient = batch_norm_params->gamma[filter_index] / sqrt(batch_norm_params->var_moving[filter_index] + batch_norm_params->epsilon);
#endif

        if (bias_sharing)
        {
            dLdb[filter_index] *= bn_coefficient;
        }
        else
        {
            for (int bias_index = 0; bias_index < conv_output_len; bias_index++)
            {
                dLdb[filter_index * conv_output_len + bias_index] *= bn_coefficient;
            }
        }

        for (int h_index = 0; h_index < input_depth * filter_len; h_index++)
        {
            dLdw[filter_index * (filter_len * input_depth) + h_index] *= bn_coefficient;
        }
    }
}


static void Adam_step_elementwise(Adam_parameters *Adam, my_type *dL, my_type *values, int size)
{
    for (int i = 0; i < size; i++)
    {
        Adam->m[i] = Adam->beta1 * Adam->m[i] + (1 - Adam->beta1) * dL[i];
        Adam->u[i] = Adam->beta2 * Adam->u[i] + (1 - Adam->beta2) * dL[i] * dL[i];

#ifdef FLOATS
        values[i] -= Adam->alpha * (Adam->m[i] / (1 - Adam->beta1_t)) / (sqrtf(Adam->u[i] / (1 - Adam->beta2_t)) + epsilon);
#else
        values[i] -= Adam->alpha * (Adam->m[i] / (1 - Adam->beta1_t)) / (sqrt(Adam->u[i] / (1 - Adam->beta2_t)) + epsilon);
#endif
    }
}


void Adam_step_optimized(my_type *filters, my_type *biases, my_type *dLdw, my_type *dLdb, Adam_parameters *Adam_filter, Adam_parameters *Adam_bias, int filter_size, int bias_size)
{

    Adam_step_elementwise(Adam_filter, dLdw, filters, filter_size);
    Adam_step_elementwise(Adam_bias, dLdb, biases, bias_size);

    Adam_filter->beta1_t *= Adam_filter->beta1;
    Adam_filter->beta2_t *= Adam_filter->beta2;

    Adam_bias->beta1_t *= Adam_bias->beta1;
    Adam_bias->beta2_t *= Adam_bias->beta2;
}
