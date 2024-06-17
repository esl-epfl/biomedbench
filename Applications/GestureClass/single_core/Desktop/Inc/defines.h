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


//////////////////////////////////////////
// Author:  Dimitrios Samakovlis        //
// Date:    February 2023               //
//////////////////////////////////////////


#ifndef _DEFINES_H_
#define _DEFINES_H_

// *** EXECUTION PARAMETERS ***

// 1. Signal parameters
#define FS_ 4000
#define WIN_LEN 200
#define N_CH 16

// 2. Decomposition parameters
#define FE 4
#define N_MU 19
#define Q 32

// 3. Classification parameters
#define N_TA 4
#define N_CA 16
#define N_OUT 5

// *** PRINTING - PROFILING OPTIONS ***
#define PRINTING
//#define PROFILING
// ***  ***

// *** INPUT - EXPECTED OUTPUT ***
#define FIST
//#define HAND_OPEN
//#define INDEX
//#define OK
//#define REST

#endif
