# Cough detection application with Random Forest

## Building and running

You can run this application on your Desktop. Just need to run "make all run". Make sure you put the correct version and path to your gcc compiler in the Makefile (line 8).


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
