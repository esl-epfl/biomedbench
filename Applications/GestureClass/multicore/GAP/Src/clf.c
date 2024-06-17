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

#include <float.h>
#include <string.h>
#include "clf.h"
#include "shared_buf.h"
#include "pulp_train_utils_fp32.h"
#include "pulp_matmul_fp32.h"
#include "defines.h"

/*
 * Function executed by each core in the cluster (MLPLight)
 */
static void clf_mlp_light_fn(void *args) {
    const size_t core_id = pi_core_id();

    // Unpack arguments
    Matrix *mlp_light1_w_l2 = ((MLPLightArgs *) args)->mlp_light1_w;
    Matrix *mlp_light1_b_l2 = ((MLPLightArgs *) args)->mlp_light1_b;
    Matrix *mlp_light2_w_l2 = ((MLPLightArgs *) args)->mlp_light2_w;
    Matrix *mlp_light2_b_l2 = ((MLPLightArgs *) args)->mlp_light2_b;
    Matrix *mlp_light3_w_l2 = ((MLPLightArgs *) args)->mlp_light3_w;
    Matrix *mlp_light3_b_l2 = ((MLPLightArgs *) args)->mlp_light3_b;
    uint8_t *class = ((MLPLightArgs *) args)->class;
    
    // Prepare L1 matrices
    Matrix mlp_light1_b = {
        .data = tmp1_data,
        .height = 1,
        .width = N_TA,
        .offset = N_TA
    };
    Matrix mlp_light2_b = {
        .data = tmp1_data,
        .height = 1,
        .width = N_CA,
        .offset = N_CA
    };
    Matrix mlp_light3_b = {
        .data = tmp1_data,
        .height = 1,
        .width = N_OUT,
        .offset = N_OUT
    };
    Matrix mlp_light1_w = {
        .data = tmp2_data,
        .height = N_TA,
        .width = N_SAMPLES,
        .offset = N_SAMPLES
    };
    Matrix mlp_light2_w = {
        .data = tmp2_data,
        .height = N_CA,
        .width = N_TA * N_MU,
        .offset = N_TA * N_MU
    };
    Matrix mlp_light3_w = {
        .data = tmp2_data,
        .height = N_OUT,
        .width = N_CA,
        .offset = N_CA
    };

    // First layer: FC
    Matrix act1 = {
        .data = tmp3_data,
        .height = N_MU,
        .width = N_TA,
        .offset = N_TA
    };
    if (core_id == 0) {
        // Move weights and biases to L1
        mat_fc2cl(mlp_light1_w_l2, &mlp_light1_w);
        mat_fc2cl(mlp_light1_b_l2, &mlp_light1_b);
        // Clear memory for results
        memset(act1.data, 0, N_TA * N_MU * sizeof(float));
    }
    pi_cl_team_barrier();
    size_t i_chunk = (N_MU + NUM_CORES) / NUM_CORES;
    size_t i_start = core_id * i_chunk;
    size_t i_end = i_start + i_chunk < N_MU ? i_start + i_chunk : N_MU;
    for (size_t i = i_start; i < i_end; i++) {
        for (size_t j = 0; j < N_TA; j++) {
            for (size_t k = 0; k < N_SAMPLES; k++) {
                asm volatile("Mattia:");
                MAT_CELL(&act1, i, j) += tmp7_data[i * N_SAMPLES + k] == 1 ? MAT_CELL(&mlp_light1_w, j, k) : 0.0f;  // (N_MU x N_SAMPLES) @ (N_TA x N_SAMPLES).T -> (N_MU x N_TA)
            }
        }
    }
    pi_cl_team_barrier();
    // Add bias
    add_row_h(&act1, &mlp_light1_b);  // (N_MU x N_TA) + (1, N_TA) -> (N_MU x N_TA)

    // First layer: ReLU
    for (size_t i = i_start; i < i_end; i++) {
        for (size_t j = 0; j < N_TA; j++) {
            MAT_CELL(&act1, i, j) = MAT_CELL(&act1, i, j) > 0.0f ? MAT_CELL(&act1, i, j) : 0.0f;
        }
    }
    pi_cl_team_barrier();

    // Flatten
    act1.height = 1;
    act1.width = N_MU * N_TA;
    act1.offset = N_MU * N_TA;

    // Second layer: FC
    Matrix act2 = {
        .data = tmp4_data,
        .height = 1,
        .width = N_CA,
        .offset = N_CA
    };
    if (core_id == 0) {
        // Move weights and biases to L1
        mat_fc2cl(mlp_light2_w_l2, &mlp_light2_w);
        mat_fc2cl(mlp_light2_b_l2, &mlp_light2_b);
        // Clear memory for results
        memset(act2.data, 0, N_CA * sizeof(float));
    }
    pi_cl_team_barrier();
    struct matMul_args mm_args1 = {
        .A = act1.data,
        .B = mlp_light2_w.data,
        .C = act2.data,
        .N = 1,
        .M = N_CA,
        .K = N_MU * N_TA,
        .trans_B = true
    };
    mm_M_unroll_2x1((void *) &mm_args1);  // (1 x N_MU * N_TA) @ (N_CA x N_MU * N_TA).T -> (1 x N_CA)
    // Add bias
    add_row_w(&act2, &mlp_light2_b);

    // Second layer: ReLU
    size_t j_chunk = (N_CA + NUM_CORES) / NUM_CORES;
    size_t j_start = core_id * j_chunk;
    size_t j_end = j_start + j_chunk < N_CA ? j_start + j_chunk : N_CA;
    for (size_t j = j_start; j < j_end; j++) {
        MAT_CELL(&act2, 0, j) = MAT_CELL(&act2, 0, j) > 0.0f ? MAT_CELL(&act2, 0, j) : 0.0f;
    }

    // Third layer: FC
    Matrix act3 = {
        .data = tmp3_data,
        .height = 1,
        .width = N_OUT,
        .offset = N_OUT
    };
    if (core_id == 0) {
        // Move weights and biases to L1
        mat_fc2cl(mlp_light3_w_l2, &mlp_light3_w);
        mat_fc2cl(mlp_light3_b_l2, &mlp_light3_b);
        // Clear memory for results
        memset(act3.data, 0, N_OUT * sizeof(float));
    }
    pi_cl_team_barrier();
    struct matMul_args mm_args3 = {
        .A = act2.data,
        .B = mlp_light3_w.data,
        .C = act3.data,
        .N = 1,
        .M = N_OUT,
        .K = N_CA,
        .trans_B = true
    };
    mm_M_unroll_2x1((void *) &mm_args3);  // (1 x N_CA) @ (N_OUT x N_CA).T -> (1 x N_OUT)
    // Add bias
    add_row_w(&act3, &mlp_light3_b);

    if (core_id == 0) {
        #ifdef PRINTING
        printf("MLP output: \n");
        #endif
        // Argmax
        size_t j_max = -1;
        float max = -FLT_MAX;
        for (size_t j = 0; j < N_OUT; j++) {
            #ifdef PRINTING
            printf("%f ", MAT_CELL(&act3, 0, j));
            #endif
            if (max < MAT_CELL(&act3, 0, j)) {
                max = MAT_CELL(&act3, 0, j);
                j_max = j;
            }
        }
        *class = j_max;
        #ifdef PRINTING
        printf("\n");
        #endif
    }
}

/*
 * Function executed by each core in the cluster (SVM)
 */
static void clf_svm_fn(void *args) {
    const size_t core_id = pi_core_id();

    // Unpack arguments
    Matrix *svm_coef_l2 = ((SVMArgs *) args)->svm_coef;
    Matrix *svm_intercept_l2 = ((SVMArgs *) args)->svm_intercept;
    uint8_t *class = ((SVMArgs *) args)->class;

    // Prepare L1 matrices
    Matrix svm_coef = {
        .data = tmp1_data,
        .height = N_OUT * (N_OUT - 1) / 2,
        .width = N_MU,
        .offset = N_MU
    };
    Matrix svm_intercept = {
        .data = tmp2_data,
        .height = N_OUT * (N_OUT - 1) / 2,
        .width = 1,
        .offset = 1
    };

    // Count spikes
    Matrix spk_cnt = {
        .data = tmp3_data,
        .height = 1,
        .width = N_MU,
        .offset = N_MU
    };
    if (core_id == 0) {
        // Clear memory for results
        memset(spk_cnt.data, 0, N_MU * sizeof(float));
    }
    size_t j_chunk = (N_SAMPLES + NUM_CORES - 1) / NUM_CORES;
    size_t j_start = core_id * j_chunk;
    size_t j_end = j_start + j_chunk < N_SAMPLES ? j_start + j_chunk : N_SAMPLES;
    for (size_t i = 0; i < N_MU; i++) {
        uint8_t acc = 0;
        for (size_t j = j_start; j < j_end; j++) {
            acc += tmp7_data[i * N_SAMPLES + j];
        }
        MAT_CELL(&spk_cnt, 0, i) = acc;
    }
    pi_cl_team_barrier();

    // Projection
    size_t n_svm = N_OUT * (N_OUT - 1) / 2;
    Matrix proj = {
        .data = tmp4_data,
        .height = 1,
        .width = n_svm,
        .offset = n_svm
    };
    if (core_id == 0) {
        // Move coefficients and intercepts to L1
        mat_fc2cl(svm_coef_l2, &svm_coef);
        mat_fc2cl(svm_intercept_l2, &svm_intercept);
        // Clear memory for results
        memset(tmp5_data, 0, N_OUT * sizeof(uint8_t));
    }
    pi_cl_team_barrier();
    if (core_id == 0) {
        // Clear memory for results
        memset(proj.data, 0, n_svm * sizeof(float));
    }
    struct matMul_args mm_args3 = {
        .A = spk_cnt.data,
        .B = svm_coef.data,
        .C = proj.data,
        .N = 1,
        .M = n_svm,
        .K = N_MU,
        .trans_B = true
    };
    mm_M((void *) &mm_args3);  // (1 x N_MU) @ (n_svm x N_MU).T -> (1 x n_svm)
    add_row_w(&proj, &svm_intercept);

    Matrix votes = {
        .data = tmp3_data,
        .height = 1,
        .width = N_OUT,
        .offset = N_OUT
    };

#ifdef PRINTING
    printf("SVM voting: \n");
#endif
    if (core_id == 0) {
        // Clear memory for results
        memset(votes.data, 0, N_OUT * sizeof(float));
        size_t j = 0;
        for (size_t c1 = 0; c1 < N_OUT; c1++) {
            for (size_t c2 = c1 + 1; c2 < N_OUT; c2++) {
                float r = MAT_CELL(&proj, 0, j++);
#ifdef PRINTING
                printf("%f ", r);
#endif
                if (r > 0) {
                    MAT_CELL(&votes, 0, c1)++;
                } else if (r < 0) {
                    MAT_CELL(&votes, 0, c2)++;
                }
            }
        }
#ifdef PRINTING
        printf("\n");
#endif
        // Argmax
        size_t j_max = -1;
        float max = -FLT_MAX;
        for (size_t j = 0; j < N_OUT; j++) {
            if (max < MAT_CELL(&votes, 0, j)) {
                max = MAT_CELL(&votes, 0, j);
                j_max = j;
            }
        }
        *class = j_max;
    }
}

/*
 * Cluster entry-point
 */
void clf_entry(void *args) {
#ifdef PROFILING
    pi_perf_conf(
        (1 << PI_PERF_CYCLES) | (1<<PI_PERF_ACTIVE_CYCLES)
    );
    pi_perf_stop();
    pi_perf_reset();
    pi_perf_start();
#endif
    
    // Spawn team of parallel processes
#if defined(USE_SVM)
    pi_cl_team_fork(NUM_CORES, clf_svm_fn, args);
#else
    pi_cl_team_fork(NUM_CORES, clf_mlp_light_fn, args);
#endif

#ifdef PROFILING
    pi_perf_stop();

    printf("Clock Cycles: %d (Active: %d)\n", pi_perf_read(PI_PERF_CYCLES), pi_perf_read(PI_PERF_ACTIVE_CYCLES));
#endif
}
