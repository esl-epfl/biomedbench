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

#ifndef BIQ_FILTER_H
#define BIQ_FILTER_H

#include <main.h>

typedef struct 
{
    uint32_t numStages;
    my_int *pState;
    my_int *pCoeffs;
    uint32_t postShift;
} filter_instance;


void init_filter(filter_instance *S, uint8_t numStages, my_int *pCoeffs, my_int *pState);
void biquad_filter(filter_instance *S, my_int *pSrc, my_int *pDst, my_int blockSize);

#endif
