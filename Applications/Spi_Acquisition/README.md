# SPI acquisition for X-Heep MCU Board

This application reads a 768-Bytes buffer from a SPI ADC device. Macros are avaliable for debugging and profiling.
This is a simple version that just reads the data. optimizations are possible by using DMA and by turning off the core during the transmission. Examples on how to do it can be found in the [X-Heep repository](https://github.com/esl-epfl/x-heep/).

## Building and Running

To build and run this application on the x-heep MCU board, follow the user guide at [X-Heep Documentation](https://x-heep.readthedocs.io/en/latest/index.html). 


This was tested on a [Xylinx PYNQ-Z2](https://www.mouser.com/datasheet/2/744/pynqz2_user_manual_v1_0-1525725.pdf) FPGA on the X-Heep version: `dab270bf056deb933e1f064037a7af8c30bd05b2`. 
