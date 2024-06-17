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


//////////////////////////////////////////////////////
// Main Author:     Dimitrios Samakovlis            //
/////////////////////////////////////////////////////



#include <pico/multicore.h>

#define VOID_PICO_MSG 2167

critical_section_t crit_sec;

void pico_barrier(int id)   {
    if (id) {
        multicore_fifo_push_blocking(VOID_PICO_MSG);
        multicore_fifo_pop_blocking();
    }
    else    {
        multicore_fifo_pop_blocking();
        multicore_fifo_push_blocking(VOID_PICO_MSG);
    }
}

void pico_critical_init()   {
    critical_section_init(&crit_sec);
}

void pico_critical_enter()  {
    critical_section_enter_blocking(&crit_sec);
}

void pico_critical_exit()  {
    critical_section_exit(&crit_sec);
}
