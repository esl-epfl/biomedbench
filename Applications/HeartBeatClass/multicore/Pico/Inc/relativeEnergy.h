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




#ifndef __RELATIVEENERGY_H__
#define __RELATIVEENERGY_H__

#include <stdint.h>
#include "defines.h"

//long window delay length (in samples): sampling frequency times long window length in seconds
//Long window length = 0.95 seconds
#define LONG_WINDOW (uint16_t) (0.95*ECG_SAMPLING_FREQUENCY+1)

//RelEn parameters from Sasan's thesis
//Short widow = 140 ms
#define SHORT_WINDOW (uint16_t) (0.14*ECG_SAMPLING_FREQUENCY)
#define SHORT_WINDOW_HALF (uint16_t) (0.14/2*ECG_SAMPLING_FREQUENCY)
#define POWER (uint8_t) 2

//USE THIS ONE!
//Generate the relative energy raw signal used to calculate peaks
//Inputs: ecgBuffer - one long window of the signal. The first index of the buffer is the last value of the previous window
//		  lastShortEnergy - the calculated short energy of the last window
//		  lastLongEnergy - the calculated long energy of the last window
//Output: the relative energy coefficient corresponding to the buffer of values

//float getRelEnCoefficientsMoreOptimized(int16_t ecgBuffer[LONG_WINDOW + 1], int32_t* lastShortEnergy, int32_t* lastLongEnergy);

void relEn_w(int32_t *arg[], int ix);
void clearAndResetRelEn();

#endif // __RELATIVEENERGY_H__
