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


//////////////////////////////////////////////////////
// Author:          Stefano Albini                  //
// Contributions:   Dimitrios Samakovlis            //
// Date:            September 2023                  //
//////////////////////////////////////////////////////



#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <helpers.h>

#include "pmsis.h"

// Costant used in the kurtosis computation using the
// Fisher definition
#define KURT_FISHER_CONST   3  


// Internal functions to support some computations
void _find_max(const float *x, int16_t len, float *max_value, int16_t *max_index);
float _simpson_step(const float *x, float spacing, int16_t start, int16_t end);

/*
    Returns the mean of a sequence of numbers
*/
float vect_mean(const float *x, int16_t len){
    float sum = 0.0;
    for(int16_t i=0; i<len; i++){
        sum += x[i];
    }
    return sum / len;
}

/*
    Computes the element-wise multiplication between x and y arrays,
    the multiplication is stored in r array.
*/
void vect_mult(const float *x, const float *y, int16_t len, float *r){
    for(int16_t i=0; i<len; i++){
        r[i] = x[i] * y[i];
    }
}


/*
    Returns the standard deviation of the given input array
*/
float vect_std(const float *x, int16_t len){
    float mean = vect_mean(x, len);
    float sum = 0.0;

    for(int16_t i=0; i<len; i++){
        sum += (x[i] - mean) * (x[i] - mean);
    }

    return sqrtf(sum / len);
}


/*
    Copies "len" samples from "in" array starting at index "start"
    Destination is "out"
    Notice that the samples are taken from the input starting at 
    index "start" but are placed in output from index 0!
*/
void vect_copy(const float *in, int16_t start, int16_t len, float *out){
    for(int16_t i=0; i<len; i++){
        out[i] = in[i + start];
    }
}


/*
    Subtract a constant to each element of an input array and stores in into res array
*/
void sub_constant(const float *x, int16_t len, float constant, float *res){
    for(int16_t i=0; i<len; i++){
        res[i] = x[i] - constant;
    }
}


/*
    Returns the index at which the maximum value of array x is found
*/
int16_t vect_max_index(const float *x, int16_t len){
    int16_t max_i = 0.0;
    float temp_v;
    _find_max(x, len, &temp_v, &max_i);
    return max_i;
}

/*
    Returns the maximum value within an array
*/
float vect_max_value(const float *x, int16_t len){
    int16_t max_i = 0;
    float max_v;
    _find_max(x, len, &max_v, &max_i);
    return max_v;
}


/*
    Divides every element in the specified "x" array by the maximum
    value.
*/
void normalize_max(const float *x, int16_t len, float *res){
    float max;
    int16_t max_i;
    _find_max(x, len, &max, &max_i);

    for(int16_t i=0; i<len; i++){
        res[i] = x[i] / max;
    }
}


/*
    Helper function for the maximum value and relative index computation
    It stores to max_value and max_index the maximum value and relative index 
    found in the array x, respectively
*/
void _find_max(const float *x, int16_t len, float *max_value, int16_t *max_index){
    float max_v = x[0];
    int16_t max_i = 0;
    for(int16_t i=0; i<len; i++){
        if(x[i] > max_v){
            max_v = x[i];
            max_i = i;
        }
    }

    *max_index = max_i;
    *max_value = max_v;
}


/*
    Returns the integral of signal x coputed using the composite 
    Simpson's rule. The spacing of the samples is defined by the
    "spacing" parameter.
    If the number of samples is even, the integral is averaged
*/
float simpson(const float *x, int16_t len, float spacing){
    float result = 0.0;

    if(len % 2 == 0){
        float val = 0.0;
        val += spacing * (x[len - 1] + x[len - 2]) / 2;
        result += _simpson_step(x, spacing, 0, len-1);

        val += spacing * (x[0] + x[1]) / 2;
        result += _simpson_step(x, spacing, 1, len);

        val /= 2;
        result /= 2;
        result = result + val;
    } else {
        result = _simpson_step(x, spacing, 0, len);
    }
    return result;
}


/*
    Helper function that implements the definition of the composite Simpson's integral.
    This function is not callable externally.
*/
float _simpson_step(const float *x, float spacing, int16_t start, int16_t end){
    
    int n_intervals = (end - start) / 2; // number of intervals (h in the formula)

    float sum = 0.0;
    int interval_start = start;

    // computes the indexes
    for (int i = 0; i < n_intervals; i++)
    {
        sum += x[interval_start] + 4 * x[interval_start+1] + x[interval_start+2];
        interval_start = interval_start + 2;
    }
    return (sum * (spacing / 3));
}

/*
    Applies a padding to the specified signal, it appends 
    "padlen" elements to the left and to the right.
    The padded values are computed differently for the 
    left and the right ends.
*/
void padding(const float *sig, int len, int padlen, float *res){

    float left_end = sig[0];
    float right_end = sig[len-1];

    float *left_ext = (float*)pvPortMalloc(padlen * sizeof(float));
    float *right_ext = (float*)pvPortMalloc(padlen * sizeof(float));

    // compute and append padding for the left side
    for(int i=0; i<padlen; i++){
        left_ext[i] = sig[padlen-i]; 
        right_ext[i] = sig[len-2-i];

        res[i] = (2 * left_end) - left_ext[i];
    }

    // copy the original signal in the central part of the result
    vect_copy(sig, 0, len, &res[padlen]);

    // computes and append padding for the right side
    for(int i=padlen+len; i<(padlen*2)+len; i++){
        res[i] = (2 * right_end) - right_ext[i - (padlen + len)];
    }

    vPortFree(left_ext);
    vPortFree(right_ext);
}

/*
    Adds a 0 padding to the left and right of the input x
    The amount of 0 added to each side is specified by side_pad_len
*/
void zero_padding(const float *x, int16_t len, int16_t side_pad_len, float *r){

    // pad with zeros in front and in the end
    for(int16_t i=0; i<side_pad_len; i++){
        r[i] = 0.0;
        r[i+side_pad_len+len] = 0.0;
    }

    // copy input vector
    vect_copy(x, 0, len, &r[side_pad_len]);
}


/*
    Computes the line length feature of the specified input array
*/
float get_line_length(const float *x, int16_t len){

    float sum = 0.0;

    for(int16_t i=0; i<len-1; i++){
        sum += fabs(x[i+1] - x[i]);
    }

    return sum / (len-1);
}


/*
    Returns the kurtosis of the given input array
*/
float get_kurtosis(const float *x, int16_t len){

    float std = vect_std(x, len);
    float mean = vect_mean(x, len);

    float sum = 0.0;

    for(int16_t i=0; i<len; i++){
        register float tmp = (x[i] - mean) * (x[i] - mean);
        sum += tmp * tmp;
    }

    return (sum / (len * pow(std, 4))) - KURT_FISHER_CONST;

}


float L2_norm(const float *x, int16_t len){
    
    float sum = 0.0;

    for(int16_t i=0; i<len; i++){
        sum += x[i] * x[i];
    }

    return sqrtf(sum);
}
