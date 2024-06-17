/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////////
// Author:          Stefano Albini                  //
// Contributions:   Dimitrios Samakovlis            //
// Date:            September 2023                  //
//////////////////////////////////////////////////////


#ifndef _MFCC_MODULE_H_
#define _MFCC_MODULE_H_

#include <inttypes.h>

/*
    This module contains the main helper functions to compute the MFCCs
*/


// Defines and macros
#define N_FFT   2048    // NOTE: if this is changed then the hann window has to be recomputed
#define FFT_RES_LEN ((N_FFT / 2) + 1)
#define HOP_LEN 512     // fro the STFT framing of the signal


#define PAD_LEN 1024

// defines used in the dB conversion
#define F_MIN 1.17549e-038 
#define TOP_DB 80.0

// define used for the DCT computation
#define PI 3.14159265358979323846

void stft(float *x, int16_t len, int16_t n_frames, float *res);

void mel_spectrogram(float *x, int16_t len, int16_t n_frames, float *res);

void power_to_dB(float *x, int16_t len, float *res);

void dct_matrix(float *x, int16_t rows, int16_t cols, float *y);

// Cluster functions
void mult_by_mel_basis_multicore(int32_t **arg, int id);
void _cmplx_mag_multicore(int32_t **arg, int id);


#endif
