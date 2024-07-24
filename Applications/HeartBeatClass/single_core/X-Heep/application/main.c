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



#include "delineationConditioned.h"
#include "defines.h"
#include "timer_sdk.h"

int main()
{	
    #ifdef PRINT_CYCLES
        timer_init();
        timer_start();
    #endif

    // run the complete app 
    classifyBeatECG();

    #ifdef PRINT_CYCLES
        uint32_t cycles=timer_stop();
        printf("Cycles: %d\n",cycles);
    #endif
    return 0;
}
