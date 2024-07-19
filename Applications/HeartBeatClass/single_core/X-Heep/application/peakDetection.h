// Porting to X-Heep : Francesco Poluzzi
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




#ifndef __PEAKDETECTION_H__
#define __PEAKDETECTION_H__

#include "relativeEnergy.h"
#include <stdint.h>
#include "defines.h"


#define MIN_PEAKS_DISTANCE (int16_t) (0.25*ECG_SAMPLING_FREQUENCY+1) //250 ms
#define NORMAL_PEAKS_DISTANCE (int16_t) (0.5*ECG_SAMPLING_FREQUENCY) //500 ms --> 120 bpm
#define MAX_BEATS_PER_MIN (int16_t) (BUFFER_LENGTH_SECONDS * 8)// 6 beats maximum but we count double in case the T peak is detected
// #define MAX_BEATS_PER_MIN (int16_t) (14)// 6 beats maximum but we count double in case the T peak is detected

#define TEMPORARY_PEAK_BUFFER_SIZE (int8_t) 100 

#define SMALL_THRESHOLD (int8_t) 15
#define BIG_THRESHOLD (int8_t) 40 //with 50 some outliers are solved 

#define TH_NEGATIVE_PEAK (int8_t) 70 //ratio between positive and negative peak related to the mean value on the long window
#define TH_LOW_WIDTH_RATIO (int8_t) 65 // ratio between width of two R peaks with a small minimum distance (checked 55,65,75,85,95)
#define TH_HIGH_WIDTH_RATIO (int16_t) 154 // ratio between width of two R peaks with a small minimum distance (checked 145,135,125,115,105)


void getPeaks_w(int32_t *arg[]);
void resetPeakDetection();

#endif // __PEAKDETECTION_H__
