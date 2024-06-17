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
    #ifdef GAPUINO
    #include "GAPuino/gpio_mgmt.h"
    #elif defined(GAP9)
    #include "GAP9/peripherals_sleep.h"
    #endif
    #include "pmsis.h"
}
#include "global_config.hpp"
#include "procedure.hpp"
#include "profile.hpp"
#include "lib/fastlomb.hpp"

#ifndef DATA_ACQUISITION
PI_L2 int32_t ecgData[WIN_SIZE] = {
#include "ecg.csv"
};
#endif

int main() {
    
    // Code for activating the GPIO when measuring the Gapuino board
    #ifdef GAP9
    setup_frequency_voltage(240000000, 650);
    printf("Geia\n");       // Need this as a delay to get correct measurements (correct voltage/frequency setting)
    #endif
    #if defined(GAPUINO) || defined(GAP9)
    gpio_ini();
    gpio_enable(); // enable GPIO signal and led for accurate measurements
    #endif
    
    if (MEASURE_RAW_TIME) {
        fProfileStart();
    }
    
    // Convert to new fixed-point representation
    // Keep in mind input data are 16-bit with 4 bits of decimal part
    for (int i = 0; i < WIN_SIZE; ++i) {
        ecgData[i] = fx_xtox(ecgData[i], ADC_FRAC, ECG_FRAC);
    }

    PredictSeizure((int32_t *)ecgData, WIN_SIZE);
    
    if (MEASURE_RAW_TIME) {
        fProfileStop();
    }

    #if defined (GAPUINO) || defined(GAP9)
        gpio_disable();         // disable GPIO to end measurements
    #endif

    return 0;
}
