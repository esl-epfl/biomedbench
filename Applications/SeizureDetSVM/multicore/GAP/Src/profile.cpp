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
    
    pi_perf_conf((1<<PI_PERF_CYCLES) | (1<<PI_PERF_ACTIVE_CYCLES));

	pi_perf_reset();
	pi_perf_start();
}

void fProfileStop() {
    int id = pi_core_id();
    
    pi_perf_stop();

    printf("[%d] cycles = %d\n", id, pi_perf_read(PI_PERF_CYCLES));
    printf("[%d] active cycles = %d\n", id, pi_perf_read(PI_PERF_ACTIVE_CYCLES));

}


void cl_profile_start(){
 
    int id = pi_core_id();
    
    if(pi_core_id()==0){
        printf("start profile\n");
    }

	pi_perf_conf((1<<PI_PERF_CYCLES) | (1<<PI_PERF_ACTIVE_CYCLES));

	pi_perf_reset();
	pi_perf_start();
}

void cl_profile_stop(){
 
    int id = pi_core_id();
		
    pi_perf_stop();
    for (int i = 0; i < NUM_CORES; i++){  
        if(pi_core_id()==i){
            printf("[%d] cycles = %d\n", id, pi_perf_read(PI_PERF_CYCLES));
            printf("[%d] active cycles = %d\n", id, pi_perf_read(PI_PERF_ACTIVE_CYCLES));
        }
        pi_cl_team_barrier();
    }
}

