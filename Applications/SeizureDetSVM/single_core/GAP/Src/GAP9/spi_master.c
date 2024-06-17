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

// Spi Master configuration
#define SPI_MASTER_BAUDRATE             2000000
#define SPI_MASTER_WORDSIZE             PI_SPI_WORDSIZE_8
#define SPI_MASTER_ENDIANESS            1
#define SPI_MASTER_POLARITY             PI_SPI_POLARITY_0
#define SPI_MASTER_PHASE                PI_SPI_PHASE_0
#define SPI_MASTER_CS                   1                                   // CSO (GPIO34)
#define SPI_MASTER_ITF                  1                                   // SPI1 peripheral is set as master
#define SPI_MASTER_DUMMY_CYCLE          0
#define SPI_MASTER_DUMMY_CYCLE_MODE     PI_SPI_DUMMY_CLK_CYCLE_BEFORE_CS

// Spi Master pad configuration
#define SPI_MASTER_PAD_SCK              PI_PAD_033  // Ball B2          > CN2 connector, pin 5
#define SPI_MASTER_PAD_CS0              PI_PAD_034  // Ball E4          > CN2 connector, pin 4
#define SPI_MASTER_PAD_SDO              PI_PAD_038  // Ball A1  (MOSI)  > CN2 connector, pin 1
#define SPI_MASTER_PAD_SDI              PI_PAD_039  // Ball E3  (MISO)  > CN3 connector, pin 5
#define SPI_MASTER_PAD_CS1              PI_PAD_067  // Ball E4          > CN2 connector, pin 4
#define SPI_MASTER_PAD_FUNC             PI_PAD_FUNC0
#define SPI_MASTER_IS_SLAVE             0


#define SPI_NO_OPTION                   0 // No option applied for SPI transfer
// Data size to send
#define CHUNK_SIZE  ( 512 )
#define CHUNK_SIZE_BITS (CHUNK_SIZE << 3) // Get buffer size expressed in bits
#define BUFFER_SIZE ( CHUNK_SIZE )

static uint8_t debug = 0;

// GPIO Initialization ****************************************************************************

static void spi_master_pad_init()
{
    pi_pad_function_set(SPI_MASTER_PAD_SCK, SPI_MASTER_PAD_FUNC);
    pi_pad_function_set(SPI_MASTER_PAD_SDO, SPI_MASTER_PAD_FUNC);
    pi_pad_function_set(SPI_MASTER_PAD_SDI, SPI_MASTER_PAD_FUNC);
    pi_pad_set_mux_group(SPI_MASTER_PAD_CS1, PI_PAD_MUX_GROUP_SPI1_CS1);
    pi_pad_function_set(SPI_MASTER_PAD_CS1, SPI_MASTER_PAD_FUNC);
    pi_pad_function_set(SPI_MASTER_PAD_CS0, SPI_MASTER_PAD_FUNC);
}


// SPI Initialization *****************************************************************************
static void spi_master_init(pi_device_t* spi_master, struct pi_spi_conf* spi_master_conf)
{
    pi_assert(spi_master);
    pi_assert(spi_master_conf);

    pi_spi_conf_init(spi_master_conf);
    spi_master_conf->wordsize = SPI_MASTER_WORDSIZE;
    spi_master_conf->big_endian = SPI_MASTER_ENDIANESS;
    spi_master_conf->max_baudrate = SPI_MASTER_BAUDRATE;
    spi_master_conf->polarity = SPI_MASTER_POLARITY;
    spi_master_conf->phase = SPI_MASTER_PHASE;
    spi_master_conf->itf = SPI_MASTER_ITF;
    spi_master_conf->cs = SPI_MASTER_CS;
    spi_master_conf->dummy_clk_cycle = SPI_MASTER_DUMMY_CYCLE;
    spi_master_conf->dummy_clk_cycle_mode = SPI_MASTER_DUMMY_CYCLE_MODE;
    spi_master_conf->is_slave = SPI_MASTER_IS_SLAVE;
    pi_open_from_conf(spi_master, spi_master_conf);
}


// Test *******************************************************************************************
static void receive_callback(void* context)
{
    pi_evt_push((pi_evt_t*)context);    // finish the end task to unblock execution
}


// Setup master SPI with default pins and config
// uint8_t *buffer: pointer to buffer to store the results
// size_t len:      length of transfer in bits
// return:          0 for success, 1 for fail to open SPI peripheral
int spi_master_setup_receive(uint8_t *buffer, size_t len)
{
    pi_device_t spi_master;
    struct pi_spi_conf spi_master_conf;
    pi_evt_t receive_task, end_task;

    // Initialize Gap9 EVK GPIOs
    spi_master_pad_init();
    
    // Initialize SPIs
    spi_master_init(&spi_master, &spi_master_conf);
    if (pi_spi_open(&spi_master))
    {
        printf("ERROR: Failed to open SPI peripheral\n");
        return -1;
    }
    
    // Initialize a notification event to block until spi master receive and spi slave sent
    pi_evt_sig_init(&end_task);

    // Initiate SPI master receive (internally uses uDMA)
    pi_spi_receive_async(&spi_master, buffer, len, SPI_NO_OPTION, pi_evt_callback_irq_init(&receive_task, &receive_callback, (void*)&end_task));

    // Blocking the execution until send and receive steps are done.
    pi_evt_wait(&end_task);
    
    return 0;
}

#endif