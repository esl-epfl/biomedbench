
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

#ifndef SHARED_BUF_H
#define SHARED_BUF_H

#include <stdint.h>

#include "defines.h"

#define N_CH_EXT N_CH * FE
#define N_SAMPLES WIN_LEN * FS_ / 1000
#define EXT_WIN Q - FE + 1

/*
 * Tmp1 buffer for:
 * mean vector (N_CH_EXT)
 * DNN bias (N_TA/N_CA/N_OUT)
 * SVM coefficients (N_OUT * (N_OUT - 1) / 2 x N_MU)
 */
 extern float tmp1_data[];  // size: N_CH_EXT

/*
 * Tmp2 buffer for:
 * whitening matrix (N_CH_EXT x N_CH_EXT)
 * DNN weight (N_TA x N_SAMPLES/N_CA x N_TA * N_MU/N_OUT x N_CA)
 * SVM intercept (N_OUT * (N_OUT - 1) x 1)
 */
 extern float tmp2_data[];  // size: N_CH_EXT * N_CH_EXT

/*
 * Tmp3 buffer for:
 * separation matrix (N_MU x N_CH_EXT)
 * 1st DNN activation (N_MU x N_TA)
 * 3rd DNN activation (1 x N_OUT)
 * spike count (N_MU x 1)
 */
 extern float tmp3_data[];  // size: N_MU * N_CH_EXT

/*
 * Tmp4 buffer for:
 * spike thresholds (N_MU x 1)
 * 2nd DNN activation (1 x N_CA)
 * SVM projection (N_OUT * (N_OUT - 1) x 1)
 */
 extern float tmp4_data[];  // size: N_MU

/*
 * Tmp5 buffer for:
 * original sEMG slice (Q x N_CH)
 * whitened sEMG slice (N_CH_EXT x EXT_WIN)
 */
 extern float tmp5_data[];  // size: N_CH_EXT * EXT_WIN

/*
 * Tmp6 buffer for:
 * extended sEMG slice (N_CH_EXT * EXT_WIN)
 * centered sEMG slice (N_CH_EXT * EXT_WIN)
 * MUAPT slice (N_MU)
 */
 extern float tmp6_data[];  // size: N_CH_EXT * EXT_WIN

/*
 * Tmp7 buffer for:
 * - spike binary matrix (N_MU x N_SAMPLES)
 */
 extern uint8_t tmp7_data[];

/* Tmp1 buffer for:
 * - original slice (FE x N_CH = N_CH_EXT)
 * - whitened slice (N_CH_EXT)
 * - slice of spikes (N_MU)
 * - DNN bias (N_TA/N_CA/N_OUT)
 * - spike count (N_MU)

 extern float tmp1_data[];  // N_CH_EXT dimensions

*
 * Tmp2 buffer for:
 * - extended slice (N_CH_EXT)
 * - mean vector (N_CH_EXT)


 extern float tmp2_data[];  // mean vector, columns of whitening matrix, separation matrix, DNN weights, SVM coefficients
 extern float tmp3_data[];  // extended and centered slice, slice of MUAPT, first and second DNN activations, SVM intercepts
 extern float tmp4_data[];  // flattened first DNN activation, third DNN activation
 extern uint8_t tmp5_data[];  // spikes, SVM 1v1 report*/

#endif
