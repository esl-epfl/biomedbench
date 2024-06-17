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



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "delineationConditioned.h" 
 
#ifdef GAPUINO
#include "GAPuino/gpio_mgmt.h"
#elif defined(GAP9)
#include "GAP9/peripherals_sleep.h"
#endif

#include "pmsis.h"
 
int main()
{
    #ifdef GAP9
    setup_frequency_voltage(240000000, 650);
    printf(" Geia \n");
    #endif

    #if defined(GAPUINO) || defined(GAP9)
    gpio_ini();
    gpio_enable(); // enable GPIO signal for accurate measurements
    #endif
    
    classifyBeatECG();
    
    #if defined(GAPUINO) || defined(GAP9)
    gpio_disable(); // disable GPIO to end measurements
    #endif
    
    return 0;
}
