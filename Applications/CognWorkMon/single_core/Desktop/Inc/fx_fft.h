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

#include <stdint.h>
#include <main.h>

void init();
void fft(my_int *input, my_int *outputR, my_int *outputI);

// Bits reversal algo for input/output reordering after the radix FFT implementation
uint32_t ReverseBits ( uint32_t index, uint32_t NumBits );
// Determine the number of bits needed to do the bits reversal
uint32_t NumberOfBitsNeeded ( uint32_t nsample );

// Iterative radix2 implementation with input in bit reversed order
void fft_cplx_radix2_iter  (my_int *Real_Out, my_int *Imag_Out, int32_t fft_size, int32_t nbits, my_int *re_factors, my_int *im_factors);