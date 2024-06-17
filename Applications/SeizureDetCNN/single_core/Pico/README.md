# Epileptic Seizure Detection - CNN - Fixed point

## Building and running

You can run this application in Raspberry Pi Pico assuming you have set up the SDK or/and the board correctly.

### Raspberry Pi PICO
Compile and run with
```sh
./run_PICO.sh
```
<br>Then drag and drop the .uf2 file inside build to PICO's mounted filesystem

## Data files

The input data is in Src/fcn.c

## Configuring behavior

In inc/defines.h there are important defines for printing and profiling. In addition, it contains more configurable parameters.
