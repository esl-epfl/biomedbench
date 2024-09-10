
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
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <float.h>
#include <utils_functions.h>
#include <fixmath.h>

void vect_min(my_int *src, my_int size, my_int *dst)
{
	my_int min = src[0];
	for (int32_t i = 0; i < size; i++)
	{
		if(src[i] < min){
			min = src[i];
		}
	}
	*dst = min;
}

void vect_add(my_int *srcA, my_int *srcB, my_int *dst, my_int size)
{
	for (int32_t i = 0; i < size; i++)
	{
		dst[i] = srcA[i] + srcB[i];
	}
}

void vect_sub(my_int *srcA, my_int *srcB, my_int *dst, my_int size)
{
	for (int32_t i = 0; i < size; i++)
	{
		dst[i] = srcA[i] - srcB[i];
	}
}

void vect_mult(my_int *srcA, my_int *srcB, my_int *dst, my_int size, int8_t n_dec)
{
	for (int32_t i = 0; i < size; i++)
	{
		dst[i] = fx_mulx(srcA[i], srcB[i], n_dec);
	}
}

void vect_scale(my_int *src, my_int scale, my_int *dst, my_int size, int8_t n_dec)
{
	for (int32_t i = 0; i < size; i++)
	{
		dst[i] = fx_mulx(src[i], scale, n_dec);
	}
}

void vect_power(my_int *src, my_int size, my_int *result, int8_t n_dec)
{
	my_int sum = 0;
	for (int32_t i = 0; i < size; i++)
	{
		sum += fx_mulx(src[i], src[i], n_dec);
	}
	*result = sum;
}

void vect_copy(int32_t *src, my_int *dst, my_int size)
{
	for (int32_t i = 0; i < size; i++)
	{
		dst[i] = src[i];
	}
}

/* 
	This function is basically the same as vect_copy but it also transforms data from int
	to fixed_t. This way they are ready to be used.
*/
void get_int_samples(const int32_t *src, my_int *dst, my_int size, int8_t n_dec)
{
	for (int32_t i = 0; i < size; i++)
	{
		dst[i] = fx_itox(src[i], n_dec);
	}
}

/*
	This function is used to get the input samples. It is specific for 
	float input samples.
*/
void get_float_samples(const float *src, my_int *dst, my_int size, int8_t n_dec)
{
	for (size_t i = 0; i < size; i++)
	{
		dst[i] = fx_ftox(src[i], n_dec);
	}
	
}

void vect_dot_prod(my_int *srcA, my_int *srcB, my_int size, my_int *dst, int8_t n_dec)
{
	*dst = 0;
	for (int32_t i = 0; i < size; i++)
	{
		*dst += fx_mulx(srcA[i], srcB[i], n_dec);
	}
}

void vect_mean(my_int *src, my_int size, my_int *result, int8_t n_dec)
{
	my_int sum = 0;
	for (int32_t i = 0; i < size; i++)
	{
		sum += src[i];
	}	
	size = fx_itox(size, n_dec);
	*result = fx_divx(sum, size, n_dec);
}

void vect_var(my_int *src, my_int size, my_int *result, int8_t n_dec)
{
	my_int sum = 0;
	my_int mean;
	vect_mean(src, size, &mean, n_dec);
	for (int32_t i = 0; i < size; i++)
	{
		sum += fx_mulx(((my_int)src[i] - mean), ((my_int)src[i] - mean), n_dec);
	}
	size = fx_itox(size, n_dec);
	*result = fx_divx(sum, size, n_dec);
}

void vect_std(my_int *src, my_int size, my_int *result, int8_t n_dec)
{
	/* This is the old implementation, not suitable */
	// my_int var;
	// vect_var(src, size, &var, n_dec);
	// *result = fx_sqrtx(var, n_dec);

	my_int sumOfSquares = 0;
	my_int sum = 0;

	for (size_t i = 0; i < size; i++)
	{
		sumOfSquares += fx_mulx(src[i], src[i], n_dec);
		sum += src[i];
	}

	// The result has this form:
	// result = sqrt((sumOfSquares - sum^2 / blockSize) / (blockSize - 1))

	size = fx_itox(size, n_dec);
	my_int sum_sq = fx_mulx(sum, sum, n_dec);
	my_int num = sumOfSquares - fx_divx(sum_sq , size, n_dec);
	my_int den = size - fx_itox(1, n_dec);
	
	*result = fx_sqrtx(fx_divx(num, den, n_dec), n_dec);
}


void vect_conv(my_int *srcA, my_int sizeA, my_int *srcB, my_int sizeB, my_int *dst, int8_t n_dec)
{
	//final len: sizeA + sizeB - 1

	for (int32_t n = 0; n < sizeA + sizeB - 1; n++)
	{
		my_int sum = 0;
		for (int32_t k = 0; k < sizeA; k++)
		{
			if ((n - k) >= 0 && (n - k) < sizeB)
			{
				sum += fx_mulx(srcA[k], srcB[n - k], n_dec);
			}
		}
		dst[n] = sum;
	}
}

void vect_max(my_int *src, my_int size, my_int *result, int16_t *index)
{
	my_int max = src[0];
	int16_t ind = 0;
	for (int16_t i = 0; i < size; i++)
	{
		if (src[i] > max)
		{
			max = src[i];
			ind = i;
		}
	}
	*result = max;
	*index = ind;
}

void vect_offset(const my_int *src, my_int offset, my_int *dst, my_int size){
	for (size_t i = 0; i < size; i++)
	{
		dst[i] = src[i] + offset;
	}
	
}

/* 
	Computed the squared magnitude of the input complex data.
	The input vector is assumed to be made of complex numbers stored in
	an interleaved way (Real1, Imag1, Real2, Imag2, Real3 ....)
 */
void cmplx_mag_squared(const my_int *src, my_int *dst, uint32_t numSamples, int8_t n_dec)
{
	for (size_t i = 0; i < numSamples; i++)
	{
		dst[i] = fx_mulx(src[2*i], src[2*i], n_dec) + fx_mulx(src[(2*i)+1], src[(2*i)+1], n_dec);
	}
}

/*	
	Converts the src made of float numbers into fixed point numbers.
	Stores the result into dest
*/
void convert_to_x(float *src, my_int* dest, my_int size, int8_t n_dec)
{
	for(uint8_t i = 0; i < size; i++)
	{
		dest[i] = fx_ftox(src[i], n_dec);
	}
}

/*
	Changes the fixpoint representation of src numbers. The bits used for the decimal part
	moves from old_depth to new_depth.
	The new numbers will be stored in dst.
	The size of the input array has to be speciied by size. 
*/
void change_bit_depth(my_int *src, my_int *dst, int32_t size, int8_t old_depth, int8_t new_depth)
{
	int8_t diff = new_depth - old_depth;
	for (size_t i = 0; i < size; i++)
	{
		dst[i] = (diff < 0) ? src[i] >> -diff : src[i] << diff;
	}
	
}
