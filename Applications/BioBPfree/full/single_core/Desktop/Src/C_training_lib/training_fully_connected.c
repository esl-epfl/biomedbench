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

// Library includes
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

// Local includes
#include "training_fully_connected.h"
#include "utils.h"
#include "defines.h"
#include "training_loss_QOID.h"



// *** FORWARD PROPAGATION ***

void fully_connected(my_type *x, my_type *weights, my_type *bias, my_type *y,
                    int input_size, int output_size)    {
    
    // y = Wx + b
    for (int i = 0; i < output_size; i++)   {
        my_type sum = 0;
        for (int j = 0; j < input_size; j++)    {
            sum += weights[i*input_size + j] * x[j];
        }
        y[i] = sum + bias[i];
    }

}


void softmax(my_type *x, my_type *y, int input_size)  {
    
    my_type sum = 0.0;
    // First calculate the sum of e^x for the denominator
    for (int i = 0; i < input_size; i++)  {
        #ifdef FLOATS
        sum += expf(x[i]);
        #else 
        sum += exp(x[i]); 
        #endif
    }

    // Then divide each e^x with the total sum
    for (int i = 0; i < input_size; i++)    {
        #ifdef FLOATS
        y[i] = expf(x[i]) / sum;
        #else
        y[i] = exp(x[i]) / sum;
        #endif
    }

}


my_type categorical_cross_entropy(int *y_true, my_type *y_pred, int no_classes)    {

    for (int i = 0; i < no_classes; i++)
        if (y_true[i] == 1)
            #ifdef FLOATS
            return -logf(y_pred[i]);
            #else
            return -log(y_pred[i]);
            #endif
    
    return 0.0;

}

// ************************************************************




// *** GRADIENTS ***

void dLdx_softmax_categorical_crossentropy(int *y_true, my_type *y_pred, my_type *dLdx, int output_size, int batch_size)    {

    for (int i = 0; i < output_size; i++)   {
        if (y_true[i] == 1)
            dLdx[i] = (y_pred[i] - 1.0) / (my_type) batch_size;
        else
            dLdx[i] = y_pred[i] / (my_type) batch_size;
    }

}


void dydw_fully_connected(my_type *x, my_type *dydw, int input_size, int output_size)  {

    int weights_size = input_size * output_size;

    // Writing only the non-0 elements of the Jacobian matrix
    for (int i = 0; i < output_size; i++)   {
        int input_index = 0;
        for (int j = i*input_size; j < (i+1)*input_size; j++)    {
            dydw[i*weights_size + j] = x[input_index++];
        }
    }

}


void dydx_column_i_softmax_(my_type *y, my_type *dydx, int size, int column_index)  {

    for (int i = 0; i < size; i++)  {
        dydx[i] = -y[column_index] * y[i];
    }

    dydx[column_index] =  y[column_index] * (1 - y[column_index]);

}

// ************************************************************




// *** EXAMPLES - BACKWARD PROPAGATION FOR COMMON FULLY-CONNECTED LAYERS *** 

void dLdw_dLdb_fully_connected_QOID_Norm1(my_type *x, my_type *y1, my_type *y2, my_type *y3, my_type *y4,
                                            my_type *dLdw, my_type *dLdb, int input_size, int output_size,  int input_index) {
    
    // Assign bias_size
    int bias_size = output_size;

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

    // No need to do the whole vector by matrix multiplication since matrix is sparse
    // Each dLdy vector by column multiplication involves exactly 1 non-zero element x_i
    int output_index = 0;
    for (int i = 0; i < output_size; i++)
        for (int j = 0; j < input_size; j++) 
            dLdw[output_index++] += dLdy[i] * x[j];
 
    for (int i = 0; i < bias_size; i++)
        dLdb[i] += dLdy[i];
    
    free(dLdy);

}


void dLdw_dLdb_fully_connected_softmax_categorical_crossentropy(my_type *x, my_type *y, int *y_true, my_type *dLdw, my_type *dLdb, 
                                                                int input_size, int output_size,  int batch_size)    {

    // Assign bias_size
    int bias_size = output_size;

    // Allocate memory for the 3 derivative vectors (Chain rule)
    my_type *dLdx_softmax = (my_type *) malloc(sizeof(my_type) * output_size);

    dLdx_softmax_categorical_crossentropy(y_true, y, dLdx_softmax, output_size, batch_size);

    // No need to do the whole vector by matrix multiplication since matrix is sparse
    // Each dLdy vector by column multiplication involves exactly 1 non-zero element x_i
    int output_index = 0;
    for (int i = 0; i < output_size; i++)
        for (int j = 0; j < input_size; j++) 
            dLdw[output_index++] += dLdx_softmax[i] * x[j];
 
    for (int i = 0; i < bias_size; i++)
        dLdb[i] += dLdx_softmax[i];

    // Free memory
    free(dLdx_softmax);

}