# Emotion Recognition using kNN

## Short description

This applications has 3 physiological signals as input PPG (Blood Vessel pressure), GSR, and Skin temperature. 
The goal it to classify if the patient is feeling fear or not. 
Preprocessing: averaging input timeseries
Inference: kNN where the closest training data points decide the classification ouput.

## Building and running

You can run this application on STM32L4R5ZI provided you use the CubeIDE or/and set up the board correctly.
```

## Data files

The input data are in Inc/input_signals.h. Only one input is provided.


## Configuration file

In Inc/defines.h you can find important parameters and runtime options (i.e. printing).
