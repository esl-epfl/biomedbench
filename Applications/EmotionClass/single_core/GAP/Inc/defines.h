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


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Original app:    Bindi from Jose A. Miranda Calero,  Universidad Carlos III de Madrid                //
// Modified by:     Dimitrios Samakovlis, Embedded Systems Laboratory (ESL), EPFL                       //
// Date:            February 2024                                                                       //  
//////////////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _DEFINES_H_
#define _DEFINES_H_

#include "pmsis.h"

// *** ACQUISITION MODE ***
//#define DATA_ACQUISITION                      // enable that to acquire signal from SPI
#ifdef DATA_ACQUISITION
#define BUFFER_SIZE 512
#endif

// *** PROFILING - PRINTING_DETAILS OPTIONS ***
#define PRINTING_DETAILS
#define PRINTING_RESULT
// #define PROFILING_MODULES
#ifndef PROFILING_MODULES
// #define PROFILE_ALL
#endif

// *** PROFILING OPTIONS ***
#define ACTIVE           //# cycles of active cores
// #define EXTACC		    //# cycles of loads and stores in EXT memory (L2) (CLUSTER ONLY)
// #define INTACC		    //# of loads and stores in INT memory
// #define LD_STALL			//# number of core stalls
// #define INSTR_MISS	    //# number of instruction cache misses
// #define INSTRUCTION	    //# number of instructions
// #define TCDM			    //# of conflicts in TCDM (L1 memory) between cores
// #define JUMPS            //# of unconditional jumps 
// #define BRANCHES         //# of branches (taken + not taken)
// #define TAKEN_BRANCHES   //# of branches taken


// *** PARAMETERS ***
#define WINDOWS 10                              // Number of batches
#define WIN_DURATION 4                          // 4 sec per batch
#define BVP_FREQ 200                            // 200 Hz
#define GSR_FREQ 5                              // 5 Hz
#define STEMP_FREQ 1                            // 1 Hz

#define BVP_SIZE BVP_FREQ * WIN_DURATION        // Number of samples for PPG --> Blood Vessel Pressure
#define GSR_SIZE GSR_FREQ * WIN_DURATION        // Number of samples for Galvanic Skin Response
#define STEMP_SIZE STEMP_FREQ * WIN_DURATION    // Number of samples for Skin temperature sensor

#define THRESHOLD 6                             // Number of fear-classified samples above which FEAR is infered

// *** Count L2 accesses ***
extern unsigned int L2_accesses;
#define COUNT_L2_ACCESS

#endif
