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
 
#include "Pico_management/gpio_plus_sleep.h"
 
int main()
{
    // enable GPIO signal for accurate measurements
    gpio_ini();
    gpio_enable();
    
    // Run the app
    classifyBeatECG();
    
    // disable GPIO to end measurements
    gpio_disable(); 
    
    return 0;
}
