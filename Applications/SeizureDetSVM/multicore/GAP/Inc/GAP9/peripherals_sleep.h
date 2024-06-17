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


//////////////////////////////////////////
// Author:  Dimitrios Samakovlis        //
// Date:    February 2023               //
//////////////////////////////////////////

#ifndef _PERIPHERALS_SLEEP_H_
#define _PERIPHERALS_SLEEP_H_

// Retentive deep sleep wake up from rtc
void ret_deep_sleep_rtc_wakeup(void);

// Retentive deep sleep wake up from GPIO input
void ret_deep_sleep_GPIO_wakeup(void);

// set frequency of FC (Hz)
// set voltage of the MCU (mV <= 800)
// Return: 0 success, -1 if fail to set frequency, -2 if fail to set voltage
int setup_frequency_voltage(uint32_t frequency, uint32_t voltage);

// set frequency of FC (Hz)
// Return: 0 success, -1 if fail to set frequency
int setup_frequency_CL(uint32_t frequency);

// initialize a GPIO pin for output
void gpio_ini();

// toggle the previously initialized GPIO_OUTPUT_PIN
void gpio_toggle();

// enable the previously initialized GPIO_OUTPUT_PIN
void gpio_enable();

// disable the previously initialized GPIO_OUTPUT_PIN
void gpio_disable();

#endif
