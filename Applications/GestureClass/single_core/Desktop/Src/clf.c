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


#include <float.h>
#include <string.h>
#include <stdint.h>

#include "clf.h"
#include "shared_buf.h"
#include "matmul_fp32.h"
#include "defines.h"

/*
 * Function executed by each core in the cluster (MLPLight)
 */
static void clf_mlp_light_fn(void *args) {

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


    mlp_light1_w = *mlp_light1_w_l2;
    mlp_light1_b = *mlp_light1_b_l2;
    // Clear memory for results
    memset(act1.data, 0, N_TA * N_MU * sizeof(float));

    size_t i_start = 0;
    size_t i_end = N_MU;
    for (size_t i = i_start; i < i_end; i++) {
        for (size_t j = 0; j < N_TA; j++) {
            for (size_t k = 0; k < N_SAMPLES; k++) {
                asm volatile("Mattia:");
                MAT_CELL(&act1, i, j) += tmp7_data[i * N_SAMPLES + k] == 1 ? MAT_CELL(&mlp_light1_w, j, k) : 0.0f;  // (N_MU x N_SAMPLES) @ (N_TA x N_SAMPLES).T -> (N_MU x N_TA)
            }
        }
    }
    
    // Add bias
    add_row_h(&act1, &mlp_light1_b);  // (N_MU x N_TA) + (1, N_TA) -> (N_MU x N_TA)

    // First layer: ReLU
    for (size_t i = i_start; i < i_end; i++) {
        for (size_t j = 0; j < N_TA; j++) {
            MAT_CELL(&act1, i, j) = MAT_CELL(&act1, i, j) > 0.0f ? MAT_CELL(&act1, i, j) : 0.0f;
        }
    }

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

    mlp_light2_w = *mlp_light2_w_l2;
    mlp_light2_b = *mlp_light2_b_l2;
    // Clear memory for results
    memset(act2.data, 0, N_CA * sizeof(float));

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
    size_t j_start = 0;
    size_t j_end = N_CA;
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

    mlp_light3_w = *mlp_light3_w_l2;
    mlp_light3_b = *mlp_light3_b_l2;
    // Clear memory for results
    memset(act3.data, 0, N_OUT * sizeof(float));

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


/*
 * Classifier start point
 */
void clf_entry(void *args) {
    clf_mlp_light_fn(args);
}
