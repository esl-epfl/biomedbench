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



#include <stdlib.h>
#include <stdio.h>

#include <helpers.h>
#include <filters_parameters.h>


/*
    Applies a linear filter to the signal. The filter is defined
*/
void linear_filer(const float *sig, int len, const float *b, const float *a, const float *zi, float *res){

    float sig_1 = 0.0;
    float y_1 = 0.0;

    float s_1 = zi[0];
    float s_2 = zi[1];

    // First one needs to be done outside the loop in order not to 
    // update the states (s_1 and s_2)
    res[0] = b[0] * sig[0] + s_1;
    y_1 = res[0];
    sig_1 = sig[0];

    for(int i=1; i<len; i++){

        // s_1 is updated before and s_2 after. This is because at every
        // iteration we need the filter state 1 (s_1) of the current step and 
        // the filter state 2 (s_2) of the previous step.
        // Also by doing so we avoid using 2 extra variables for the signal
        // and output values of 2 steps before

        s_1 = b[1] * sig_1 - a[1] * y_1 + s_2;
        res[i] = b[0] * sig[i] + s_1;
        s_2 = b[2] * sig_1 - a[2] * y_1;

        sig_1 = sig[i];
        y_1 = res[i];
    }
}


/*
    Applies a filter to the signal.
    The filter has transfer function with coefficients b and a, passed as parameters
    The way this filter is implemented is compliant with the filtfilt() function
    of the python package "scipy".

    b, a and zi are respectively the b, a coefficients of the filter and
    its initial state.

    The main steps are:
    - padding of the signal
    - forward filtering
    - backward filtering
*/
void filtfilt(const float *sig, int len, const float *b, const float* a, const float *zi, float *res){

    // PADDING //
    int padded_len = (2 * PADLEN) + len;
    float *pad = (float*)malloc(padded_len * sizeof(float));
    padding(sig, len, PADLEN, pad);
    
    // GET INITIAL STATE //
    float x0 = pad[0];

    // FILTER FORWARD //
    float *intermediate = (float*)malloc(padded_len * sizeof(float)); // intermediate output, cannot be res because it's not padded

    float *initial = (float*)malloc(2 * sizeof(float));
    initial[0] = zi[0] * x0;
    initial[1] = zi[1] * x0;
    linear_filer(pad, padded_len, b, a, initial, intermediate);

    // FILTER BACKWARD //
    float *reverse = (float*)malloc(padded_len * sizeof(float));    // for the reverse filtering
    for(int i=0; i<padded_len; i++){
        reverse[i] = intermediate[padded_len-1-i];
    }
    
    initial[0] = zi[0] * reverse[0];
    initial[1] = zi[1] * reverse[0];
    
    float *res_padded = (float*)malloc(padded_len * sizeof(float));
    linear_filer(reverse, padded_len, b, a, initial, res_padded);

    // CUT THE PADDING TO GET THE FINAL RESULT //  
    for(int i=0; i<len; i++){
        res[i] = res_padded[PADLEN+len-1-i];    // cut the padding and reverse
    }

    free(pad);
    free(intermediate);
    free(res_padded);
    free(initial);
    free(reverse);
}