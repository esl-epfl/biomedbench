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
 
#ifdef PICO
#include "PICO/gpio_plus_sleep.h"
#endif

int main()
{	
    #ifdef PICO
    gpio_ini();
    gpio_enable(); //enable GPIO signal for accurate measurements
    #endif
    
    // run the the complete app 
    classifyBeatECG();
    
    #ifdef PICO
    gpio_disable(); // disable GPIO to end measurements
    #endif

    return 0;
}
