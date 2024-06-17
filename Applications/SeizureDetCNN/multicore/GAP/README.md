# Parallel Epileptic Seizure Detection - CNN - Fixed point

## Building and running

You can run this application in Gapuino assuming you have set up the SDK or/and the board correctly. <br> This implementation uses the Cluster for the convolutions in the first three layers.
However, we have to implement circular buffering to bring the input data to the L1 while computing (overlapping computations/transfers for maximum efficiency).


### Gapuino

Compile and run with
```sh
make --file=Gapuino.mk clean all run platform=board/gvsoc
```

## Data files

The input data is in src/fcn.c (or src/adc.csv for HEEP)

## Configuration file

In src/defines.h there are important defines for printing and profiling. In addition, it contains more configurable parameters.
