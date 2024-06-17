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


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <mfcc_module.h>

// To convert the STFT output into MEL domain
#include <mel_basis.h>

// Hanning window to be used in the STFT method before the RFFT
#include <mfcc_hann_wind.h>

#include <helpers.h>

#include <kiss_fftr.h>

#include "main.h"

// This function is internally used to avoid the helper module
// the need of importing the kiss_fft library 
void _cmplx_mag(kiss_fft_cpx *x, int16_t len, float *res);

// internal use function to compute the dct on a linear array
void _dct_linear(float *x, int16_t len, float *y);

/*
    Computes the complex magnitude of the input RFFT result and put
    it inside res array
*/
void _cmplx_mag(kiss_fft_cpx *x, int16_t len, float *res){
    for(int16_t i=0; i<len; i++){
        res[i] = sqrtf((x[i].r * x[i].r) + (x[i].i * x[i].i));
    }
}

/*
    Computes the power of the signal using the STFT method.

    The final result will be a matrix stored in the 1D array res.
    Note that the matrix is stored columns by column, one after the other:
    res = [.... COLUMN 0 ....|.... COLUMN 1 ....|....]
*/
void stft(float *x, int16_t len, int16_t n_frames, float *res){

    // apply padding
    int16_t padded_len = (2 * PAD_LEN) + len;
    float *padded = (float*)pi_cl_l1_malloc(&cluster_dev, padded_len * sizeof(float));
    zero_padding(x, len, PAD_LEN, padded);

    float *column = (float*)pi_cl_l1_malloc(&cluster_dev, N_FFT * sizeof(float));
    float *fft_res = (float*)pi_cl_l1_malloc(&cluster_dev, FFT_RES_LEN * sizeof(float));

    // initialize RFFT structures
    kiss_fftr_cfg cfg = kiss_fftr_alloc(N_FFT, 0, 0, 0);
    kiss_fft_cpx *cx_out = (kiss_fft_cpx*)pi_cl_l1_malloc(&cluster_dev, N_FFT *sizeof(kiss_fft_cpx));

    for(int16_t i=0; i<n_frames; i++){

        // get the i-th column of the padded input (i.e. i-th frame)
        // Basically the framing is considered to produce a matrix, each
        // column is a frame to process. It's done like this in order to
        // be compliant with the python code
        for(int16_t j=0; j<N_FFT; j++){
            column[j] = padded[j + (i*HOP_LEN)];
        }

        // apply the window
        vect_mult(column, hann_mfcc_wind, N_FFT, column);

        kiss_fftr(cfg, column, cx_out);
        _cmplx_mag(cx_out, FFT_RES_LEN, column);
        vect_mult(column, column, FFT_RES_LEN, column); // element-wise power of 2
        
        // stores the current column result into the result array
        // Note that the each column is stored sequentially
        //
        // res = [.... COLUMN 0 ....|.... COLUMN 1 ....|....]
        for(int16_t j=0; j<FFT_RES_LEN; j++){
            res[(i * FFT_RES_LEN) + j] = column[j];
        }
    }

    pi_cl_l1_free(&cluster_dev, cfg, N_FFT * sizeof(kiss_fft_cpx));
    pi_cl_l1_free(&cluster_dev, cx_out,  N_FFT * sizeof(kiss_fft_cpx));
    pi_cl_l1_free(&cluster_dev, padded, padded_len * sizeof(float));
    pi_cl_l1_free(&cluster_dev, column, N_FFT * sizeof(float));
    pi_cl_l1_free(&cluster_dev, fft_res, FFT_RES_LEN * sizeof(float));
}

/*
    Computes the melodic spectrogram by using the STFT method
    and by multiplying it a mel_basis matrix
*/
void mel_spectrogram(float *x, int16_t len, int16_t n_frames, float *res){

    // STFT
    float *frames_power = (float*)pi_cl_l1_malloc(&cluster_dev, (FFT_RES_LEN * n_frames) * sizeof(float));
    stft(x, len, n_frames, frames_power);

    // MULT BY MEL BASIS (matrix multiplication)
    // frames_power is saved one column after the other
    // the result matrix is saved one row after the other
    for(int16_t i=0; i<MEL_ROWS; i++){              // rows in the mel basis
        for(int16_t j=0; j<n_frames; j++){          // columns for frames_powers and raws for mel_basis
            for(int16_t k=0; k<MEL_COLUMNS; k++){   // columns in the mel basis

                // check is the index is out of the non-zero elements range
                if(k < mel_nz_indexes[i][0] || k > mel_nz_indexes[i][1]){
                    res[(i*n_frames) + j] += 0.0;   // 0.0 beacuse would be multiplied by 0.0
                } else {  // mel_basis element is non-zero 
                    res[(i*n_frames) + j] += mel_basis[i][k - mel_nz_indexes[i][0]] * frames_power[(j*MEL_COLUMNS) + k];
                }

            }
        }
    }

    pi_cl_l1_free(&cluster_dev, frames_power, (FFT_RES_LEN * n_frames) * sizeof(float));
}


/*
    Converts the input array of powers to dB and stores the
    result into res array
*/
void power_to_dB(float *x, int16_t len, float *res){
    float sample = 0.0;
    for(int16_t i=0; i<len; i++){
        if(x[i] == 0){
            sample = F_MIN;
        } else {
            sample = x[i];
        }
        res[i] = 10.0 * log10f(sample);
    }
    float max = vect_max_value(res, len);

    for(int16_t i=0; i<len; i++){
        if((max - TOP_DB) > res[i]){
            res[i] = (max - TOP_DB);
        }
    }
}

/*
    Computes the DCT on a single dimentional array
*/
void _dct_linear(float *x, int16_t len, float *y){

    float sum = 0.0;
    float scaling = sqrtf(1.0 / (2 * len));

    float cos_argument = 0.0;
    float cos_v = 0.0;

    for(int16_t k=0; k<len; k++){
        sum = 0.0;
        for(int16_t n=0; n<len; n++){
            cos_argument = (PI * k * ((2 * n) + 1)) / (2 * len);
            // cos_v = cos(cos_argument);
            cos_v = cosine(cos_argument);
            sum += x[n] * cos_v;
        }
        y[k] = sum * 2;
        
        if(k>0){
            y[k] = y[k] * scaling;
        }
    }
    y[0] = y[0] * sqrtf(1.0 / (4 * len));
}

/*
    Computes the DCT (Discrete Cosine Transform) on the input matrix and 
    stores the result in the output matrix.
    
    Both input and output matrices are stored as 1D arrays.
    
    The matrix is stored on a row by row basis:
    x = [... ROW 0 ... | ... ROW 1 ... | ....]
*/
void dct_matrix(float *x, int16_t rows, int16_t cols, float *y){

    float *column = (float*)pi_cl_l1_malloc(&cluster_dev, rows * sizeof(float));
    float *c_res = (float*)pi_cl_l1_malloc(&cluster_dev, rows * sizeof(float));

    for(int16_t c=0; c<cols; c++){
        // fill the column array with the current column
        for(int16_t r=0; r<rows; r++){
            column[r] = x[(r*cols) + c];
        }

        _dct_linear(column, rows, c_res);

        // copy the result on the output
        for(int16_t r=0; r<rows; r++){
            y[(r*cols) + c] = c_res[r];
        }
    }

    pi_cl_l1_free(&cluster_dev, column, rows * sizeof(float));
    pi_cl_l1_free(&cluster_dev, c_res, rows * sizeof(float));
}