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



#include "relativeEnergy.h"
#include <stdint.h>
#include "delineationConditioned.h"

#include "pmsis.h"

//circular ecg buffer and its pointers
PI_L1 int16_t ecgBuffer_re[NUM_CORES * (LONG_WINDOW + 1)];
PI_L1 uint16_t ecgBufferPointer[NUM_CORES] = {0};
PI_L1 uint16_t lastEcgBufferPointer[NUM_CORES] = {0};
PI_L1 uint16_t currentShortWindowPointer[NUM_CORES] = {0};
PI_L1 uint16_t lastShortWindowPointer[NUM_CORES] = {0};
PI_L1 uint16_t relEnBufferPointer[NUM_CORES];
PI_L1 int8_t start[NUM_CORES];
PI_L1 uint16_t multiplier = 10000;

void clearAndResetRelEn() {
	//Previously-found short and long energies for RelEn signal generation
	for(int i = 0; i < NUM_CORES; i++){

		//circular ecg buffer and its pointers
		ecgBufferPointer[i] = 0;
		lastEcgBufferPointer[i] = 0;
		currentShortWindowPointer[i] = 0;
		lastShortWindowPointer[i] = 0;
		relEnBufferPointer[i] = (LONG_WINDOW / 2);
		start[i] = 1;
	}
}
 
 
//This is the main function where rel-En coefficients are generated
uint16_t getRelEnCoefficients(uint8_t fillingBuffer, int16_t sample, uint32_t *lastShortEnergy, uint32_t *lastLongEnergy) {
 	
	int id = pi_core_id(); 
  
	if (fillingBuffer == 1) {
		
		if (ecgBufferPointer[id] >= (LONG_WINDOW / 2 - SHORT_WINDOW_HALF) && ecgBufferPointer[id] <= (LONG_WINDOW / 2 + SHORT_WINDOW_HALF)) {
			*lastShortEnergy += (sample) * (sample);
		}

		*lastLongEnergy += (sample) * (sample);
		

		if (ecgBufferPointer[id] == LONG_WINDOW - 1) {
			lastEcgBufferPointer[id] = 0;
			currentShortWindowPointer[id] = LONG_WINDOW / 2 + SHORT_WINDOW_HALF + 1;
			lastShortWindowPointer[id] = LONG_WINDOW / 2 - SHORT_WINDOW_HALF;
		} 

		return 0;

	}
	else { //If not, compute Rel-En coefficient c(n)


		*lastLongEnergy = *lastLongEnergy + (sample * sample) - (ecgBuffer_re[id * (LONG_WINDOW + 1) + lastEcgBufferPointer[id]] * ecgBuffer_re[id * (LONG_WINDOW + 1) + lastEcgBufferPointer[id]]);

		*lastShortEnergy = *lastShortEnergy + (ecgBuffer_re[id * (LONG_WINDOW + 1) + currentShortWindowPointer[id]]*ecgBuffer_re[id * (LONG_WINDOW + 1) + currentShortWindowPointer[id]]) - (ecgBuffer_re[id * (LONG_WINDOW + 1) + lastShortWindowPointer[id]]*ecgBuffer_re[id * (LONG_WINDOW + 1) + lastShortWindowPointer[id]]);

		lastEcgBufferPointer[id]++;
		if (lastEcgBufferPointer[id] > LONG_WINDOW) {
			lastEcgBufferPointer[id] = 0;
		}
		currentShortWindowPointer[id]++;
		if (currentShortWindowPointer[id] > LONG_WINDOW) {
			currentShortWindowPointer[id] = 0;
		}
		lastShortWindowPointer[id]++;
		if (lastShortWindowPointer[id] > LONG_WINDOW) {
			lastShortWindowPointer[id] = 0;
		}

		uint16_t result = 0;

		if (*lastLongEnergy > multiplier) {
			result = *lastShortEnergy / (*lastLongEnergy / multiplier);
		}
		else {
			result = (*lastShortEnergy * multiplier) / *lastLongEnergy;
		}

 		return result;
	}
 
}

void relEn_w(int32_t *arg[]){

	int16_t *ecg_w = (int16_t*) arg[0];
	int16_t *out = (int16_t*) arg[1];
	int16_t rawRelEn = 0;
	uint16_t relEnOutput;
	int16_t currentECG;
	int tmp, end; 
	 	
	uint32_t lastShortEnergy = 0;
	uint32_t lastLongEnergy = 0;
	int ix = pi_core_id();

	if (ix == 0){
		end = (BUFFER_SIZE * N)/NUM_CORES + LONG_WINDOW;
		tmp = ix * (BUFFER_SIZE * N)/NUM_CORES;

	}else if(ix == (NUM_CORES-1)){

		end = (BUFFER_SIZE * N) + LONG_WINDOW; 
		tmp = ix * ((BUFFER_SIZE * N)/NUM_CORES); 

	}else if(ix > 0 && ix < (NUM_CORES-1)) {

		end = ((ix + 1) * ((BUFFER_SIZE * N)/NUM_CORES)) + LONG_WINDOW;
		tmp = ix * ((BUFFER_SIZE * N)/NUM_CORES); 
	}

	for(int j = tmp; j < end; j++)
	{
		rawRelEn = 0;

		if (start[ix] == 0) {

			ecgBuffer_re[ix * (LONG_WINDOW + 1) + ecgBufferPointer[ix]] = ecg_w[j];

			relEnOutput = getRelEnCoefficients(start[ix], ecg_w[j], &lastShortEnergy, &lastLongEnergy);


			currentECG = ecgBuffer_re[ix * (LONG_WINDOW + 1) + relEnBufferPointer[ix]];
			rawRelEn = (relEnOutput * currentECG) / multiplier;
			ecgBufferPointer[ix]++;
			relEnBufferPointer[ix]++;

		} //The buffer is initially being filled
		else {

			ecgBuffer_re[ix * (LONG_WINDOW + 1) + ecgBufferPointer[ix]] = ecg_w[j];

			relEnOutput = getRelEnCoefficients(start[ix], ecg_w[j], &lastShortEnergy, &lastLongEnergy);
			
			if (ecgBufferPointer[ix] == LONG_WINDOW - 1) {
				rawRelEn = (relEnOutput * ecgBuffer_re[ix * (LONG_WINDOW + 1) + relEnBufferPointer[ix]]) / multiplier;
				start[ix] = 0;
				relEnBufferPointer[ix]++;
			}
			ecgBufferPointer[ix]++;
		}

		if (relEnBufferPointer[ix] > LONG_WINDOW) {
			relEnBufferPointer[ix] = 0;
		}
		if (ecgBufferPointer[ix] > LONG_WINDOW) {
			ecgBufferPointer[ix] = 0;
		}

		if(j > tmp + LONG_WINDOW - 1){
			out[j] = rawRelEn;
		}
	}
	pi_cl_team_barrier();
}
