# Parallel Epileptic Seizure Detection - CNN - Fixed point

## Building and running

You can run this application on Gapuino assuming you have set up the Pico SDK or/and the board correctly.


### Gapuino
Compile and run with
```sh
./run_PICO.sh
```
<br>Then drag and drop the .uf2 file inside build to PICO's mounted filesystem

## Data files

The input data is in Src/fist.c - hand_open.c - index.c - ok.c - rest.c 

## Configuration file

In src/defines.h there are important defines like printing options and selection of input data.
