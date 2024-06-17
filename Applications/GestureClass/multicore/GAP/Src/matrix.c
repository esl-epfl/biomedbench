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

#include "defines.h"

/*
 * Move matrices from Fabric Controller to Cluster
 */
void mat_fc2cl(const Matrix *m_fc, const Matrix *m_cl) {
    // Transfer data field
    pi_cl_dma_cmd_t copy;
    pi_cl_dma_cmd((uint32_t) MAT_DATA(m_fc), (uint32_t) MAT_DATA(m_cl), m_fc->height * m_fc->width * sizeof(float), PI_CL_DMA_DIR_EXT2LOC, &copy);
    pi_cl_dma_cmd_wait(&copy);
}

/*
 * Move matrices from Cluster to Fabric Controller
 */
void mat_cl2fc(const Matrix *m_cl, const Matrix *m_fc) {
    // Transfer data field
    pi_cl_dma_cmd_t copy;
    pi_cl_dma_cmd((uint32_t) MAT_DATA(m_fc), (uint32_t) MAT_DATA(m_cl), m_cl->height * m_cl->width * sizeof(float), PI_CL_DMA_DIR_LOC2EXT, &copy);
    pi_cl_dma_cmd_wait(&copy);
}

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
    const size_t core_id = pi_core_id();
    const size_t i_chunk = (m->height + NUM_CORES - 1) / NUM_CORES;
    const size_t i_start = core_id * i_chunk;
    const size_t i_end = i_start + i_chunk < m->height ? i_start + i_chunk : m->height;

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
    const size_t core_id = pi_core_id();
    const size_t j_chunk = (m->width + NUM_CORES - 1) / NUM_CORES;
    const size_t j_start = core_id * j_chunk;
    const size_t j_end = j_start + j_chunk < m->width ? j_start + j_chunk : m->width;

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
    const size_t core_id = pi_core_id();
    const size_t i_chunk = (m->height + NUM_CORES - 1) / NUM_CORES;
    const size_t i_start = core_id * i_chunk;
    const size_t i_end = i_start + i_chunk < m->height ? i_start + i_chunk : m->height;

    for (size_t i = i_start; i < i_end; i++) {
        for (size_t j = 0; j < m->width; j++) {
            MAT_CELL(m, i, j) -= MAT_CELL(c, i, 0);
        }
    }

    pi_cl_team_barrier();
}

/*
 * Perform column-wise subtraction between a matrix and a column vector with same height (parallelization along width)
 */
void sub_col_w(const Matrix *m, const Matrix *c) {
    const size_t core_id = pi_core_id();
    const size_t j_chunk = (m->width + NUM_CORES - 1) / NUM_CORES;
    const size_t j_start = core_id * j_chunk;
    const size_t j_end = j_start + j_chunk < m->width ? j_start + j_chunk : m->width;

    for (size_t i = 0; i < m->height; i++) {
        for (size_t j = j_start; j < j_end; j++) {
            MAT_CELL(m, i, j) -= MAT_CELL(c, i, 0);
        }
    }

    pi_cl_team_barrier();
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
