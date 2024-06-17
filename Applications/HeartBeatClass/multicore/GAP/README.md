# Random-projection Heartbeat Classifier - Parallel implementation

## Building and running

You can run this application in Gapuino assuming you have set up the SDK or/and the board correctly. <br> This implementation uses the cluster for speeding up computations.

### Gapuino

Compile and run with
```sh
make clean all run platform=board/gvsoc
```

## Data files

The input data are in data/


## Configuration file

In defines.h you can find important configuration parameters like printing and profiling options.
