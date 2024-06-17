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
// Author:          Dimitrios Samakovlis        //
/////////////////////////////////////////////////



#ifdef GAP9

#include "pmsis.h"
#include "GAP9/peripherals_sleep.h"

#define RTC_DIV 0x8000
#define RTC_COUNT 1


// Retentive deep sleep wake up from rtc
void ret_deep_sleep_rtc_wakeup(void)
{
    // We will wakup from RTC, so let's set it
    struct pi_rtc_conf conf;
    pi_device_t rtc;

    pi_rtc_conf_init(&conf);
    conf.mode = PI_RTC_MODE_TIMER;
    conf.clk_div = RTC_DIV;
    conf.counter = RTC_COUNT;
    pi_open_from_conf(&rtc, &conf);

    if (pi_rtc_open(&rtc))
    {
        return -1;
    }

    pi_rtc_timer_set(&rtc, RTC_COUNT);
    pi_rtc_ioctl(&rtc, PI_RTC_TIMER_START, (void *)1);

    // Set source for wakeup
    pi_pmu_wakeup_control(PI_PMU_WAKEUP_RTC, 0);
    // Set sleep mode and go to sleep
    pi_pmu_domain_state_change(PI_PMU_DOMAIN_CHIP, PI_PMU_DOMAIN_STATE_DEEP_SLEEP_RETENTIVE, 0);
}

// It is not straightforward what we should put in this define (must trial and error to find a working solution) 
#define GPIO_WAKEUP 0b1000000000                    // this is from UART wakeup example 

// Retentive deep sleep wake up from GPIO input
void ret_deep_sleep_GPIO_wakeup(void)
{
    // Setting wake up source as gpio, and the gpio that will wakes gap9 up
    pi_pmu_wakeup_control(PI_PMU_WAKEUP_GPIO, GPIO_WAKEUP);
    // Setting sleep mode as light sleep, and going to sleep
    pi_pmu_domain_state_change(PI_PMU_DOMAIN_CHIP, PI_PMU_DOMAIN_STATE_DEEP_SLEEP_RETENTIVE, 0);

    /*
     * When we wake up, we restart here, where we stopped
     */
}

// set frequency of FC (Hz)
// set voltage of the MCU (mV <= 800)
// Return: 0 success, -1 if fail to set frequency, -2 if fail to set voltage
int setup_frequency_voltage(uint32_t frequency, uint32_t voltage)
{
    int function_check = 0;
    
    //Frequency
    function_check = pi_freq_set(PI_FREQ_DOMAIN_FC, frequency);
    if (function_check == -1)
        return -1;
    
    // for cluster use: pi_freq_set(PI_FREQ_DOMAIN_CL, frequency);
    
    // Voltage
    function_check = pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, voltage);
    if (function_check == -1)
        return -2;
    
    return 0;
}


// set frequency of FC (Hz)
// Return: 0 success, -1 if fail to set frequency
int setup_frequency_CL(uint32_t frequency)
{
    return pi_freq_set(PI_FREQ_DOMAIN_CL, frequency);
}



// initialize a GPIO pin for output
void gpio_ini()
{
    // use PI_PAD_067 -> PWM on top left females
    pi_pad_function_set(PI_PAD_067, PI_PAD_FUNC1);     // PI_PAD_FUNC1 defines GPIO usage
    pi_gpio_flags_e flags = PI_GPIO_OUTPUT;
    pi_gpio_pin_configure(PI_PAD_067, flags);
}


// toggle the previously initialized GPIO_OUTPUT_PIN
void gpio_toggle()
{
    pi_gpio_pin_toggle(PI_PAD_067);
}

// enable the previously initialized GPIO_OUTPUT_PIN
void gpio_enable()
{
    pi_gpio_pin_write(PI_PAD_067, 1);
}

// disable the previously initialized GPIO_OUTPUT_PIN
void gpio_disable()
{
    pi_gpio_pin_write(PI_PAD_067, 0);
}
#endif
