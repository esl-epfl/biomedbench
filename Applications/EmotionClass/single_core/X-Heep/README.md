# EmotionClass Application for x-heep MCU Board

## Building and Running

To build and run this application on the x-heep MCU board, follow the user guide at [X-Heep Documentation](https://x-heep.readthedocs.io/en/latest/index.html). 


This was tested on a Xylinx PYNQ-Z2 FPGA on the X-Heep version: `dab270bf056deb933e1f064037a7af8c30bd05b2`.

On the file `defines.h`, you can find debugging and profiling macros.

### Memory Configuration

The application requires specific memory configurations to fit on the SRAM of the x-heep MCU board. To change the memory configuration, create a `custom_memory_map.hjson` file in the `configs` folder and build using:

```bash
make mcu-gen X_HEEP_CFG=configs/custom_memory_map.hjson
```

This is an example of a working memory configuration 
```hjson
{
  {
  ram_address: 0
  bus_type: "NtoM",
  ram_banks: {
    code_and_data: {
      num: 16
      sizes: [32]
    }
  }

  linker_sections:
  [
    {
      name: code
      start: 0
      size: 0x000055000   
    },
    {
      name: data
      start: 0x000055000
    }
  ]
}
}
```
Also, this application requires an increased heap memory size, configurable in the file `mcu_cfg.hjson`. An heap size of `0x10000` Bytes is enough.
