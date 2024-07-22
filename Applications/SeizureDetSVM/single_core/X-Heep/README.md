# SeizDetSVM Application for X-Heep MCU Board

## Building and Running

To build and run this application on the x-heep MCU board, follow the user guide at [X-Heep Documentation](https://x-heep.readthedocs.io/en/latest/index.html). 


This was tested on a Xylinx PYNQ-Z2 FPGA on the X-Heep version: `dab270bf056deb933e1f064037a7af8c30bd05b2`.

On the file `global_configs.hpp`, you can find debugging and profiling options.

### Memory Configuration

The application requires at least 6 32KB-memory banks to fit on the SRAM of the x-heep MCU board. Build with:

```bash
make mcu-gen  MEMORY_BANKS=6
```

Also, this application requires an increased heap memory size, configurable in the file `mcu_cfg.hjson`
