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

#ifndef SIGNAL_PRE_PROC_H
#define SIGNAL_PRE_PROC_H

#include <filter_signal_param.h>

#include <main.h>

#define PEAK_WIDTH_MINIMUM			(uint16_t)0.02*SAMPLING_FREQ		//minimum amount of samples between
#define MAX_N_PEAKS_EXPECTED		10								//how many peaks at maximum can be expected in one WINDOW
#define BLINK_N_POINTS_USED			128								//how many points are used
#define BLINK_N_POINTS_BEFORE_PEAK	32								//32/250 = 128ms before the peak
#define BLINK_N_POINTS_AFTER_PEAK	96								//384 ms = after

// Cluster entry functions
void ClusterPreProc_FindPeaks(int *arg);
void ClusterPreProc_BlinkRemoval(int *arg);

uint8_t PreProc_FindPeaks(my_int *data, uint16_t len);
uint8_t PreProc_BlinkRemoval(my_int * signal, uint16_t npeaks);

#endif
