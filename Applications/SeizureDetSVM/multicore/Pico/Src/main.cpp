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



extern "C" {
    #include <stdio.h>
    #include <cstdlib>
    
    #include "Pico_management/gpio_plus_sleep.h" 
}

#include "global_config.hpp"
#include "procedure.hpp"


int32_t ecgData[WIN_SIZE] = {
#include "ecg.csv"
};

int main() {
    
    gpio_ini();
    // GPIO up to start measurements
    gpio_enable();

    // run application
    PredictSeizure(ecgData);
    
    // GPIO down to end measurements
    gpio_disable();
    
    return 0;
}
