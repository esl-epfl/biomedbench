
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



#ifndef DEFINES_GLOBALS_H
#define DEFINES_GLOBALS_H

#include <stdint.h>
#include "defines.h"


/************** APPLICATION PARAMETERS *************/
#define SQUARE_ROOT_SAMPLING_FREQ_DIV_250 142/100	//Square root of 2 (500/250)
#define RMS_CONST	(1 << 14)
/***************************************************/

typedef int16_t data_t;

#endif
