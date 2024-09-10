
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



#ifndef DELINEATION_CONDITIONED_H
#define DELINEATION_CONDITIONED_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "config.h" //for cntType and dType, cntType cannot be unsigned
#include "delineation.h"
#include "peakDetection.h"
#include "relativeEnergy.h"

#define FPSIZE 9 // It's included the R peak but it is always detected and the index from the beginning of the signal will be computed. Change this depending on your needs

extern uint16_t fiducial_point_code[FPSIZE];
extern uint16_t type_wave[FPSIZE];
extern uint16_t type_point[FPSIZE];

void classifyBeatECG();

#endif
