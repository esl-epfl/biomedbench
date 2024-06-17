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



/*==========================================================
 * delineation.h
 * ECG Delineation Algorithms
 *
 * The functions not mentionned in the header are internal functions.
 *
 *
 * The intended execution procedure is:
 *
 * R_peak_extract 			(Done with external function, for example pan-tompkin)
 *		|
 *		V
 * delineation_rr_extract 	(find the other points)
 *		|
 *		V
 * delineation_correction 	(correct possible errors and inaccuracies)
 *		|
 *		V
 * triang_wave_onoff		(calculate onset and offset points for triangular waves)
 *
 *
 *
 * 2018, Yves Rychener <yves.rychener@epfl.ch>
 *========================================================*/
#ifndef DELINEATION_H
#define DELINEATION_H

#include "config.h"  //for cntType and dType, cntType cannot be unsigned
#include <stdint.h>
#include "defines.h"

#define HALFWAVESIZE (int32_t) (ECG_SAMPLING_FREQUENCY*0.25)
#define FPSIZE 9 // It's included the R peak but it is always detected and the index from the beginning of the signal will be computed. Change this depending on your needs

#define SAMPLES_AFTER_T (int32_t) (ECG_SAMPLING_FREQUENCY*0.1) //It should be more than half wave of p (0.1); before (0.09)
#define SAMPLES_BEFORE_P (int32_t) (ECG_SAMPLING_FREQUENCY*0.06) //It should be more than half wave of t (0.06); before(0.035)
#define MAX_T_IN_P (int32_t) (ECG_SAMPLING_FREQUENCY*0.11) //max distance of T peak if T wave is in P wave (starting from the previous R peak)
#define MIN_DIST_ISO_IN_P (int32_t) (ECG_SAMPLING_FREQUENCY*0.09) //min distance to find isoline if T wave is in P wave (starting from the previous R peak)
#define MAX_HALF_QRS (int32_t) (ECG_SAMPLING_FREQUENCY*0.055)    //0.11/2 //Half QRS to find the Q and the S
#define QRS_HALF_DUR (int32_t) (ECG_SAMPLING_FREQUENCY*0.04)          //0.08/2 half duration of QRS complex
#define PWAVE_HALF_DUR (int32_t) (ECG_SAMPLING_FREQUENCY*0.05)          //0.10/2 half duration of P wave
#define TWAVE_HALF_DUR (int32_t) (ECG_SAMPLING_FREQUENCY*0.095)          //0.19/2 half duration of T wave


void delineateECG_w(int32_t *arg[]);

#endif
