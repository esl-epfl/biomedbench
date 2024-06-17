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
/////////////////////////////////////////////////



#include "profile.hpp"

extern "C" {
    #include "pmsis.h"
}

void fProfileStart() {  
#ifdef CYCLES
    pi_perf_conf((1<<PI_PERF_CYCLES) | (1<<PI_PERF_ACTIVE_CYCLES));
#endif
#ifdef INSTRUCTION
	pi_perf_conf((1<<PI_PERF_INSTR));
#endif
#ifdef JUMPS
    pi_perf_conf( (1<<PI_PERF_JUMP) );
#endif
#ifdef BRANCHES
    pi_perf_conf( (1<<PI_PERF_BRANCH) );
#endif
#ifdef TAKEN_BRANCHES
    pi_perf_conf( (1<<PI_PERF_BTAKEN) );
#endif
    
    pi_perf_reset();
    pi_perf_start();
}

void fProfileStop() {  
    pi_perf_stop();
    
#ifdef CYCLES
    printf("Raw Time (cycles): %u\n\n", pi_perf_read(PI_PERF_CYCLES));
    printf("Raw Time (active cycles): %u\n\n", pi_perf_read(PI_PERF_ACTIVE_CYCLES));
#endif
#ifdef INSTRUCTION
    printf("Total instruction: %u\n\n", pi_perf_read (PI_PERF_INSTR));
#endif
#ifdef JUMPS
    printf("Unconditional jumps: %u\n\n", pi_perf_read (PI_PERF_JUMP));
#endif
#ifdef BRANCHES
    printf("Total branches: %u\n\n", pi_perf_read (PI_PERF_BRANCH));
#endif
#ifdef TAKEN_BRANCHES
    printf("Taken branches: %u\n\n", pi_perf_read (PI_PERF_BTAKEN));
#endif
    
}

