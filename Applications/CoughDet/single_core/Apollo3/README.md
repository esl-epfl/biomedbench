# Cough Detection with Random Forest

## Building and running

You can run this application on Apollo 3 assuming you have set up the SDK or/and the board correctly.

### Apollo 3 Blue

Compile with ```cd gcc && make all```

Note because of relative paths in Makefile, the folder should be placed 4 layers in depth from the SDK home directory (i.e. ApolloSDK/boards/apollo3_evb/examples/<App_root_folder>).

Run by flashing the gcc/bin/SeizDetCNN.bin file with JFlashLite. Program starting address should be 0xc000 in JFLash parameters.

## Data files

The input data are devided in 3 files for audio, IMU and bio data.
These files are:
 - Inc/audio_input_55502_w2.h
 - Inc/imu_input_55502_w2.h
 - Inc/bio_input_55502.h

Only one window of data is provided (0.3 seconds).


## Configuration file

In Inc/launcher.h you can find relevant definition concerning the dimensioning of the window, overlapping and the feature
selection vectors.
