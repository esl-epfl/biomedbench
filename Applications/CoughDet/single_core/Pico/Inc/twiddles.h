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


#ifndef _TWIDDLES_H_
#define _TWIDDLES_H_


/**
 * This file contains precomputed twiddle factors for the FFT
 * computation.
 * Twiddle factors are computed with cos() and sin() on a specific
 * phase angle.
 * Each twiddle factor is stored inside a struct containing cos and sin
 * computation of a given phase.
 * 
 * The factors are subdivided in different arrays, each one is very specific for
 * the amount of FFT samples considered:
 * twiddles_225 --> twiddle factors computed for 225-sized fftr.
 * 
 * There are arrays for the following sizes:
 * - 225
 * - 450
 * - 512
 * - 1024
 * - 1200
 * - 2400
 * 
 * Pay attention to the fact that these sizes are very application specific.
 * Here they were considered for the specifi application in which this file
 * is used.
*/

typedef struct twiddles_entry
{
	float cosine;
	float sine;
} twiddles_t;

extern const twiddles_t twiddles_225[225];

extern const twiddles_t twiddles_450[450];

extern const twiddles_t twiddles_512[512];

extern const twiddles_t twiddles_1024[1024];

extern const twiddles_t twiddles_1200[1200];

extern const twiddles_t twiddles_2400[2400];

#endif
