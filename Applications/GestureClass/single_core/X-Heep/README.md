# GestureClass Application for x-heep MCU Board

## Building and Running

To build and run this application on the x-heep MCU board, follow the user guide at [X-Heep Documentation](https://x-heep.readthedocs.io/en/latest/index.html). 


This was tested on a Xylinx PYNQ-Z2 FPGA on the X-Heep version: `7ffd2022f88ff9dd9ba970a2a00fedb85b89059b`.

On the file `defines.h`, you can find debugging and profiling macros.

### Memory Configuration

The application requires at least 6 32KB-memory banks to fit on the SRAM of the x-heep MCU board. Build with:

```bash
make mcu-gen  MEMORY_BANKS=6
```

