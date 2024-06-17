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


#include <stdio.h>
#include <stdlib.h>
#include "separator.h"
#include "mlp_light.h"
#include "shared_buf.h"
#include "matrix.h"
#include "decomp.h"
#include "clf.h"


// printing/profiling/input data options
#include "defines.h"

#include "pico/multicore.h"
#include "Pico_management/gpio_plus_sleep.h"
#include "Pico_management/multicore.h"

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

// global pointer to pass arguments
void *args;


/*
 * Cluster entry-point
 */
void decomp_entry(void) {
    // Unpack arguments
    Matrix *emg_l2 = ((DecompArgs *) args)->emg;
    Matrix *mean_vec_l2 = ((DecompArgs *) args)->mean_vec;
    Matrix *white_mtx_l2 = ((DecompArgs *) args)->white_mtx;
    Matrix *sep_mtx_l2 = ((DecompArgs *) args)->sep_mtx;
    Matrix *spike_th_l2 = ((DecompArgs *) args)->spike_th;

    // Move decomposition parameters to L1
    mean_vec = *mean_vec_l2;
    white_mtx = *white_mtx_l2;
    sep_mtx = *sep_mtx_l2;
    spike_th = *spike_th_l2;

    pico_barrier(0);

    // Spawn team of parallel processes
    decomp_fn(emg_l2, 0);
}


void multicore_decomp_entry(void)   {
    Matrix *emg_l2 = ((DecompArgs *) args)->emg;
    pico_barrier(1);
    decomp_fn(emg_l2, 1);
}

/*
 * Cluster entry-point
 */
void clf_entry(void) {
    
    clf_mlp_light_fn(args, 0);
}

void multicore_clf_entry(void) {
    
    // Spawn team of parallel processes
    clf_mlp_light_fn(args, 1);
}


static void run_semg_bss() {
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
    
    // Execute decomposition task on the cluster
    DecompArgs decomp_args = {
        .emg = &emg,
        .mean_vec = &mean_vec,
        .white_mtx = &white_mtx,
        .sep_mtx = &sep_mtx,
        .spike_th = &spike_th
    };

    // Call core 1
    args = (void *)&decomp_args;
    multicore_launch_core1(multicore_decomp_entry);
    decomp_entry();
    multicore_reset_core1();

    // Execute classification task on the cluster
    uint8_t class = 0;
#ifdef PRINTING
    printf("\nUsing MLPLight\n");
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


    // Call core 1
    args = (void *)&clf_args;
    multicore_launch_core1(multicore_clf_entry);
    clf_entry();
    multicore_reset_core1();


#ifdef PRINTING
    switch (class) {
        case 0:
            printf("Predicted class: rest\n");
            break;
        case 1:
            printf("Predicted class: hand_open\n");
            break;
        case 2:
            printf("Predicted class: fist\n");
            break;
        case 3:
            printf("Predicted class: index\n");
            break;
        case 4:
            printf("Predicted class: ok\n");
            break;
        default:
            printf("Unexpected class\n");
    }
#endif


}

int main(int argc, char *argv[]) {

    gpio_ini();
    gpio_enable();

    // run application
    run_semg_bss();
    gpio_disable();

    return 0;
}
