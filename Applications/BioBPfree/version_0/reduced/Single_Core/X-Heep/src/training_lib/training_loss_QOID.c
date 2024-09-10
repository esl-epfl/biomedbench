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

#include "utils.h"
#include "training_loss_QOID.h"


// Keep distances in global variables (reused often!)
static my_type dist_y1y2, dist_y1y3, dist_y1y4, dist_y2y3, dist_y2y4, dist_y3y4, epsilon = 1e-7;
static my_type sum_abs_y1y2, sum_abs_y1y3, sum_abs_y1y4, sum_abs_y2y3, sum_abs_y2y4, sum_abs_y3y4;



// *** QOID Loss using sum of Abs distance (Norm1) ***

void update_loss_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, int output_size)    {

    sum_abs_y1y2 = Sum_Abs_Diff(y1, y2, output_size); dist_y1y2 = sum_abs_y1y2 / output_size;
    sum_abs_y1y3 = Sum_Abs_Diff(y1, y3, output_size); dist_y1y3 = sum_abs_y1y3 / output_size;
    sum_abs_y1y4 = Sum_Abs_Diff(y1, y4, output_size); dist_y1y4 = sum_abs_y1y4 / output_size;
    sum_abs_y2y3 = Sum_Abs_Diff(y2, y3, output_size); dist_y2y3 = sum_abs_y2y3 / output_size;
    sum_abs_y2y4 = Sum_Abs_Diff(y2, y4, output_size); dist_y2y4 = sum_abs_y2y4 / output_size;
    sum_abs_y3y4 = Sum_Abs_Diff(y3, y4, output_size); dist_y3y4 = sum_abs_y3y4 / output_size;

}


// Note: First call update_loss_QOID_Norm1() to update the distances
my_type loss_QOID_Norm1()    {

    my_type loss = dist_y1y2 + dist_y3y4 + (1 / (dist_y1y3 + epsilon)) + (1 / (dist_y1y4 + epsilon)) + (1 / (dist_y2y3 + epsilon)) + (1 / (dist_y2y4 + epsilon));

    return loss;

}


// Must first call update_loss_QOID_Norm1() to update the distances
// *** Note: y1[i] = y2[i]? -> Derivative not defined -> 0 ***
void dLdy1_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size)  {

    // Intermediate variables for better readability
    my_type term, sum;

    // Calculate the 3 terms that arise from the derivative of the loss function
    for (int i = 0; i < output_size; i++)  {

        sum = 0;

        // Term 1
        term = 1.0 / output_size;
        if (y1[i] > y2[i])
            sum += term;
        else if (y1[i] < y2[i])
            sum -= term;

        // Term 2
        //term = -output_size / (sum_abs_y1y3 * sum_abs_y1y3);
        term = -1 / (output_size * (dist_y1y3+epsilon) * (dist_y1y3+epsilon));
        if (y1[i] > y3[i])
            sum += term;
        else if (y1[i] < y3[i])
            sum -= term;

        // Term 3
        //term = -output_size / (sum_abs_y1y4 * sum_abs_y1y4);
        term = -1 / (output_size * (dist_y1y4+epsilon) * (dist_y1y4+epsilon));
        if (y1[i] > y4[i])
            sum += term;
        else if (y1[i] < y4[i])
            sum -= term;

        output[i] = sum;

    }

}


void dLdy2_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size)  {

    // Intermediate variables for better readability
    my_type term, sum;

    // Calculate the 3 terms that arise from the derivative of the loss function
    for (int i = 0; i < output_size; i++)  {

        sum = 0;

        // Term 1
        term = 1.0 / output_size;
        if (y2[i] > y1[i])
            sum += term;
        else if (y2[i] < y1[i])
            sum -= term;

        // Term 2
        //term = -output_size / (sum_abs_y2y3 * sum_abs_y2y3);
        term = -1 / (output_size * (dist_y2y3+epsilon)*(dist_y2y3+epsilon));
        if (y2[i] > y3[i])
            sum += term;
        else if (y2[i] < y3[i])
            sum -= term;

        // Term 3
        //term = -output_size / (sum_abs_y2y4 * sum_abs_y2y4);
        term = -1 / (output_size * (dist_y2y4+epsilon)*(dist_y2y4+epsilon));
        if (y2[i] > y4[i])
            sum += term;
        else if (y2[i] < y4[i])
            sum -= term;

        output[i] = sum;

    }

}


void dLdy3_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size)  {

    // Intermediate variables for better readability
    my_type term, sum;

    // Calculate the 3 terms that arise from the derivative of the loss function
    for (int i = 0; i < output_size; i++)  {

        sum = 0;

        // Term 1
        term = 1.0 / output_size;
        if (y3[i] > y4[i])
            sum += term;
        else if (y3[i] < y4[i])
            sum -= term;

        // Term 2
        //term = -output_size / (sum_abs_y1y3 * sum_abs_y1y3);
        term = -1 / (output_size * (dist_y1y3+epsilon)*(dist_y1y3+epsilon));
        if (y3[i] > y1[i])
            sum += term;
        else if (y3[i] < y1[i])
            sum -= term;

        // Term 3
        //term = -output_size / (sum_abs_y2y3 * sum_abs_y2y3);
        term = -1 / (output_size * (dist_y2y3+epsilon)*(dist_y2y3+epsilon));
        if (y3[i] > y2[i])
            sum += term;
        else if (y3[i] < y2[i])
            sum -= term;

        output[i] = sum;

    }

}


void dLdy4_QOID_Norm1(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size)  {

    // Intermediate variables for better readability
    my_type term, sum;

    // Calculate the 3 terms that arise from the derivative of the loss function
    for (int i = 0; i < output_size; i++)  {

        sum = 0;

        // Term 1
        term = 1.0 / output_size;
        if (y4[i] > y3[i])
            sum += term;
        else if (y4[i] < y3[i])
            sum -= term;

        // Term 2
        //term = -output_size / (sum_abs_y1y4 * sum_abs_y1y4);
        term = -1 / (output_size * (dist_y1y4+epsilon)*(dist_y1y4+epsilon));
        if (y4[i] > y1[i])
            sum += term;
        else if (y4[i] < y1[i])
            sum -= term;

        // Term 3
        //term = -output_size / (sum_abs_y2y4 * sum_abs_y2y4);
        term = -1 / (output_size * (dist_y2y4+epsilon)*(dist_y2y4+epsilon));
        if (y4[i] > y2[i])
            sum += term;
        else if (y4[i] < y2[i])
            sum -= term;

        output[i] = sum;

    }

}

// *********************************************************************************************************************




// *** QOID Loss using Euclidean distance ***

// Keep distances in global variables (reused often!)
static my_type dist_y1y2_Euclidean, dist_y1y3_Euclidean, dist_y1y4_Euclidean, dist_y2y3_Euclidean, dist_y2y4_Euclidean, dist_y3y4_Euclidean;

my_type loss_QOID_Euclidean()    {

    my_type loss = dist_y1y2_Euclidean + dist_y3y4_Euclidean + (1 / dist_y1y3_Euclidean) + (1 / dist_y1y4_Euclidean) + (1 / dist_y2y3_Euclidean) + (1 / dist_y2y4_Euclidean);

    return loss;
}


void update_loss_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, int output_size) {
    dist_y1y2_Euclidean = Euclidean_distance(y1, y2, output_size);
    dist_y1y3_Euclidean = Euclidean_distance(y1, y3, output_size);
    dist_y1y4_Euclidean = Euclidean_distance(y1, y4, output_size);
    dist_y2y3_Euclidean = Euclidean_distance(y2, y3, output_size);
    dist_y2y4_Euclidean = Euclidean_distance(y2, y4, output_size);
    dist_y3y4_Euclidean = Euclidean_distance(y3, y4, output_size);
}


void dLdy1_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size) {

    // Intermediate variable for better readability
    my_type term;

    // Calculate the 3 terms that arise from the derivative of the loss function
    for (int i = 0; i < output_size; i++)  {
        // Term 1
        term = (y1[i] - y2[i]) / dist_y1y2_Euclidean;
        output[i] = term;

        // Term 2
        term = -(y1[i] - y3[i]) / ( dist_y1y3_Euclidean * dist_y1y3_Euclidean * dist_y1y3_Euclidean );
        output[i] += term;

        // Term 3
        term = -(y1[i] - y4[i]) / ( dist_y1y4_Euclidean * dist_y1y4_Euclidean * dist_y1y4_Euclidean );
        output[i] += term;
    }

}

void dLdy2_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size) {

    // Intermediate variable for better readability
    my_type term;

    // Calculate the 3 terms that arise from the derivative of the loss function
    for (int i = 0; i < output_size; i++)  {
        // Term 1
        term = (y2[i] - y1[i]) / dist_y1y2_Euclidean;
        output[i] = term;

        // Term 2
        term = -(y2[i] - y3[i]) / ( dist_y2y3_Euclidean * dist_y2y3_Euclidean * dist_y2y3_Euclidean );
        output[i] += term;

        // Term 3
        term = -(y2[i] - y4[i]) / ( dist_y2y4_Euclidean * dist_y2y4_Euclidean * dist_y2y4_Euclidean );
        output[i] += term;
    }

}

void dLdy3_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size) {

    // Intermediate variable for better readability
    my_type term;

    // Calculate the 3 terms that arise from the derivative of the loss function
    for (int i = 0; i < output_size; i++)  {
        // Term 1
        term = (y3[i] - y4[i]) / dist_y3y4_Euclidean;
        output[i] = term;

        // Term 2
        term = -(y3[i] - y1[i]) / ( dist_y1y3_Euclidean * dist_y1y3_Euclidean * dist_y1y3_Euclidean );
        output[i] += term;

        // Term 3
        term = -(y3[i] - y2[i]) / ( dist_y2y3_Euclidean * dist_y2y3_Euclidean * dist_y2y3_Euclidean );
        output[i] += term;
    }

}

void dLdy4_QOID_Euclidean(my_type *y1, my_type *y2, my_type *y3, my_type *y4, my_type *output, int output_size) {

    // Intermediate variable for better readability
    my_type term;

    // Calculate the 3 terms that arise from the derivative of the loss function
    for (int i = 0; i < output_size; i++)  {
        // Term 1
        term = (y4[i] - y3[i]) / dist_y3y4_Euclidean;
        output[i] = term;

        // Term 2
        term = -(y4[i] - y1[i]) / ( dist_y1y4_Euclidean * dist_y1y4_Euclidean * dist_y1y4_Euclidean );
        output[i] += term;

        // Term 3
        term = -(y4[i] - y2[i]) / ( dist_y2y4_Euclidean * dist_y2y4_Euclidean * dist_y2y4_Euclidean );
        output[i] += term;
    }

}

// *********************************************************************************************************************