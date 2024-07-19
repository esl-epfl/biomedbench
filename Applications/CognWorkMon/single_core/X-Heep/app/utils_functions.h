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
#include <stdint.h>
#include <float.h>
#include <main.h>

void vect_min(my_int *src, my_int size, my_int *dst);
void vect_add(my_int *srcA, my_int *srcB, my_int *dst, my_int size);
void vect_sub(my_int *srcA, my_int *srcB, my_int *dst, my_int size);
void vect_mult(my_int *srcA, my_int *srcB, my_int *dst, my_int size, int8_t n_dec);
void vect_scale(my_int *src, my_int scale, my_int *dst, my_int size, int8_t n_dec);
void vect_power(my_int *src, my_int size, my_int *result, int8_t n_dec);
void vect_copy(int32_t *src, my_int *dst, my_int size);
void get_int_samples(const int32_t *src, my_int *dst, my_int size, int8_t n_dec);
void get_float_samples(const float *src, my_int *dst, my_int size, int8_t n_dec);
void vect_dot_prod(my_int *srcA, my_int *srcB, my_int size, my_int *dst, int8_t n_dec);
void vect_mean(my_int *src, my_int size, my_int *result, int8_t n_dec);
void vect_var(my_int *src, my_int size, my_int *result, int8_t n_dec);
void vect_std(my_int *src, my_int size, my_int *result, int8_t n_dec);
void vect_max(my_int *src, my_int size, my_int *result, int16_t *index);
void vect_conv(my_int *srcA, my_int sizeA, my_int *srcB, my_int sizeB, my_int *dst, int8_t n_dec);
void vect_offset(const my_int *src, my_int offset, my_int *dts, my_int size);
void cmplx_mag_squared(const my_int *src, my_int *dst, uint32_t numSamples, int8_t n_dec);
void convert_to_x(float *src, my_int* dest, my_int size, int8_t n_dec);
void change_bit_depth(my_int *src, my_int *dst, int32_t size, int8_t old_depth, int8_t new_depth);

