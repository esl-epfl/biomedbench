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




#include "profile.h"
#include "pmsis.h"

#define ACTIVE

static int cos_sin_cycles = 0;

void profile_start()	{
	
#ifdef ACTIVE
	pi_perf_conf((1<<PI_PERF_CYCLES) | (1<<PI_PERF_ACTIVE_CYCLES));
#endif
#ifdef STALL
	pi_perf_conf((1<<PI_PERF_LD_STALL) | (1<<PI_PERF_IMISS));
#endif
#ifdef EXTACC
	pi_perf_conf((1<<PI_PERF_LD_EXT_CYC) | (1<<PI_PERF_ST_EXT_CYC));
#endif
#ifdef INTACC
	pi_perf_conf((1<<PI_PERF_LD) | (1<<PI_PERF_ST));
#endif
#ifdef INSTRUCTION
	pi_perf_conf((1<<PI_PERF_INSTR));
#endif
#ifdef TCDM
	pi_perf_conf((1<<PI_PERF_TCDM_CONT));
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


void profile_stop()	{
	    
	int id = pi_core_id();
    
	pi_perf_stop();

	cos_sin_cycles += pi_perf_read (PI_PERF_CYCLES);

#ifdef ACTIVE
	printf("[%d] cycles = %d\n", id, pi_perf_read (PI_PERF_CYCLES));
	printf("[%d] active cycles = %d\n", id, pi_perf_read (PI_PERF_ACTIVE_CYCLES));
#endif
#ifdef STALL
	printf("[%d] LD stall = %d\n", id, pi_perf_read (PI_PERF_LD_STALL));
	printf("[%d] IMISS = %d\n", id, pi_perf_read (PI_PERF_IMISS));
#endif
#ifdef EXTACC
	printf("[%d] PI_PERF_LD_EXT_CYC = %d\n", id, pi_perf_read (PI_PERF_LD_EXT_CYC));
	printf("[%d] PI_PERF_ST_EXT_CYC = %d\n", id, pi_perf_read (PI_PERF_ST_EXT_CYC));
#endif
#ifdef INTACC
	printf("[%d] PI_PERF_LD_CYC = %d\n", id, pi_perf_read (PI_PERF_LD));
	printf("[%d] PI_PERF_ST_CYC = %d\n", id, pi_perf_read (PI_PERF_ST));
#endif
#ifdef INSTRUCTION
	printf("[%d] instr = %d\n", id, pi_perf_read (PI_PERF_INSTR));	
#endif
#ifdef TCDM
	printf("[%d] PI_PERF_TCDM_CONT = %d\n", id, pi_perf_read (PI_PERF_TCDM_CONT));	
#endif
#ifdef JUMPS
    printf("[%d] Unconditional jumps = %d\n", id, pi_perf_read (PI_PERF_JUMP));
#endif
#ifdef BRANCHES
    printf("[%d] Total branches = %d\n", id, pi_perf_read (PI_PERF_BRANCH));
#endif
#ifdef TAKEN_BRANCHES
    printf("[%d] Branches taken = %d\n", id, pi_perf_read (PI_PERF_BTAKEN));
#endif
}

