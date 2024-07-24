// Porting to X-Heep : Francesco Poluzzi
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

#include "matrix.h"

#include <stdio.h>
#include <stdint.h>


/*
 * Slice given matrix and return a new matrix (allocated in the same memory)
 */
Matrix slice(const Matrix *m, const size_t row_start, const size_t row_stop, const size_t col_start, const size_t col_stop) {
    size_t row_span = row_stop - row_start + 1;
    size_t col_span = col_stop - col_start + 1;

    // Get pointer to (row_start, col_start) cell
    float *new_pt = MAT_DATA(m) + MAT_IDX(m, row_start, col_start);
    // Create matrix s
    Matrix s = {
        .data = new_pt,
        .height = row_span,
        .width = col_span,
        .offset = m->width  // same offset as m
    };

    return s;
}

/*
 * Perform row-wise addition between a matrix and a row vector with same width (parallelization along height)
 */
void add_row_h(const Matrix *m, const Matrix *r) {
    const size_t i_start = 0;
    const size_t i_end = m->height;

    for (size_t i = i_start; i < i_end; i++) {
        for (size_t j = 0; j < m->width; j++) {
            MAT_CELL(m, i, j) += MAT_CELL(r, 0, j);
        }
    }
}

/*
 * Perform row-wise addition between a matrix and a row vector with same width (parallelization along width)
 */
void add_row_w(const Matrix *m, const Matrix *r) {
    const size_t j_start = 0;
    const size_t j_end = m->width;

    for (size_t i = 0; i < m->height; i++) {
        for (size_t j = j_start; j < j_end; j++) {
            MAT_CELL(m, i, j) += MAT_CELL(r, 0, j);
        }
    }
}

/*
 * Perform column-wise subtraction between a matrix and a column vector with same height (parallelization along height)
 */
void sub_col_h(const Matrix *m, const Matrix *c) {
    const size_t i_start = 0;
    const size_t i_end = m->height;

    for (size_t i = i_start; i < i_end; i++) {
        for (size_t j = 0; j < m->width; j++) {
            MAT_CELL(m, i, j) -= MAT_CELL(c, i, 0);
        }
    }
}

/*
 * Perform column-wise subtraction between a matrix and a column vector with same height (parallelization along width)
 */
void sub_col_w(const Matrix *m, const Matrix *c) {
    const size_t j_start = 0;
    const size_t j_end = m->width;

    for (size_t i = 0; i < m->height; i++) {
        for (size_t j = j_start; j < j_end; j++) {
            MAT_CELL(m, i, j) -= MAT_CELL(c, i, 0);
        }
    }
}

/*
 * Prints a matrix to standard output
 */
void print_mat(const Matrix *m) {
    for (size_t i = 0; i < m->height; i++) {
        for (size_t j = 0; j < m->width; j++) {
            printf("%.2f ", MAT_CELL(m, i, j));
        }
        printf("\n");
    }
}
