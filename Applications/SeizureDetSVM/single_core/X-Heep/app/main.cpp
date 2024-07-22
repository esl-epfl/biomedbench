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
// Porting to X-Heep: Francesco Poluzzi         //
/////////////////////////////////////////////////



extern "C" {
    #include <stdio.h>
    #include <cstdlib>
}
#include "global_config.hpp"
#include "procedure.hpp"
#include "lib/fastlomb.hpp"
#include "timer_sdk.h"

#ifndef DATA_ACQUISITION
int32_t ecgData[WIN_SIZE] = {
#include "ecg.csv"
};
#endif

int main() {
    // Convert to new fixed-point representation
    // Keep in mind input data are 16-bit with 4 bits of decimal part
    if(PRINT_CYCLES){
        timer_init();
        timer_start();
    }
    for (int i = 0; i < WIN_SIZE; ++i) {
        ecgData[i] = fx_xtox(ecgData[i], ADC_FRAC, ECG_FRAC);
    }

    PredictSeizure((int32_t *)ecgData, WIN_SIZE);
    if(PRINT_CYCLES){
        uint32_t cycles=timer_stop();
        printf("Cycles: %d\n",cycles);
    }
    return 0;
}
