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



#ifndef _AUDIO_FEATURES_H_
#define _AUDIO_FEATURES_H_

#include <inttypes.h>

/* Defines of the hyperparameters of audio features extraction */

// Number of MFCCs to extract
#define N_MFCC 13

// number of PSD bands
#define N_PSD 7

// Start and end frequencies for the PSD bands
// #define PSD_BAND_1_START 100
// #define PSD_BAND_1_END 300

// #define PSD_BAND_2_START 400
// #define PSD_BAND_2_END 500

// #define PSD_BAND_3_START 600
// #define PSD_BAND_3_END 700

#define PSD_BAND_1_START 0
#define PSD_BAND_1_END 50

#define PSD_BAND_2_START 50
#define PSD_BAND_2_END 175

#define PSD_BAND_3_START 175
#define PSD_BAND_3_END 400

#define PSD_BAND_4_START 500
#define PSD_BAND_4_END 800

#define PSD_BAND_5_START 800
#define PSD_BAND_5_END 1100

#define PSD_BAND_6_START 1400
#define PSD_BAND_6_END 2100

#define PSD_BAND_7_START 2500
#define PSD_BAND_7_END 2900

// struct to store the limits of each PSD band
typedef struct band {
    int16_t start;
    int16_t end;
} band_t;

// array to access all the bands in a compact way
extern const band_t psd_bands[N_PSD];


// Start, end and bandwidth frequencies for the EEPD bandpass filter
#define EEPD_START 50
#define EEPD_END 1000
#define WIDTH 50 // TODO: chiedi se questo parametro è lo stesso di EEPD_BANDWIDTH
#define EEPD_BANDWIDTH 50
#define N_EEPD (EEPD_END - EEPD_START) / WIDTH


/* All the families of features used. The values of the enum are adjusted 
   considering the amount of features for every family */
enum audio_features_families{
    SPECTRAL_DECREASE,
    SPECTRAL_SLOPE,
    SPECTRAL_ROLLOFF,
    SPECTRAL_CENTROID,
    SPECTRAL_SPREAD,
    SPECTRAL_KURTOSIS,
    SPECTRAL_SKEW,
    SPECTRAL_FLATNESS,
    SPECTRAL_STD,
    DOMINANT_FREQUENCY,
    POWER_SPECTRAL_DENSITY,
    MEL_FREQUENCY_CEPSTRAL_COEFFICIENT = POWER_SPECTRAL_DENSITY + N_PSD,
    ZERO_CROSSING_RATE = MEL_FREQUENCY_CEPSTRAL_COEFFICIENT + (N_MFCC * 2),
    ROOT_MEANS_SQUARED,
    CREST_FACTOR,
    ENERGY_ENVELOPE_PEAK_DETECT,
    Number_AUDIO_Features = ENERGY_ENVELOPE_PEAK_DETECT + N_EEPD// Hardcoded just to get the final length
};

#endif
