# Epileptic Seizure Detection - CNN - Fixed point

## Building and running

You can run this application in Gapuino, GAP9 assuming you have set up the SDK or/and the board correctly.

### Gapuino

Compile and run with
```sh
make --file=Gapuino.mk clean all run
```

### GAP9
Compile and run with
```sh
make --file=GAP9.mk clean all run
```
<br>Then drag and drop the .uf2 file inside build to PICO's mounted filesystem

## Data files

The input data is in Src/fcn.c

## Configuring behavior

In Inc/defines.h there are important defines for printing and profiling. In addition, it contains more configurable parameters.
