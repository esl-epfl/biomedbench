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


#include "defines.h"
#include "rms.h"
#include "library_int.h"

int16_t rms_combination(int16_t *samples)
{
	int32_t sumsqr = 0;
	int16_t lead;
	int16_t c_RMS_CONST = RMS_CONST;
	uint16_t result = 0;
	for (lead = 0; lead < RMS_NLEADS; lead++){
			sumsqr += ((int32_t)((samples[lead]) + c_RMS_CONST))*((samples[lead]) + c_RMS_CONST);
	}
	result = isqrt32(&sumsqr); 
	result = (((int32_t)result*9459)>>14);	//Multiplication by sqrt(1/3) Assuming 3 leads!!!
	return result - c_RMS_CONST;
}

void rms_combinationW(int16_t* buffer, int32_t bufferSize, int16_t* output_buff){
	int16_t samples[RMS_NLEADS];
	for(int32_t indsample = 0; indsample<bufferSize;indsample++){
		for(int32_t lead=0; lead<RMS_NLEADS; lead++){
			samples[lead] = buffer[indsample+dim*lead];
		}
		output_buff[indsample] = rms_combination(samples);
	}
}

void combine_leads(int32_t *arg[])
{
	int16_t* buffer = (int16_t*) arg[0];
	int16_t* output_buff = (int16_t*) arg[1];
	int32_t* bufferSize = arg[10];
	rms_combinationW(buffer,*bufferSize,output_buff);
}
