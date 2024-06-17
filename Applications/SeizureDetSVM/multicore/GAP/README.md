# Epileptic Seizure Detection - Support Vector Machine - Fixed point - Parallel implementation

## Building and running

You can run this application in Gapuino assuming you have set up the SDK or/and the board correctly. <br> This implementation uses the cluster for speeding up computations.

### Gapuino

Compile and run with
```sh
make clean all run platform=board/gvsoc
```

## Data files

Some of the files of the project are meant to be included others as they are,
because they contain important globs of data, for example.
The following two files condition the behaviour of the application:

* [ecg.csv](/src/ecg.csv) contains the ecg signal
used when the application does not capture data.
In order to run the application with a different signal in this condition,
you would have to change this file.
There are other signals inside the [Inc/data/] folder.

* [svm.inc](/src/svm.inc) contains the trained parameters for the svm.
To use another parameters, you should supply a file with the same format.

## Configuration file
In src/global_config.hpp you can find important configuration parameters like printing and profiling options.
