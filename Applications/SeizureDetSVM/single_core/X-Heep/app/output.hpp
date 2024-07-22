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


//////////////////////////////////////////////////
// Author:          ESL team                    //
// Optimizations:   Dimitrios Samakovlis        //
// Porting to X-Heep: Francesco Poluzzi         //
/////////////////////////////////////////////////



#ifndef OUTPUT_HPP_
#define OUTPUT_HPP_

#include "rri_features.hpp"

void OutputVector(const int32_t* v, int size, const char* name);
void OutputVector(const int16_t* v, int size, const char* name);
void OutputFixVector(const int32_t* v, int size, const char* name, int FRAC);
void Output(const RriStatisticalFeats& f);
void Output(const RriLorenzFeats& f);
void Output(const RriFeats& f);
void Output(const RriFreqFeats& f);

#endif  // OUTPUT_HPP_

