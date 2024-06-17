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

#include <string.h>
#include "decomp.h"
#include "shared_buf.h"
#include "pulp_train_utils_fp32.h"
#include "pulp_matmul_fp32.h"

#include "defines.h"

static Matrix mean_vec = {
    .data = tmp1_data,
    .height = N_CH_EXT,
    .width = 1,
    .offset = 1
};

static Matrix white_mtx = {
    .data = tmp2_data,
    .height = N_CH_EXT,
    .width = N_CH_EXT,
    .offset = N_CH_EXT
};

static Matrix sep_mtx = {
    .data = tmp3_data,
    .height = N_MU,
    .width = N_CH_EXT,
    .offset = N_CH_EXT
};

static Matrix spike_th = {
    .data = tmp4_data,
    .height = N_MU,
    .width = 1,
    .offset = 1
};

/*
 * Function executed by each core in the cluster
 */
void decomp_fn(void *args) {
    const size_t core_id = pi_core_id();
    Matrix *emg_l2 = (Matrix *) args;

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
        if (core_id == 0) {
            // Move slice to L1
            mat_fc2cl(&emg_slice_l2, &emg_slice);
        }
        pi_cl_team_barrier();
        
        // Preprocessing: extension
        Matrix emg_slice_ext = {
            .data = tmp6_data,
            .height = N_CH_EXT,
            .width = EXT_WIN,
            .offset = EXT_WIN
        };
        size_t q_chunk = (EXT_WIN + NUM_CORES) / NUM_CORES;
        size_t q_start = core_id * q_chunk;
        size_t q_end = q_start + q_chunk < EXT_WIN ? q_start + q_chunk : EXT_WIN;
        for (size_t i = 0; i < FE; i++) {
            for (size_t j = 0; j < N_CH; j++) {
                for (size_t q = q_start; q < q_end; q++) {
                    MAT_CELL(&emg_slice_ext, j * FE + i, q) = MAT_CELL(&emg_slice, q + FE - i - 1, j);
                }
            }
        }
        pi_cl_team_barrier();
        
        // Preprocessing: centering
        sub_col_h(&emg_slice_ext, &mean_vec);  // (N_CH_EXT x EXT_WIN) - (N_CH_EXT x 1) -> (N_CH_EXT x EXT_WIN)

        // Preprocessing: whitening
        Matrix emg_slice_white = {
            .data = tmp5_data,
            .height = N_CH_EXT,
            .width = EXT_WIN,
            .offset = EXT_WIN
        };
        if (core_id == 0) {
            // Clear memory for results
            memset(emg_slice_white.data, 0, N_CH_EXT * EXT_WIN * sizeof(float));
        }
        pi_cl_team_barrier();
        struct matMul_args mm_args1 = {
            .A = white_mtx.data,
            .B = emg_slice_ext.data,
            .C = emg_slice_white.data,
            .N = N_CH_EXT,
            .M = EXT_WIN,
            .K = N_CH_EXT
        };
        mm_unroll_1x8((void *) &mm_args1);  // (N_CH_EXT x N_CH_EXT) @ (N_CH_EXT x EXT_WIN) -> (N_CH_EXT x EXT_WIN)
        
        // Decomposition
        Matrix muapt_slice = {
            .data = tmp6_data,
            .height = N_MU,
            .width = EXT_WIN,
            .offset = EXT_WIN
        };
        if (core_id == 0) {
            // Clear memory for results
            memset(muapt_slice.data, 0, N_MU * EXT_WIN * sizeof(float));
        }
        pi_cl_team_barrier();
        struct matMul_args mm_args2 = {
            .A = sep_mtx.data,
            .B = emg_slice_white.data,
            .C = muapt_slice.data,
            .N = N_MU,
            .M = EXT_WIN,
            .K = N_CH_EXT
        };
        mm_unroll_1x8((void *) &mm_args2);  // (N_MU x N_CH_EXT) @ (N_CH_EXT x EXT_WIN) -> (N_MU x EXT_WIN)

        pi_cl_team_barrier();
        
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
    Matrix *emg_l2 = ((DecompArgs *) args)->emg;
    Matrix *mean_vec_l2 = ((DecompArgs *) args)->mean_vec;
    Matrix *white_mtx_l2 = ((DecompArgs *) args)->white_mtx;
    Matrix *sep_mtx_l2 = ((DecompArgs *) args)->sep_mtx;
    Matrix *spike_th_l2 = ((DecompArgs *) args)->spike_th;

    // Move decomposition parameters to L1
    mat_fc2cl(mean_vec_l2, &mean_vec);
    mat_fc2cl(white_mtx_l2, &white_mtx);
    mat_fc2cl(sep_mtx_l2, &sep_mtx);
    mat_fc2cl(spike_th_l2, &spike_th);
    
#ifdef PROFILING 
    pi_perf_conf(
        (1 << PI_PERF_CYCLES) | (1<<PI_PERF_ACTIVE_CYCLES)
    );
    pi_perf_stop();
    pi_perf_reset();
    pi_perf_start();
#endif

    // Spawn team of parallel processes
    pi_cl_team_fork(NUM_CORES, decomp_fn, (void *) emg_l2);

    pi_perf_stop();

#ifdef PROFILING
    uint32_t cycles_cnt = pi_perf_read(PI_PERF_CYCLES);
#endif
#if defined(PROFILING)
    printf("Clock Cycles: %d (Active: %d)\n", pi_perf_read(PI_PERF_CYCLES), pi_perf_read(PI_PERF_ACTIVE_CYCLES));
#endif
}
