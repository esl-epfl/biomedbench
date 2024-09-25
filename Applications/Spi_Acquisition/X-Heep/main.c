/**
 * @file main.c
 * @brief Biomedbench SPI Acquisition example - X-Heep
 * 
 * Example for reading the 768-Byte standard batch of the BioMedBench
 * benchmark (https://github.com/esl-epfl/biomedbench) trough SPI 
 * communication with an ADC device.
 *
 * @author Francesco Poluzzi
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "csr.h"
#include "spi_host.h"
#include "dma.h"
#include "x-heep.h"
#include  "timer_sdk.h"

#define FULL_BUFFER_SIZE 768
#define BUFFER_SIZE FULL_BUFFER_SIZE/3  // SPI FIFO size is 256 bytes
#define PRINT_DATA 1
#define PRINT_CYCLES 1

uint8_t read_buffer[FULL_BUFFER_SIZE];

uint8_t slave_id = 0;

int main(int argc, char *argv[]) {

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    CSR_SET_BITS(CSR_REG_MIE, 1<<20); // Enable machine-level fast spi interrupt

    spi_configopts_t configs = {
        .clkdiv = 0,  
        .csnidle = 0xF,     
        .csntrail = 0,    
        .csnlead = 0,     
        .__rsvd0 = 0,     
        .fullcyc = false,  
        .cpha = 0,  
        .cpol = 0          
    };
    spi_command_t cmd = {
        .len = BUFFER_SIZE - 1,
        .csaat = false,
        .speed = SPI_SPEED_STANDARD,
        .direction = SPI_DIR_RX_ONLY
    };

    uint32_t config_word = spi_create_configopts(configs);
    uint32_t cmd_word = spi_create_command(cmd);

    #if PRINT_CYCLES
        timer_cycles_init();
        timer_start();
    #endif

    for(int j=0; j<FULL_BUFFER_SIZE; j+=BUFFER_SIZE) {

        if (spi_set_configopts(spi_host1, slave_id, config_word) != SPI_FLAG_OK) {
            printf("Error setting SPI configuration\n");
            return EXIT_FAILURE;
        }

        if (spi_set_enable(spi_host1, 1) != SPI_FLAG_OK) {
            printf("Error enabling SPI host\n");
            return EXIT_FAILURE;
        }

        if (spi_output_enable(spi_host1, 1) != SPI_FLAG_OK) {
            printf("Error enabling SPI output\n");
            return EXIT_FAILURE;
        }


        if (spi_set_csid(spi_host1, slave_id) != SPI_FLAG_OK) {
            printf("Error setting slave id\n");
            return EXIT_FAILURE;
        }

        if (spi_set_rx_watermark(spi_host1, BUFFER_SIZE) != SPI_FLAG_OK) {
            printf("Error setting command\n");
            return EXIT_FAILURE;
        }

        spi_get_ready(spi_host1);
        spi_wait_for_ready(spi_host1);

        if (spi_set_command(spi_host1, cmd_word) != SPI_FLAG_OK) {
            printf("Error setting command\n");
            return EXIT_FAILURE;
        }

        spi_wait_for_rx_watermark(spi_host1);

        for(int i = 0; i < BUFFER_SIZE; i++) {
            spi_read_word(spi_host1, (uint32_t*)&read_buffer[i+j]);
        }

    }

    #if PRINT_CYCLES
        uint32_t cycles = timer_stop();
    #endif

    #if PRINT_DATA
        for(int i = 0; i < FULL_BUFFER_SIZE; i++) {
            printf("Data[%d] = %d\n", i, read_buffer[i]);
        }
    #endif

    #if PRINT_CYCLES
        printf("SPI acquisition took %d cycles\n", cycles);
    #endif

    return EXIT_SUCCESS;

}
