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


#ifndef _HELPERS_H_
#define _HELPERS_H_

#include <inttypes.h>


/*
    Set of general functions to help the computations of features 
*/


float vect_mean(float *x, int16_t len);
void vect_mult(float *x, float *y, int16_t len, float *r);
float vect_std(float *x, int16_t len);
void vect_copy(float *in, int16_t start, int16_t len, float *out);
void sub_constant(float *x, int16_t len, float constant, float *res);
int16_t vect_max_index(float *x, int16_t len);
float vect_max_value(float *x, int16_t len);
void normalize_max(float *x, int16_t len, float *res);
float simpson(float *x, int16_t len, float spacing);
void padding(float *sig, int len, int padlen, float *res);
void zero_padding(float *x, int16_t len, int16_t side_pad_len, float *r);

float get_line_length(float *x, int16_t len);
float get_kurtosis(float *x, int16_t len);
float L2_norm(float *x, int16_t len);

#endif