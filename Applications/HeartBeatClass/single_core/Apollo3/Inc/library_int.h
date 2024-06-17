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


/*
 * library_int.h
 *
 *  Created on: Dec 7, 2011
 *      Author: esl-adming-elg023
 */

#ifndef LIBRARY_INT_H_
#define LIBRARY_INT_H_
#include "defines_globals.h"

data_t highpass(data_t d1, data_t d2);
data_t lowpass(data_t d1, data_t d2, data_t d3, data_t d4);
uint16_t isqrt32(int32_t* in_ptr);
int16_t myDiv(int16_t a, int16_t b);
void myDiv32_sat16(int32_t* a, int32_t* b, int32_t* dest);

#endif /* LIBRARY_INT_H_ */
