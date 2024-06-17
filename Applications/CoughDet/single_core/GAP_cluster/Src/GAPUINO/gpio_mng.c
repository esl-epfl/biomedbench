/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////////
// Author:          Dimitrios Samakovlis            //
// Date:            September 2023                  //
//////////////////////////////////////////////////////



#ifdef GAPUINO

#include "pmsis.h"
	
struct pi_device gpio_a1;
struct pi_device gpio_led;
struct pi_gpio_conf gpio_conf;
pi_gpio_e gpio_out_a1;
pi_gpio_e gpio_out_led;
	
void gpio_ini() {
    pi_pmu_voltage_set(PI_PMU_DOMAIN_FC, 800); // 0.8V
    pi_freq_set(PI_FREQ_DOMAIN_FC, 150000000);  // 150MHz
    //Setting pad to alternate 1
    //GPIO A1
    // pi_pad_set_function(PI_PAD_13_B2_RF_PACTRL1, PI_PAD_13_B2_GPIO_A1_FUNC1);
    pi_pad_set_function(PI_PAD_12_A3_RF_PACTRL0, PI_PAD_12_A3_GPIO_A0_FUNC1);
    //GPIO LED (A3)
    pi_pad_set_function(PI_PAD_15_B1_RF_PACTRL3, PI_PAD_FUNC1);
	    
    // gpio_out_a1 = PI_GPIO_A1_PAD_13_B2;
    gpio_out_a1 = PI_GPIO_A0_PAD_12_A3;
    gpio_out_led = PI_GPIO_A3_PAD_15_B1;

    /* Configure gpio output. */
    pi_gpio_flags_e cfg_flags = PI_GPIO_OUTPUT;
    pi_gpio_pin_configure(&gpio_a1, gpio_out_a1, cfg_flags);
    pi_gpio_pin_configure(&gpio_led, gpio_out_led, cfg_flags);
}

// toggle the previously initialized GPIO_OUTPUT_PIN
void gpio_toggle()
{
    pi_gpio_pin_toggle(&gpio_a1, gpio_out_a1);
    pi_gpio_pin_toggle(&gpio_led, gpio_out_led);
}

void gpio_enable(){
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 1);
    pi_gpio_pin_write(&gpio_led, gpio_out_led, 1);
}

void gpio_disable(){
    pi_gpio_pin_write(&gpio_a1, gpio_out_a1, 0);
    pi_gpio_pin_write(&gpio_led, gpio_out_led, 0);
}
#endif
