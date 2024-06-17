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
#include <stdint.h>
#include <math.h>
#include <fixmath.h>
#include <fx_fft.h>
#include <window_definitions.h>

#define FFT_SIZE WINDOW_LENGTH
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// STARTO OF DUMMY
// Floating point FT input/ouput
// float RealIn[FFT_SIZE];
// float ImagIn[FFT_SIZE];
// float RealOut[FFT_SIZE];
// float ImagOut[FFT_SIZE];
// END OF DUMMY

// radix 2
my_int real_w_fxp_r2[FFT_SIZE / 2];
my_int imag_w_fxp_r2[FFT_SIZE / 2];

// split ops factors arrays
my_int A_r[FFT_SIZE / 2];
my_int A_i[FFT_SIZE / 2];
my_int B_r[FFT_SIZE / 2];
my_int B_i[FFT_SIZE / 2];

// tmp array for real fft
my_int fft_re_tmp[FFT_SIZE];
my_int fft_im_tmp[FFT_SIZE];

uint32_t NumBits;

void init()
{
  // Real FFT of size FFT_SIZE is transform to a complex FFT of size FFT_SIZE/2
  NumBits = NumberOfBitsNeeded(FFT_SIZE / 2);

  // Create twiddle factors radix-2
  my_int num = fx_mulx(fx_itox(2, N_DEC_POW), fx_ftox(M_PI, N_DEC_POW), N_DEC_POW);     // 2. * M_PI
  my_int den = fx_divx(fx_itox(FFT_SIZE, N_DEC_POW), fx_itox(2, N_DEC_POW), N_DEC_POW); // FFT_SIZE / 2.
  for (int i = 0; i < FFT_SIZE / 4; i++)
  {
    // real_w_fxp_r2[i] = fx_dtox(cos(2. * M_PI / (double)(FFT_SIZE / 2.0) * i), N_DEC_POW);
    // imag_w_fxp_r2[i] = fx_dtox(-sin(2. * M_PI / (double)(FFT_SIZE / 2.0) * i), N_DEC_POW);
    my_int arg = fx_mulx(fx_divx(num, den, N_DEC_POW), fx_itox(i, N_DEC_POW), N_DEC_POW);
    real_w_fxp_r2[i] = fx_cosx(arg, N_DEC_POW);
    imag_w_fxp_r2[i] = -fx_sinx(arg, N_DEC_POW);
  }

  // The recovery additional step to recover the correct output is called split ops
  // The factors are precomputed here
  den = fx_divx(fx_mulx(fx_itox(2, N_DEC_POW), fx_itox(FFT_SIZE, N_DEC_POW), N_DEC_POW), fx_itox(2, N_DEC_POW), N_DEC_POW);
  for (int i = 0, j = FFT_SIZE / 2 - 1; i < FFT_SIZE / 2; i++, j--)
  {
    my_int arg = fx_mulx(fx_divx(num, den, N_DEC_POW), fx_itox(i, N_DEC_POW), N_DEC_POW);
    my_int sinx = fx_sinx(arg, N_DEC_POW);
    my_int cosx = fx_cosx(arg, N_DEC_POW);

    // A_r[i] = fx_dtox((double)(0.5 * (1.0 - sin(2.0 * M_PI / (double)(2.0 * FFT_SIZE / 2.0) * i))), N_DEC_POW);
    // A_i[i] = fx_dtox((double)(0.5 * (-1.0 * cos(2.0 * M_PI / (double)(2.0 * FFT_SIZE / 2.0) * i))), N_DEC_POW);
    A_r[i] = fx_mulx(fx_ftox(0.5, N_DEC_POW), (fx_itox(1, N_DEC_POW) - sinx), N_DEC_POW);
    A_i[i] = fx_mulx(fx_ftox(0.5, N_DEC_POW), fx_mulx(fx_itox(-1, N_DEC_POW), cosx, N_DEC_POW), N_DEC_POW);

    // B_r[i] = fx_dtox((double)(0.5 * (1.0 + sin(2.0 * M_PI / (double)(2.0 * FFT_SIZE / 2.0) * i))), N_DEC_POW);
    // B_i[i] = fx_dtox((double)(0.5 * (1.0 * cos(2.0 * M_PI / (double)(2.0 * FFT_SIZE / 2.0) * i))), N_DEC_POW);
    B_r[i] = fx_mulx(fx_ftox(0.5, N_DEC_POW), (fx_itox(1, N_DEC_POW) + sinx), N_DEC_POW);
    B_i[i] = fx_mulx(fx_ftox(0.5, N_DEC_POW), fx_mulx(fx_itox(1, N_DEC_POW), cosx, N_DEC_POW), N_DEC_POW);
  }
}

void fft(my_int *input, my_int *outputR, my_int *outputI)
{
  // // START OF DUMMY, just float implmentation to double check
  // float RealIn[FFT_SIZE];
  // float ImagIn[FFT_SIZE];
  // float RealOut[FFT_SIZE];
  // float ImagOut[FFT_SIZE];
  // for (int i = 0; i < FFT_SIZE; i++)
  // {
  //   RealIn[i] = fx_xtof(input[i], N_DEC_POW);

  //   ImagIn[i] = 0.0;
  // }

  // // Fourier transform definition implementation
  // for (int j = 0; j < FFT_SIZE; j++)
  // {
  //   for (int i = 0; i < FFT_SIZE; i++)
  //   {
  //     int32_t index_comb = (i * j) % FFT_SIZE;

  //     RealOut[j] += RealIn[i] * cos(2 * M_PI / (float)(FFT_SIZE)*index_comb) + ImagIn[i] * sin(2 * M_PI / (float)(FFT_SIZE)*index_comb);
  //     ImagOut[j] += -RealIn[i] * sin(2 * M_PI / (float)(FFT_SIZE)*index_comb) + ImagIn[i] * cos(2 * M_PI / (float)(FFT_SIZE)*index_comb);
  //   }
  // }
  // // END OF DUMMY

  int i, j;
  // Real input is transformed to complex input
  for (i = 0; i < FFT_SIZE / 2; i++)
  {
    j = ReverseBits(i, NumBits);
    fft_re_tmp[j] = input[2 * i];
    fft_im_tmp[j] = input[2 * i + 1];
  }

  fft_cplx_radix2_iter(fft_re_tmp, fft_im_tmp, FFT_SIZE / 2, NumBits, real_w_fxp_r2, imag_w_fxp_r2);

  fft_re_tmp[FFT_SIZE / 2] = fft_re_tmp[0];
  fft_im_tmp[FFT_SIZE / 2] = fft_im_tmp[0];

  // Split ops
  for (i = 0; i < FFT_SIZE / 2; i++)
  {
    outputR[i] = fx_mulx(fft_re_tmp[i], A_r[i], N_DEC_POW) - fx_mulx(fft_im_tmp[i], A_i[i], N_DEC_POW) + fx_mulx(fft_re_tmp[FFT_SIZE / 2 - i], B_r[i], N_DEC_POW) + fx_mulx(fft_im_tmp[FFT_SIZE / 2 - i], B_i[i], N_DEC_POW);
    outputI[i] = fx_mulx(fft_im_tmp[i], A_r[i], N_DEC_POW) + fx_mulx(fft_re_tmp[i], A_i[i], N_DEC_POW) + fx_mulx(fft_re_tmp[FFT_SIZE / 2 - i], B_i[i], N_DEC_POW) - fx_mulx(fft_im_tmp[FFT_SIZE / 2 - i], B_r[i], N_DEC_POW);
  }

  // Copying the complex conjugate
  outputR[FFT_SIZE / 2] = outputR[0] - outputI[0];
  outputI[FFT_SIZE / 2] = 0;

  for (i = 1; i < FFT_SIZE / 2; i++)
  {
    outputR[FFT_SIZE - i] = outputR[i];
    outputI[FFT_SIZE - i] = -outputI[i];
  }

}

uint32_t ReverseBits(uint32_t index, uint32_t NumBits)
{
  uint32_t i, rev;

  for (i = rev = 0; i < NumBits; i++)
  {
    rev = (rev << 1) | (index & 1);
    index >>= 1;
  }

  return rev;
}

uint32_t NumberOfBitsNeeded(uint32_t nsample)
{
  uint32_t i;

  if (nsample < 2)
  {
    exit(1);
    return 0;
  }

  for (i = 0;; i++)
  {
    if (nsample & (1 << i))
      return i;
  }
}

void fft_cplx_radix2_iter(my_int *Real_Out, my_int *Imag_Out, int32_t fft_size, int32_t nbits, my_int *re_factors, my_int *im_factors)
{
  int32_t n_group = fft_size / 2;
  int32_t group_size = 1;
  int32_t base_idx;

  for (int32_t i = 1; i <= nbits; i++)
  {
    for (int32_t k = 0; k < n_group; k++)
    {
      base_idx = k << i;

      for (int32_t j = 0; j < group_size; j++)
      {
        // Even inputs
        my_int even_real = Real_Out[base_idx + j];
        my_int even_imag = Imag_Out[base_idx + j];
        // Odd inputs
        my_int odd_real = Real_Out[base_idx + j + group_size];
        my_int odd_imag = Imag_Out[base_idx + j + group_size];

        // use look-up table
        my_int w_r = re_factors[j * n_group];
        my_int w_i = im_factors[j * n_group];

        // Butterfly
        my_int p_sum_r = fx_mulx(w_r, odd_real, N_DEC_POW) - fx_mulx(w_i, odd_imag, N_DEC_POW);
        my_int p_sum_i = fx_mulx(w_i, odd_real, N_DEC_POW) + fx_mulx(w_r, odd_imag, N_DEC_POW);

        Real_Out[base_idx + j] = even_real + p_sum_r;
        Real_Out[base_idx + j + group_size] = even_real - p_sum_r;

        Imag_Out[base_idx + j] = even_imag + p_sum_i;
        Imag_Out[base_idx + j + group_size] = even_imag - p_sum_i;
      }
    }
    n_group >>= 1;
    group_size <<= 1;
  }
}
