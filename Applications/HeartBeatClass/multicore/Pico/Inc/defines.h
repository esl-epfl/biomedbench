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



#ifndef DEFINES_H_
#define DEFINES_H_ 

//========= DEFINE PRINT ========//
//#define PRINT_SIG_RAW
//#define PRINT_SIG_MF_3L
//#define PRINT_RELEN
//#define PRINT_RPEAKS
//#define PRINT_BEATCLASS
//#define PRINT_DEL

//=========== Sampling frequency ========== //
#define ECG_SAMPLING_FREQUENCY 250

//=========== Number of leads/channels ============//
#define NLEADS 3

//=========== Buffer size for R peak detection ========== //
//The minumum distance between two ECG peaks based on physiological limits: 200 miliseconds * sampling frequency
#define BUFFER_LENGTH_SECONDS (float) 1.75 //2.048 //
#define BUFFER_SIZE (int16_t) (BUFFER_LENGTH_SECONDS*ECG_SAMPLING_FREQUENCY)

//=========== Overlap for Rel-En ========== //
//long window delay length (in samples): sampling frequency times long window length in seconds
//Long window length = 0.95 seconds
#define LONG_WINDOW (uint16_t) (0.95*ECG_SAMPLING_FREQUENCY+1)

//======== DEFINE MODULES ==========//
#define MODULE_MF_3L
#define MODULE_RMS_3L
#define MODULE_RELEN
#define MODULE_RPEAK
#define MODULE_BEATCLASS
#define MODULE_DEL_3L
#define MODULE_STEP2

//======== DEFINE WINDOWS (ONLY FOR DEBUG) ==========//
#define ONLY_FIRST_WINDOW
//#define ONLY_TWO_WINDOW
// #define DEBUG_FIRST_MODULES

// Number of windows collected depend on the number of available core
#define N 8
#define H_B 30
#define dim  (int)((BUFFER_SIZE * N) + LONG_WINDOW)

#define MUL 
#define SCALE 100//6//		64

#endif
