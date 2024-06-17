# Epileptic Seizure Detection - Support Vector Machine - Fixed point

## Building and running

You can run this application on Apollo 3 assuming you have set up the SDK 3 or/and the board correctly.

### Apollo 3 Blue

Compile with
```sh
cd gcc
make all

```
Note because of relative paths in Makefile, the folder should be placed 4 layers in depth from the SDK home directory (i.e. ApolloSDK/boards/apollo3_evb/examples/<App_root_folder>).

Run by flashing the gcc/bin/SeizDetSVM.bin file with JFlashLite. Program starting address should be 0xc000 in JFLash parameters.

## Data files

Some of the files of the project are meant to be included others as they are,
because they contain important globs of data, for example.
The following two files condition the behaviour of the application:

* [ecg.csv](/Inc/ecg.csv) contains the ecg signal
used when the application does not capture data.
In order to run the application with a different signal in this condition,
you would have to change this file.

* [svm.inc](/Inc/svm.inc) contains the trained parameters for the svm.
To use another parameters, you should supply a file with the same format.

## Configuration file
In src/global_config.hpp you can find important configuration parameters like printing and profiling options.
