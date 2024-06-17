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

    #ifdef PICO
    #include "PICO/gpio_plus_sleep.h"
    #endif
}

#include "global_config.hpp"
#include "procedure.hpp"
#include "lib/fastlomb.hpp"

#ifndef DATA_ACQUISITION
const int ECG_SIZE = 3840;
int32_t ecgData[ECG_SIZE] = {
#include "ecg.csv"
};
#endif

int main() {
    
    #ifdef PICO
    gpio_ini();
    gpio_enable(); // enable GPIO signal and led for accurate measurements
    #endif

    // Convert to new fixed-point representation
    // Keep in mind input data are 16-bit with 4 bits of decimal part
    for (int i = 0; i < WIN_SIZE; ++i) {
        ecgData[i] = fx_xtox(ecgData[i], ADC_FRAC, ECG_FRAC);
    }


    PredictSeizure(ecgData, WIN_SIZE);

    #ifdef PICO
    gpio_disable();         // disable GPIO to end measurements
    #endif
    
    return 0;
}
