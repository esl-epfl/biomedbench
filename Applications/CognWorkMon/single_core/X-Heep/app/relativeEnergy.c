// Porting to X-Heep : Francesco Poluzzi
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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <relativeEnergy.h>
#include <utils_functions.h>
#include <filter_signal_param.h> 

my_int accDenominator=0, accNumerator=0; //store cumulative values for sample power used to calculate the relative coefficients

typedef struct{
	my_int* data_1stHalf;
	my_int* data_2ndHalf;
	my_int* data_3rdHalf;
} _dataLocalSamples_t; //data structure to have all the data in a continuous vector (trick in the init function)

typedef struct{
	uint8_t stage; //internal control for state machine
	uint8_t status; //1 in case a new batch of coefficients is available, 0 in case coefficients are being calculated
	my_int sumOfSquares; //auxiliar variable
} _relEn_stages_t;

_dataLocalSamples_t localSamples;
_relEn_stages_t relEn_status;
my_int* relEN_coeff;

inline uint8_t relEn_ReturnStatus(void){return relEn_status.status;}
inline uint8_t relEn_SetStatus(uint8_t new_status){relEn_status.status=new_status;}

uint8_t relEn_Init(uint16_t buffer_len)
{
	uint8_t status=0;
	my_int *p;

	if(buffer_len!=LONG_WINDOW)
		printf("ERROR!\n");

	relEN_coeff = (my_int *) malloc(LONG_WINDOW * sizeof(my_int));

	//store the data copy in a contiguous memory
	p = (my_int *) malloc(3*LONG_WINDOW_HALF*sizeof(my_int));
	if(!p){
		printf("MEM ERROR!\n");
	}

	if(relEN_coeff && p){
		status = 1;
		relEn_status.stage=0;
		relEn_status.status=REL_EN_START;
		relEn_status.sumOfSquares=0.0;

		localSamples.data_1stHalf = p;
		localSamples.data_2ndHalf = &p[LONG_WINDOW_HALF];
		localSamples.data_3rdHalf = &p[2*LONG_WINDOW_HALF];
	}

	return status;
}

//considers that we receive half of a long-window vector each time this function is called
//data: --W/2--|--W/2--|--W/2--
//as, to generate one long window (W) of RelEn coefficients, we need at least 3W/2 of data
uint8_t relEn(my_int *data, uint16_t len) {
	uint8_t status=REL_EN_ONGOING; //0: coefficients are being calculated, 1: vector of coefficients is full
	uint16_t towrite;

	my_int *l,*s, *relEN;
	my_int s1, s2;

	if(len!=LONG_WINDOW_HALF)
		printf("ERROR 1!\n");

	switch(relEn_status.stage)
	{
		case 0: //treat first ever-received segment-of-data (half of large-window)
			memset(relEN_coeff, 0, LONG_WINDOW_HALF*sizeof(my_int));

			//store total sum of squares for half long window
			vect_power(data, LONG_WINDOW_HALF, &accDenominator, N_DEC_REL);

			vect_copy(data, &localSamples.data_1stHalf[0], LONG_WINDOW_HALF);
			
			relEn_status.stage = 1; //goes to second segment of data
			relEn_status.status=REL_EN_ONGOING;
			break;
		case 1: //treat the second ever-received segment-of-data (second half of large-window)
			//calculate the half short-window power

			vect_power(&localSamples.data_1stHalf[LONG_WINDOW_HALF-SHORT_WINDOW_HALF], SHORT_WINDOW_HALF, &accNumerator, N_DEC_REL);

			vect_power(data, LONG_WINDOW_HALF, &s1, N_DEC_REL);
			accDenominator += s1; //adds first to second-half powers

			//calculate the other half short-window power
			vect_power(data, SHORT_WINDOW_HALF, &s1, N_DEC_REL);
			accNumerator+= s1;

			// update the data copy to the latest received
			vect_copy(data, &localSamples.data_2ndHalf[0], LONG_WINDOW_HALF);

			relEn_status.stage = 2; //goes to third segment and others to come
			break;
		case 2:

			if(relEn_status.status!=REL_EN_START && relEn_status.status!=REL_EN_ONGOING) //has to be restarted explicitly to indicate that the previous coefficients were used already
			{
				printf("ERROR 2!\n");
			}

			if(relEn_status.status==REL_EN_ONGOING)
			{
				relEN = &relEN_coeff[LONG_WINDOW_HALF];
				status = REL_EN_AVAILABLE; //relEn ready for use after the execution
			}

			if(relEn_status.status==REL_EN_START)
			{
				relEN = &relEN_coeff[0];
				status = REL_EN_ONGOING; //relEn ready for use
			}

			//calculate the RelEN coefficient for the middle sample
			*relEN++ = fx_divx(accNumerator, accDenominator, N_DEC_REL);

			//guarantee all data in a continuous memory space
			vect_copy(data, &localSamples.data_3rdHalf[0], LONG_WINDOW_HALF);

			towrite = LONG_WINDOW_HALF-1; //how many coefficients to calculate
			s = &localSamples.data_1stHalf[LONG_WINDOW_HALF-SHORT_WINDOW_HALF];
			l = &localSamples.data_1stHalf[0];
			while(towrite > 0u) //write the first part
			{
				s1 = *(s+SHORT_WINDOW);
				accNumerator += fx_mulx(s1, s1, N_DEC_REL);

				s2 = *s++;
				accNumerator -= fx_mulx(s2, s2, N_DEC_REL);

				s1 = *(l+LONG_WINDOW);
				accDenominator += fx_mulx(s1, s1, N_DEC_REL);

				s2 = *l++;
				accDenominator -= fx_mulx(s2, s2, N_DEC_REL);

				*relEN++ = fx_divx(accNumerator, accDenominator, N_DEC_REL);

				towrite--;
			}
			//the last point of data_3rdHalf has not been used yet, as it corresponds to the first coefficient of next data batch
			s1 = *(s+SHORT_WINDOW);
			accNumerator += fx_mulx(s1, s1, N_DEC_REL);

			s2 = *s++;
			accNumerator -= fx_mulx(s2, s2, N_DEC_REL);

			s1 = *(l+LONG_WINDOW);
			accDenominator += fx_mulx(s1, s1, N_DEC_REL);

			s2 = *l++;
			accDenominator -= fx_mulx(s2, s2, N_DEC_REL);

			if(relEn_status.status==REL_EN_ONGOING) //after obtaining the energy ratio, multiply it by data to get final output
			{
				vect_mult(&localSamples.data_1stHalf[0], &relEN_coeff[0], &relEN_coeff[0], LONG_WINDOW, N_DEC_REL);
			}

			vect_copy(&localSamples.data_2ndHalf[0], &localSamples.data_1stHalf[0], LONG_WINDOW_HALF);
			vect_copy(&localSamples.data_3rdHalf[0], &localSamples.data_2ndHalf[0], LONG_WINDOW_HALF);

			relEn_status.status=status;

			break;

		default:
			status = 0;
			clearAndResetRelEn();
			relEn_status.stage = 1;
			break;
	}

	return status;
}

// Resets the algorithm variables
void clearAndResetRelEn(){
	accDenominator=0;
	accNumerator=0;

}

void endRelEnModule(){
	free(relEN_coeff);
	free(localSamples.data_1stHalf);
}
