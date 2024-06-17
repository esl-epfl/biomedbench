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
#include "svm.h"
#include "shared_buf.h"
#include "matrix.h"
#include "decomp.h"
#include "clf.h"

#include <pmsis.h>

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

#ifdef GAP9
#include "GAP9/peripherals_sleep.h"
#elif defined(GAPUINO)
#include "GAPuino/gpio_mgmt.h"
#endif

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
    // 4. SVM
    Matrix svm_coef = {
        .data = svm_coef_data,
        .height = N_OUT * (N_OUT - 1) / 2,
        .width = N_MU,
        .offset = N_MU
    };
    Matrix svm_intercept = {
        .data = svm_intercept_data,
        .height = N_OUT * (N_OUT - 1) / 2,
        .width = 1,
        .offset = 1
    };
    

    // Initialize cluster device
    struct pi_cluster_conf cl_conf;
    pi_cluster_conf_init(&cl_conf);
    struct pi_device cluster_dev;
    pi_open_from_conf(&cluster_dev, &cl_conf);

    // Open cluster device
    if (pi_cluster_open(&cluster_dev))
        pmsis_exit(-1);
    
    #ifdef GAP9
    printf("Setting CL freq\n");
    pi_freq_set(PI_FREQ_DOMAIN_CL, 240000000);
    #elif defined(GAPUINO)
    pi_freq_set(PI_FREQ_DOMAIN_CL, 87000000);
    #endif
    
    // Execute decomposition task on the cluster
    DecompArgs decomp_args = {
        .emg = &emg,
        .mean_vec = &mean_vec,
        .white_mtx = &white_mtx,
        .sep_mtx = &sep_mtx,
        .spike_th = &spike_th
    };
    struct pi_cluster_task decomp_task;
    pi_cluster_task(&decomp_task, decomp_entry, (void *) &decomp_args);
    pi_cluster_send_task_to_cl(&cluster_dev, &decomp_task);

    // Execute classification task on the cluster
    uint8_t class = 0;
#if defined(USE_SVM)
#ifdef PRINTING
    printf("\nUsing SVM\n");
#endif
    SVMArgs clf_args = {
        .svm_coef = &svm_coef,
        .svm_intercept = &svm_intercept,
        .class = &class
    };
#else
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
#endif
    struct pi_cluster_task clf_task;
    pi_cluster_task(&clf_task, clf_entry, (void *) &clf_args);
    pi_cluster_send_task_to_cl(&cluster_dev, &clf_task);

    // Release cluster device
    pi_cluster_close(&cluster_dev);

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

    #ifdef GAP9
    printf("Setting FC freq\n");
    while   (setup_frequency_voltage(240000000, 650))   {
        printf("FC: Cannot set correct freq\n");
    }
    printf("Geia\n");       // Need this as a delay to get correct measurements (correct voltage/frequency setting)
    #endif

    #ifdef GPIO_PROFILING
    #if defined(GAP9) || defined(GAPUINO) 
    gpio_ini();
    gpio_enable();
    #endif
    #endif

    run_semg_bss();

    #ifdef GPIO_PROFILING
    #if defined(GAP9) || defined(GAPUINO)
    gpio_disable();
    #endif
    #endif
}
