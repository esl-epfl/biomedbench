# Cough detection application with Random Forest

## Building and running

You can run this application on Raspberry Pi Pico assuming you have set up the SDK or/and the board correctly. This implementation exploits both cores of Raspberry Pi Pico for speeding up intensive computational kernels.


### Raspberry Pi Pico
Compile and run with
```sh
./run_PICO.sh
```
<br>Then drag and drop the .uf2 file inside build to PICO's mounted filesystem


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
