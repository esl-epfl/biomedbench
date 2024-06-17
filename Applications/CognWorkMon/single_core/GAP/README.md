# Random-projection Heartbeat Classifier

## Building and running

You can run this application on Gapuino / GAP9 assuming you have set up the SDK or/and the board correctly.


### Gapuino

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

The input data are in Inc/Data. Only one input is provided.


## Configuration file

In Inc/main.h and Inc/window_definition.h you can find important configuration parameters like printing options.
