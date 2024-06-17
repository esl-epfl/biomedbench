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



/*
**********************************************************************************************************************************************************
*  \details         CognWorkMon Application with deepsleep and SPI acquisition
*
*  \author          Dimitrios Smakovlis
*                   Ruben Rodriguez Alvarez
*
**********************************************************************************************************************************************************
*/

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include <stdlib.h>

#include "eglass.h"

#define     IOM_MODULE          0
#define     XOR_BYTE            0
#define     EMPTY_BYTE          0xaa
#define 	GPIO_OUTPUT_PIN		44


typedef enum
{
    AM_IOSTEST_CMD_START_DATA    = 0,
    AM_IOSTEST_CMD_STOP_DATA     = 1,
    AM_IOSTEST_CMD_ACK_DATA      = 2,
} AM_IOSTEST_CMD_E;

#define IOSOFFSET_WRITE_INTEN       0xF8
#define IOSOFFSET_WRITE_INTCLR      0xFA
#define IOSOFFSET_WRITE_CMD         0x80
#define IOSOFFSET_READ_INTSTAT      0x79
#define IOSOFFSET_READ_FIFO         0x00
#define IOSOFFSET_READ_FIFOCTR      0x7C

#define AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK  1

//#define PRINTING_ON
#define WINDOW_SIZE_BYTES    15360 
#define MAX_TRANSFER_BYTES  1024    // Max Size we can receive is 1024
uint8_t recv_buffer[MAX_TRANSFER_BYTES];	// receive location in memory
uint8_t finished = 0;
void *g_IOMHandle;

void clear_rx_buf(void);
void print_buf(void);
void iom_slave_read(uint32_t offset, uint32_t *pBuf, uint32_t size);
static void iom_set_up(uint32_t iomModule);
void am_gpio_isr(void);

// Clear the receive location in memory for the SPI 
void clear_rx_buf(void)
{
    uint32_t i;
    for ( i = 0; i < MAX_TRANSFER_BYTES; i++ )
    {
        recv_buffer[i] = EMPTY_BYTE;
    }
}

void print_buf(void)
{
    am_util_stdio_printf(" -------------- Buffer -------------- \n");
    for (int i = 0; i < MAX_TRANSFER_BYTES; i++ )
    {
        am_util_stdio_printf("0x%X ", recv_buffer[i]);
        if (i % 16 == 0)
                am_util_stdio_printf("\n");
    }
    am_util_stdio_printf("\n ------------------------------------- \n");
}

// Configuration structure for the IO Master.
static am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode = AM_HAL_IOM_SPI_MODE,
  .ui32ClockFreq = AM_HAL_IOM_8MHZ,
    .eSpiMode = AM_HAL_IOM_SPI_MODE_0,
    // Those are for non-blocking SPI transfer
    .pNBTxnBuf = (uint32_t *) recv_buffer,
    .ui32NBTxnBufLength = WINDOW_SIZE_BYTES
};

// Spi read function
void iom_slave_read(uint32_t offset, uint32_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 0;
    Transaction.ui32Instr       = offset;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32RxBuffer   = pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;
    //Transaction.ui8Priority = 1;
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
    //am_hal_iom_callback_t SPI_Callback = &SPI_Callback_implementation;
    //am_hal_iom_nonblocking_transfer(g_IOMHandle, &Transaction, SPI_Callback, NULL);
}

static void iom_set_up(uint32_t iomModule)
{
    // Initialize the IOM.
    am_hal_iom_initialize(iomModule, &g_IOMHandle);

    am_hal_iom_power_ctrl(g_IOMHandle, AM_HAL_SYSCTRL_WAKE, false);

    // Set the required configuration settings for the IOM.
    am_hal_iom_configure(g_IOMHandle, &g_sIOMSpiConfig);

    // Configure the IOM pins.
    am_bsp_iom_pins_enable(iomModule, AM_HAL_IOM_SPI_MODE);

    // Enable the IOM.
    am_hal_iom_enable(g_IOMHandle);
}



// BUTTON0 pin configuration settings.
// Set up the configuration for BUTTON0.
const am_hal_gpio_pincfg_t g_deepsleep_button0 =
{
    .uFuncSel = 3,
    .eGPInput = AM_HAL_GPIO_PIN_INPUT_ENABLE,
    .eIntDir = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
};


// setting everything in the lowest operating mode for optimal energy
int set_low_power() {
    // Configure the board for low power operation.
    am_bsp_low_power_init();
	uint32_t msg;
    while( ( msg = am_hal_pwrctrl_low_power_init() ) != AM_HAL_STATUS_SUCCESS )
#ifdef PRINTING_ON
        am_util_stdio_printf("Failed to set hal_pwrctl_low_power: %d\n", msg)
#endif
        ;
        
    // this fails and has no influence on the energy even when it suceeds
     /*   
    while ( ( msg = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_MAX) ) != AM_HAL_STATUS_SUCCESS)
#ifdef PRINTING_ON
        am_util_stdio_printf("Failed to disable all peripherals: %d\n", msg)
#endif        
        ;*/
    
#if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)

    // For optimal Deep Sleep current, configure cache to be powered-down in deepsleep:
    am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_CACHE);

    // Power down uneeded SRAM, whole FLASH, only 96K SRAM retained
    while( ( msg = am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_FLASH_1M) ) != AM_HAL_STATUS_SUCCESS )
#ifdef PRINTING_ON
        am_util_stdio_printf("Failed to set am_hal_pwrctrl_memory_deepsleep_powerdown for FLASH 1M: %d\n", msg)
#endif
    ;
    
    while( ( msg = am_hal_pwrctrl_memory_deepsleep_retain(AM_HAL_PWRCTRL_MEM_SRAM_MAX) ) != AM_HAL_STATUS_SUCCESS )
#ifdef PRINTING_ON
        am_util_stdio_printf("Failed to set am_hal_pwrctrl_memory_deepsleep_retain for SRAM 96K: %d\n", msg)
#endif
    ;
    
#endif // AM_PART_APOLLO3
}


// configuring buttons and leds
// keep the led off for best current
void set_buttons_and_leds() {
    #if defined(AM_BSP_NUM_BUTTONS)  &&  defined(AM_BSP_NUM_LEDS)
    // Configure the button pin.
    am_hal_gpio_pinconfig(AM_BSP_GPIO_BUTTON0, g_deepsleep_button0);
    
    // Configure GPIO_Output
    am_hal_gpio_pinconfig( GPIO_OUTPUT_PIN, g_AM_HAL_GPIO_OUTPUT);

    // Clear the GPIO Interrupt (write to clear).
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_BUTTON0));

    // Enable the GPIO/button interrupt.
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_BUTTON0));

    // Configure the LEDs.
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);
    // Turn the LEDs off, but initialize LED1 on so user will see something.
    for (int ix = 0; ix < AM_BSP_NUM_LEDS; ix++)
    {
        am_devices_led_off(am_bsp_psLEDs, ix);
    }
    //am_devices_led_on(am_bsp_psLEDs, 1);

#endif // defined(AM_BSP_NUM_BUTTONS)  &&  defined(AM_BSP_NUM_LEDS)
}


// **** MAIN FUNCTIONALITY IS HERE **** 
// WAKE UP FROM DEEP SLEEP AFTER BUTTON 0 PRESS
// SPI TRANSFER AND APPLICATION RUNNING
void am_gpio_isr(void)
{
    //am_devices_led_off(am_bsp_psLEDs, 1);

    // Clear the GPIO Interrupt (write to clear).
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_BUTTON0));
    
#ifdef PRINTING_ON
    am_util_stdio_printf("\n --- Clearing buffer and initiating SPI transfer (%d) ---\n", finished);
#endif
    
    //clear_rx_buf();
    
    uint32_t msg;
    // enable IOM
    while ( ( msg = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_IOM0) ) != AM_HAL_STATUS_SUCCESS)
#ifdef PRINTING_ON
         am_util_stdio_printf("Failed to enable IOM peripheral: %d\n", msg)
#endif
    ;
    iom_set_up(IOM_MODULE); 
    
    am_hal_iom_interrupt_service(g_IOMHandle, 1);
    
    // **** SPI TRANSFER *******
    // Need a SW loop to read more than 1K
    //
    // Enable GPIO
    am_hal_gpio_state_write( GPIO_OUTPUT_PIN, AM_HAL_GPIO_OUTPUT_SET);
    
    int transfer_bytes = MAX_TRANSFER_BYTES;
    int loops = WINDOW_SIZE_BYTES/MAX_TRANSFER_BYTES + (WINDOW_SIZE_BYTES % MAX_TRANSFER_BYTES ? 1 : 0);
    for (int i = 0; i < loops; i++)	{
        if (i + 1 == loops)
            transfer_bytes = WINDOW_SIZE_BYTES % MAX_TRANSFER_BYTES;
            
        iom_slave_read(IOSOFFSET_READ_FIFO, (uint32_t *)(recv_buffer), transfer_bytes);
        #ifdef PRINTING_ON
        print_buf();
        #endif
    }
    
    // disable IOM
    while ( ( msg = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM0) ) != AM_HAL_STATUS_SUCCESS)
#ifdef PRINTING_ON
        am_util_stdio_printf("Failed to enable IOM peripheral: %d\n", msg)
#endif
    ;
    
    // disable GPIO
    am_hal_gpio_state_write( GPIO_OUTPUT_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    // ******* END OF SPI TRANSFER ********
        
    //am_util_delay_ms(1000);   // small pause
    
    // ******** APPLICATION RUNNING ************
    // Enable GPIO
    am_hal_gpio_state_write( GPIO_OUTPUT_PIN, AM_HAL_GPIO_OUTPUT_SET);
    
    // Run Application
    if ( eglass() )
        #ifdef PRINTING_ON
        am_util_stdio_printf("Application ERROR - Incorrect termination!\n");
        #endif
    // disable GPIO
    am_hal_gpio_state_write( GPIO_OUTPUT_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    // ******* END OF APPLICATION *************
        
#ifdef PRINTING_ON
    am_util_stdio_printf("Finished the whole cycle of a Biomedical Application\n");
#endif
    //am_devices_led_on(am_bsp_psLEDs, 1);
}


//*****************************************************************************
//
// MAIN
//
//*****************************************************************************
int main(void)	{
    // Set the clock frequency.
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    // Set the default cache configuration
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

#ifdef PRINTING_ON
    // Initialize the printf interface for ITM/SWO output.
    am_bsp_itm_printf_enable();
	
    // Print the banner.
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Deepsleep Wake and SPI master receive Example\n");
#endif
    // Go to Deep Sleep.
    set_low_power();
    
    set_buttons_and_leds();

    // Enable GPIO interrupts to the NVIC.
    NVIC_EnableIRQ(GPIO_IRQn);

    // Enable interrupts to the core.
    am_hal_interrupt_master_enable();
    
	while (1)
    {
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
    
}
