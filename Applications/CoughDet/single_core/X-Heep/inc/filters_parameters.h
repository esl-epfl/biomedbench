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



#ifndef _FILTERS_PARAMETERS_H
#define _FILTERS_PARAMETERS_H

#define PADLEN 9    // Length of the padding applied for the filtering


// Holds the coefficients of the filter and its initial states
typedef struct filter_param {
    float b[3];
    float a[3];
    float zi[2];
} filter_param_t;


// Holds all the filters used. Note that these are just the bandpass filters that
// depend on the band.
// The low pass filter is stores separately ince it's the same
typedef struct all_filters {
    filter_param_t filters[19];
} all_filters_t;


/* Parameters for the various filters.                      */
/* b and a are the coefficients of the transfer function    */
/* zi are the initial states of the filter                  */
extern const all_filters_t filters_parameters;

// Parameters of the second filter used in the EEPD function. These are the same for every band
extern const float b_second[3];
extern const float a_second[3];

extern const float zi_second[2];

#endif
