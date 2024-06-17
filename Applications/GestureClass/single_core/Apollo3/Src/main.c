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

#include "am_util_stdio.h"
#include <stdlib.h>

#include "separator.h"
#include "mlp_light.h"
#include "shared_buf.h"
#include "matrix.h"
#include "decomp.h"
#include "clf.h"
#include "main.h"


// printing/profiling/input data options
#include "defines.h"

// choose input data
#if defined(FIST)
#include "fist.h"
#elif defined(HAND_OPEN)
#include "hand_open.h"
#elif defined(INDEX)
#include "index.h"
#elif defined(OK)
#include "ok.h"
#elif defined(REST)
#include "rest.h"
#endif


void run_semg_bss() {
    // Prepare matrices:
    // 1. Input sEMG signal
    Matrix emg = {
        .data = emg_data,
        .height = N_SAMPLES,
        .width = N_CH,
        .offset = N_CH
    };
    // 2. Decomposition model
    Matrix mean_vec = {
        .data = mean_vec_data,
        .height = N_CH_EXT,
        .width = 1,
        .offset = 1
    };
    Matrix white_mtx = {
        .data = white_mtx_data,
        .height = N_CH_EXT,
        .width = N_CH_EXT,
        .offset = N_CH_EXT
    };
    Matrix sep_mtx = {
        .data = sep_mtx_data,
        .height = N_MU,
        .width = N_CH_EXT,
        .offset = N_CH_EXT
    };
    Matrix spike_th = {
        .data = spike_th_data,
        .height = N_MU,
        .width = 1,
        .offset = 1
    };
    // 3. MLPLight
    Matrix mlp_light1_w = {
        .data = mlp_light1_w_data,
        .height = N_TA,
        .width = N_SAMPLES,
        .offset = N_SAMPLES
    };
    Matrix mlp_light1_b = {
        .data = mlp_light1_b_data,
        .height = N_TA,
        .width = 1,
        .offset = 1
    };
    Matrix mlp_light2_w = {
        .data = mlp_light2_w_data,
        .height = N_CA,
        .width = N_MU * N_TA,
        .offset = N_MU * N_TA
    };
    Matrix mlp_light2_b = {
        .data = mlp_light2_b_data,
        .height = N_CA,
        .width = 1,
        .offset = 1
    };
    Matrix mlp_light3_w = {
        .data = mlp_light3_w_data,
        .height = N_OUT,
        .width = N_CA,
        .offset = N_CA
    };
    Matrix mlp_light3_b = {
        .data = mlp_light3_b_data,
        .height = N_OUT,
        .width = 1,
        .offset = 1
    };
 
    
    // *** DECOMPOSITION ***
    DecompArgs decomp_args = {
        .emg = &emg,
        .mean_vec = &mean_vec,
        .white_mtx = &white_mtx,
        .sep_mtx = &sep_mtx,
        .spike_th = &spike_th
    };

    // decompose signal
    decomp_entry(&decomp_args);

    // *** CLASSIFICATION ***
    uint8_t class = 0;
#ifdef PRINTING
    am_util_stdio_printf("\nMLPLight\n");
#endif
    MLPLightArgs clf_args = {
        .mlp_light1_w = &mlp_light1_w,
        .mlp_light1_b = &mlp_light1_b,
        .mlp_light2_w = &mlp_light2_w,
        .mlp_light2_b = &mlp_light2_b,
        .mlp_light3_w = &mlp_light3_w,
        .mlp_light3_b = &mlp_light3_b,
        .class = &class
    };

    // classify
    clf_entry(&clf_args);

#ifdef PRINTING
    switch (class) {
        case 0:
            am_util_stdio_printf("Predicted class: rest\n");
            break;
        case 1:
            am_util_stdio_printf("Predicted class: hand_open\n");
            break;
        case 2:
            am_util_stdio_printf("Predicted class: fist\n");
            break;
        case 3:
            am_util_stdio_printf("Predicted class: index\n");
            break;
        case 4:
            am_util_stdio_printf("Predicted class: ok\n");
            break;
        default:
            am_util_stdio_printf("Unexpected class\n");
    }
#endif

}
