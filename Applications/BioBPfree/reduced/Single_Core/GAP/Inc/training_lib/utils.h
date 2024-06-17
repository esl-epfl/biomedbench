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

#ifndef _UTILS_H_
#define _UTILS_H_

#include "defines.h"

#include <stdbool.h>

#define ABS(x) ((x)>0?(x):(-(x)))

// Scalar by vector multiplication (alpha x vec)
// Output: vector of same dimension
void scalar_by_vector_mul(my_type alpha, my_type *vec, my_type *output, int size);


// Average of 4 vectors of same size
// Output: average vector
void vector_by_vector_sum_4(my_type *vec1, my_type *vec2, my_type *vec3, my_type *vec4, my_type *vec_avg, int size);


// Multiplication of vectors (1xn) x (nx1)
// Return: scalar value
my_type vector_by_vector_mul(my_type *vec1, my_type *vec2, int size);


// Elementwise multiplication of vectors with same dimension
void vector_by_vector_mul_elementwise(my_type *vec1, my_type *vec2, my_type *vec_out, int size);


// Elementwise division of vectors with same dimension
// Use epsilon to avoid division by 0 (typically epsilon=1e-8)
void vector_by_vector_div_elementwise(my_type *vec1, my_type *vec2, my_type *vec_out, int size, my_type epsilon);


// Elementwise square root of a vector
void vector_sqrt(my_type *vec, int size);


// Vector by matrix multiplication
void vector_by_matrix_mul(my_type *vec, my_type *mat, my_type *output, int dim1, int dim2);

// Returns the sum of squared elements
my_type vector_sum_squared(const my_type *vec, int size);


// Calculate Euclidean distance of two vectors with the same dimension
// Return: scalar value of distance
my_type Euclidean_distance(my_type *vec1, my_type *vec2, int size);


// Calculate Euclidean norm of vector
// Return: Euclidean norm
my_type Euclidean_norm(my_type *vec, int size);


// Calculate sum of absolute differences
my_type Sum_Abs_Diff(my_type *vec1, my_type *vec2, int size);


// Calculate average of absolute differences
my_type Avg_Abs_Diff(my_type *vec1, my_type *vec2, int size);


// Compare floating-point numbers
// Return: TRUE if within tolerance limits else FALSE
bool compare_floating(my_type f1, my_type f2, my_type tolerance);


#endif
