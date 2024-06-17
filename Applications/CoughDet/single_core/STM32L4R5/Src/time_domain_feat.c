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
#include <math.h>

#include <feature_extraction.h>
#include <time_domain_feat.h>
#include <helpers.h>
#include <filters_parameters.h>
#include <filtering.h>

#include <audio_features.h>


// Here I put all the functions to compute time domain features

int16_t _find_peaks(const float *x, int16_t len);

// Returns the max value of an array 
float get_max(const float *sig, int16_t len){
    
    float max = sig[0];

    for(int16_t i=1; i<len; i++){
        if(sig[i] > max){
            max = sig[i];
        }
    }
    return max;
}


// Subtracts to each sample of a signal its mean
void sub_mean(const float *sig, float *res, int16_t len){

    float mean = vect_mean(sig, len);

    sub_constant(sig, len, mean, res);
}

// Returns the RMS of the input signal
float get_rms(const float *sig, int16_t len){
    float sum = 0;
    for(int16_t i=0; i<len; i++){
        sum += sig[i] * sig[i];
    }
    return sqrtf(sum / len);
}

// Computes the Zero Crossing Rate
float compute_zrc(const float *sig, int16_t len){

    int sum = 0;
    float interm_product = 0;

    for(int16_t i=0; i<len-1; i++){
        interm_product = sig[i] * sig[i + 1];
        if(interm_product < 0){
            sum++;
        }
    }
    return (float) sum / (len - 1);
}


/*
    Helper function to count the number of local maxima in a signal.
    This function mimics the _local_maxima_1d() function in the python
    "scipy" module.
*/
int16_t _find_peaks(const float *x, int16_t len){

    int16_t i = 1; // points to the first considered sample (the second one)
    int16_t i_max = len-1;
    int16_t i_ahead = 0;

    int16_t npeaks = 0;

    while(i < i_max){

        if(x[i-1] < x[i]){
            i_ahead = i + 1;

            while(i_ahead < i_max && x[i_ahead] == x[i]){
                i_ahead++;
            }

            if(x[i_ahead] < x[i]){
                npeaks++;
                i = i_ahead;
            }
        }
        i++;
    }
    return npeaks;
}

// Computes the EEPD features
void eepd(const float *sig, int16_t len, int16_t fs, int16_t *res){

    float *interm = (float*)malloc(len * sizeof(float));    // to store the intermediate result between the first and the second filter 
    float *filtered = (float*)malloc(len * sizeof(float));       // temporary to store the result of each filter

    const float *b, *a, *zi;

    for(int16_t i=0; i<N_EEPD; i++){

        b = filters_parameters.filters[i].b;
        a = filters_parameters.filters[i].a;
        zi = filters_parameters.filters[i].zi;  

        filtfilt(sig, len, b, a, zi, interm);
        vect_mult(interm, interm, len, interm);     // squared vector
        filtfilt(interm, len, b_second, a_second, zi_second, filtered);

        normalize_max(filtered, len, filtered);   // divide each number by the maximum

        res[i] = _find_peaks(filtered, len);
    }

    free(interm);
    free(filtered);
}
