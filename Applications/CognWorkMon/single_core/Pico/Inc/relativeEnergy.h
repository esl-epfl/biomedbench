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

#ifndef RELATIVE_ENERGY_H
#define RELATIVE_ENERGY_H

#include <main.h>
#include <window_definitions.h>

#define LONG_WINDOW 		(uint16_t) (WINDOW_LENGTH)            // Window length for the denominator relEn calculation
#define LONG_WINDOW_HALF 	(uint16_t) (LONG_WINDOW/2)            // Window length for the denominator relEn calculation

#define SHORT_WINDOW 		(uint16_t) (((uint16_t)0.1*SAMPLING_FREQ)%2 ? 0.1*SAMPLING_FREQ+1:0.1*SAMPLING_FREQ)     // Window length for the numerator relEn calculation
#define SHORT_WINDOW_HALF 	(uint16_t) (SHORT_WINDOW/2)      // Window length half for the numerator relEn calculation

typedef enum _relEn_status_t{REL_EN_START=0, REL_EN_ONGOING, REL_EN_AVAILABLE} _relEn_status_t;

extern my_int* relEN_coeff;

uint8_t relEn_Init(uint16_t buffer_len);
uint8_t relEn(my_int *sample, uint16_t len);

uint8_t relEn_ReturnStatus(void);
uint8_t relEn_SetStatus(uint8_t new_status);

void clearAndResetRelEn();

void endRelEnModule();

#endif
