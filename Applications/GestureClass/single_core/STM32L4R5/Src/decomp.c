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


#include <string.h>

#include "decomp.h"
#include "shared_buf.h"
#include "matmul_fp32.h"
#include "defines.h"

static Matrix mean_vec;
static Matrix white_mtx;
static Matrix sep_mtx;
static Matrix spike_th;

/*
 * Function executed by each core in the cluster
 */
void decomp_fn(Matrix *emg_l2) {
    // Iterate over Q-long windows
    size_t t = 0;
    while (t < N_SAMPLES) {
        // Take slice of signal
        size_t from = t;
        size_t to = from + Q - 1 < N_SAMPLES ? from + Q - 1 : N_SAMPLES - 1;
        Matrix emg_slice_l2 = slice(emg_l2, from, to, 0, N_CH - 1);  // (Q x N_CH)
        Matrix emg_slice = {
            .data = tmp5_data,
            .height = Q,
            .width = N_CH,
            .offset = N_CH
        };

        emg_slice = emg_slice_l2;
        
        // Preprocessing: extension
        Matrix emg_slice_ext = {
            .data = tmp6_data,
            .height = N_CH_EXT,
            .width = EXT_WIN,
            .offset = EXT_WIN
        };

        size_t q_start = 0;
        size_t q_end = EXT_WIN;
        for (size_t i = 0; i < FE; i++) {
            for (size_t j = 0; j < N_CH; j++) {
                for (size_t q = q_start; q < q_end; q++) {
                    MAT_CELL(&emg_slice_ext, j * FE + i, q) = MAT_CELL(&emg_slice, q + FE - i - 1, j);
                }
            }
        }
        
        // Preprocessing: centering
        sub_col_h(&emg_slice_ext, &mean_vec);  // (N_CH_EXT x EXT_WIN) - (N_CH_EXT x 1) -> (N_CH_EXT x EXT_WIN)

        // Preprocessing: whitening
        Matrix emg_slice_white = {
            .data = tmp5_data,
            .height = N_CH_EXT,
            .width = EXT_WIN,
            .offset = EXT_WIN
        };
        
        // Clear memory for results
        memset(emg_slice_white.data, 0, N_CH_EXT * EXT_WIN * sizeof(float));

        struct matMul_args mm_args1 = {
            .A = white_mtx.data,
            .B = emg_slice_ext.data,
            .C = emg_slice_white.data,
            .N = N_CH_EXT,
            .M = EXT_WIN,
            .K = N_CH_EXT
        };

        mm_unroll_1x4((void *) &mm_args1);  // (N_CH_EXT x N_CH_EXT) @ (N_CH_EXT x EXT_WIN) -> (N_CH_EXT x EXT_WIN)

        // Decomposition
        Matrix muapt_slice = {
            .data = tmp6_data,
            .height = N_MU,
            .width = EXT_WIN,
            .offset = EXT_WIN
        };
        
        // Clear memory for results
        memset(muapt_slice.data, 0, N_MU * EXT_WIN * sizeof(float));

        struct matMul_args mm_args2 = {
            .A = sep_mtx.data,
            .B = emg_slice_white.data,
            .C = muapt_slice.data,
            .N = N_MU,
            .M = EXT_WIN,
            .K = N_CH_EXT
        };

        mm_unroll_1x4((void *) &mm_args2);  // (N_MU x N_CH_EXT) @ (N_CH_EXT x EXT_WIN) -> (N_MU x EXT_WIN)
        
        // Post-processing: spike detection
        for (int i = 0; i < N_MU; i++) {
            for (int q = q_start; q < q_end; q++) {
                tmp7_data[i * N_SAMPLES + t + q] = MAT_CELL(&muapt_slice, i, q) * MAT_CELL(&muapt_slice, i, q) >= MAT_CELL(&spike_th, i, 0) ? 1 : 0;
            }
        }

        t += Q;
    }
}

/*
 * Cluster entry-point
 */
void decomp_entry(void *args) {
    // Unpack arguments
    Matrix *emg =  ((DecompArgs *) args)->emg;
    mean_vec = *( ((DecompArgs *) args)->mean_vec);
    white_mtx = *( ((DecompArgs *) args)->white_mtx);
    sep_mtx = *( ((DecompArgs *) args)->sep_mtx);
    spike_th = *( ((DecompArgs *) args)->spike_th);

    decomp_fn(emg);
}
