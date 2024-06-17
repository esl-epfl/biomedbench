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



#ifndef _FREQ_FEAT_H_  
#define _FREQ_FEAT_H_

#include <inttypes.h>
#include <welch_psd.h>

/*
    Set of functions to compute the RFFT (Real FFT) and spectral features of a signal
*/
void compute_rfft(const float *sig, int16_t len, int16_t fs, float *mags, float *freqs, float *sum_mags);
void compute_periodogram(const float *sig, int16_t len, int16_t fs, float *psd, float *freqs);
float compute_spec_decrease(float* mags, float* freqs, int16_t len, float sum_mags);
float compute_spectral_slope(float *mags, float *freqs, int16_t len, float sum_mags);
float compute_rolloff(float *mags, float *freqs, int16_t len, float sum_mags);
float compute_centroid(float *mags, float *freqs, int16_t len, float sum_mags);
float compute_spread(float *mags, float *freqs, int16_t len, float sum_mags, float centroid);
float compute_kurt(float *mags, float *freqs, int16_t len, float sum_mags, float centroid, float spread);
float compute_skew(float *mags, float *freqs, int16_t len, float sum_mags, float centroid, float spread);
float compute_flatness(const float *x, int16_t len);
float compute_std(const float *x, int16_t len);
float get_domiant_freq(float *psd, float *freqs, int16_t len);
void normalized_bandpowers(float *psd, float *freqs, int16_t len, const int8_t *psd_selector, float *band_powers);
void mfcc_computation(const float *x, int16_t len, int16_t n_frames, float *coeffs);
void get_mfcc_features(const float *x, int16_t len, float *mean_mfcc, float *std_mfcc);

#endif