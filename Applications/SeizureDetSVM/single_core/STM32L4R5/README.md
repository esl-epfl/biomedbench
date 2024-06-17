# Epileptic Seizure Detection - Support Vector Machine - Fixed point

## Building and running

You can run this application on STM32L4R5ZI provided you use the CubeIDE or/and set up the board correctly.


## Data files

Some of the files of the project are meant to be included others as they are,
because they contain important globs of data, for example.
The following two files condition the behaviour of the application:

* [ecg.csv](/Src/ecg.csv) contains the ecg signal
used when the application does not capture data.
In order to run the application with a different signal in this condition,
you would have to change this file.

* [svm.inc](/Src/svm.inc) contains the trained parameters for the svm.
To use another parameters, you should supply a file with the same format.

## Configuration file
In Inc/global_config.hpp you can find important configuration parameters like printing and profiling options.
