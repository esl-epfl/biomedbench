# Random-projection Heartbeat Classifier

## Building and running

You can run this application on GAPuino / GAP9 assuming you have set up the SDK or/and the board correctly.

### GAPuino

Compile and run with
```sh
make --file=Gapuino.mk clean all run platform=board/gvsoc
```

### GAP9

Compile and run with
```sh
make --file=GAP9.mk clean all run platform=board/gvsoc
```

## Data files

The input data are in Inc/data


## Configuration file

In Inc/defines.h you can find important configuration parameters like printing and profiling options.
