/*
Copyright 2022 Mattia Orlandi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

///////////////////////////////////////////////////////////////////
// Additional contributions by:                                  //
// - Dimitrios Samakovlis (Embedded System Laboratory - EPFL)    //
///////////////////////////////////////////////////////////////////    

#ifndef MATRIX_H
#define MATRIX_H

#include <stdbool.h>
#include <stddef.h>
#include "am_util_stdio.h"

/*
 * Data type declaration and macros
 */
typedef struct {
    float *data;
    size_t height;
    size_t width;
    size_t offset;
} Matrix;

#define MAT_DATA(m) (((Matrix *) (m))->data)
#define MAT_IDX(m, i, j) (((i) * ((m)->offset)) + (j))
#define MAT_CELL(m, i, j) ((MAT_DATA(m))[MAT_IDX(m, i, j)])


/*
 * Matrix manipulation
 */
Matrix slice(const Matrix *m, size_t row_start, size_t row_stop, size_t col_start, size_t col_stop);

/* 
 * Matrix operations
 */
void add_row_h(const Matrix *m, const Matrix *r);
void add_row_w(const Matrix *m, const Matrix *r);
void sub_col_h(const Matrix *m, const Matrix *c);
void sub_col_w(const Matrix *m, const Matrix *c);

/*
 * Utils
 */
void print_mat(const Matrix *m);

#endif
