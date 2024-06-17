# Epileptic Seizure Detection - Support Vector Machine - Fixed point

## Building and running

You can run this application in GAPuino, GAP9 assuming you have set up the SDK or/and the board correctly.

```

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

## Data file

* [ecg.csv](/src/ecg.csv) contains the unprocessed ecg input signal in 16-bit fixed point with 4 bits decimal
used when the application does not capture data.
In order to run the application with a different signal in this condition,
you would have to change this file.


## Configuration file
In Inc/global_config.hpp you can find important configuration parameters like printing and profiling options.
