
/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////
// Author:  Dimitrios Samakovlis        //
// Date:    September 2023              //
//////////////////////////////////////////



/* Functions for integer implementation */
#include "defines.h"
#include "defines_globals.h"
#include "library_int.h"

uint16_t isqrt32(int32_t* in_ptr) 
{
	uint16_t mask = 1<< 14;
	uint16_t result = 0;
	uint16_t temp = 0;

	if(*in_ptr < 0) return 0;
	while(mask)
	{
		temp = result | mask;
		if((((uint32_t)temp)*((uint32_t)temp)) <= ((uint32_t)(*in_ptr)))
			result = temp;
		mask >>= 1;
	}
	return result;
}

int16_t myDiv(int16_t a, int16_t b)
{
	int16_t dividend = a;
	int16_t divisor = b;
	int16_t quotient = 0;
	int16_t sign = 1;
	int16_t temp = 1<<14;
	int16_t dest = 0;

	//Avoid division by 0
	if(divisor != 0)
	{
		//Calculate sign
		if(dividend < 0 && divisor > 0) sign = -1;
		if(dividend > 0 && divisor < 0) sign = -1;

		//Absolute values of divident and divisor
		if(dividend < 0) dividend = -dividend;
		if(divisor < 0) divisor = -divisor;

		//Iterative division (15 iterations)
		while(temp !=0)
		{
			if(((quotient | temp) * divisor) <= dividend)
				quotient = quotient | temp;
			temp = temp >> 1;
		}

		//sign adjustment
		if(sign == -1)
			dest = (int32_t)-quotient;
		else
			dest = (int32_t)quotient;
	}
	return dest;
}

/*
	This function as input parameters 32 bit numbers but
it returns a 16 bit value inside a 32 bit value. It saturates
to 0x7FFF (or negative).
*/
void myDiv32_sat16(int32_t* a, int32_t* b, int32_t* dest)
{
	int32_t dividend = *a;
	int32_t divisor = *b;
	int16_t quotient = 0;
	int16_t sign = 1;
	int16_t temp = 1<<14;

	//Avoid division by 0
	if(divisor != 0)
	{
		//Calculate sign
		if(dividend < 0 && divisor > 0) sign = -1;
		if(dividend > 0 && divisor < 0) sign = -1;

		//Absolute values of divident and divisor
		if(dividend < 0) dividend = -dividend;
		if(divisor < 0) divisor = -divisor;

		//Iterative division (15 iterations)
		while(temp !=0)
		{
			if(((quotient | temp) * divisor) <= dividend)
				quotient = quotient | temp;
			temp = temp >> 1;
		}

		//sign adjustment
		if(sign == -1)
			*dest = (int32_t)-quotient;
		else
			*dest = (int32_t)quotient;
	}
}
