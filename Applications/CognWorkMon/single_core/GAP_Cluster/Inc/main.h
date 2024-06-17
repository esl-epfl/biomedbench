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

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <fixmath.h>

#include "pmsis.h"


// #define USE_64  // toggles the 64bit operations 

// #define PRINT_INFO
// #define PRINT_FEAT               // to print all the final features at once
// #define PRINT_CLASSIFICATION     // to print the classification result

#define APPLY_BLINK_REMOVAL
#define FILTER_ACTIVE

// defines the number of halfindow to process before the script terminates
// If the input data is not enough, they are repeated in a cyclic way
#define N_HALFWIND_TO_PROCESS   29

#define CH_TO_STORE 4

#define N_DEC       18
#define N_DEC_BIQ   24
#define N_DEC_REL   12
#define N_DEC_BLINK 9

// Statistical feature extraction
#define N_DEC_STAT  8   // general for the module
#define N_STAT_AVRG 17  // to compute the average
#define N_STAT_VAR  7   // to compute the variance

// Entropy feature extraction
#define N_DEC_ENTR  20

// Power feature extraction
#define N_DEC_POW   18
#define N_DEC_TP    11   // used to store the total power
#define N_DEC_CMPLX 10   // used to compute the complex magnitude squared
#define N_DEC_PER   11  // used to store the final accumulated periodogram
#define N_DEC_PSD   20

typedef int32_t my_int;

#define MUL(x, y, sh) ((x * y) >> sh)


//========= DEFINE PROFILING ================//
//#define PROFILING_ON                    // choose this to profile the whole app through the FC
//#define PROFILING_CLUSTER_ON          // CHoose this to profile the cluster cores
//#define ACTIVE
//#define EXTACC		//# cycles of loads and stores in EXT memory (L2) (CLUSTER ONLY)
//#define INTACC		//# of loads and stores in INT memory
//#define STALL			//# number of core stalls
//#define INSTRUCTION	//# number of instructions
//#define TCDM			//# of conflicts in TCDM (L1 memory) between cores
//#define JUMPS         //# of unconditional jumps 
//#define BRANCHES      //# of branches (taken + not taken)
//#define TAKEN_BRANCHES //# of branches taken

extern PI_L2 struct pi_device cluster_dev;            // important for allocating memory in the cluster L1 memory

#endif
