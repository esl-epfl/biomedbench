#include "../defines.h"

// Library to deep_sleep and gpio/led enable in Raspberry Pi PICO
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/sleep.h"

#include "hardware/gpio.h"
#include "hardware/rtc.h"

#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"

// core voltage scaling
#include "hardware/vreg.h"

void sleep_callback(void)   {
    return;
}

void recover_from_sleep(uint scb_orig, uint clock0_orig, uint clock1_orig){

    //Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    //reset procs back to default
    scb_hw->scr = scb_orig;
    clocks_hw->sleep_en0 = clock0_orig;
    clocks_hw->sleep_en1 = clock1_orig;

    //reset clocks
    clocks_init();
    stdio_init_all();

    return;
}

void rtc_sleep(int seconds) {
    int minutes = 1;
    if (seconds <= 0)   {
        printf("Invalid seconds for RTC\n");
        return;
    }
    if (seconds > 60) 
    {
        minutes = 1 + seconds / 60;
        seconds = seconds % 60;
    }
        
    
    // start of RTC
    datetime_t t = {
        .year = 2022,
        .month = 4,
        .day = 05,
        .dotw = 5,
        .hour = 12,
        .min = 01,
        .sec = 00
    };
    
    // alarm tick - 10sec later
    datetime_t t_alarm = {
        .year = 2022,
        .month = 4,
        .day = 05,
        .dotw = 5,
        .hour = 12,
        .min = minutes,
        .sec = seconds
    };
        
    // save values for recovery
    uint scb_orig = scb_hw->scr;
    uint clock0_orig = clocks_hw->sleep_en0;
    uint clock1_orig = clocks_hw->sleep_en1;

    
    rtc_init();
    rtc_set_datetime(&t);
    
    sleep_goto_sleep_until(&t_alarm, &sleep_callback);
    
    //reset processor and clocks back to defaults
    recover_from_sleep(scb_orig, clock0_orig, clock1_orig);
    stdio_init_all();
}

const uint OUT_PIN = 10;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// setup gpio_ouptut and led
void gpio_ini()  {
    vreg_set_voltage(VREG_VOLTAGE_0_90);
    set_sys_clock_khz(133000, true);
    // also setup the output printing and wait to connect to minicom
    stdio_init_all();
    sleep_ms(3000); 
    
    //init gpio/led
    gpio_init(OUT_PIN);
	gpio_set_dir(OUT_PIN, GPIO_OUT);
    
    //gpio_init(LED_PIN);
    //gpio_set_dir(LED_PIN, GPIO_OUT);
}

// enable gpio out and led
void gpio_enable()   {
    gpio_put(OUT_PIN, 1);
   // gpio_put(LED_PIN, 1);
}

// disable gpio out and led
void gpio_disable()  {
    //gpio_put(LED_PIN, 0);
    gpio_put(OUT_PIN, 0);
}

