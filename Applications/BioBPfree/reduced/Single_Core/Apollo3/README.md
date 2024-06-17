# Bio-BPfree on Convolutional block

## Short description

This applications performs the Bio-BPfree training on a convolutional block layer (Conv1D --> Batch Normalization --> ReLU --> Max Pooling).
<br> __This is not the full Bio-BPfree benchmark on SeizDetCNN rather a short version for only 1 layer with reduced dimensions that fits in all platforms with RAM < 280 KiB__.

Bio-BPfree uses 4 inputs (2 positive and 2 negative class cases).

<ul>
  <li>Input dimensions: 64x32 (length 64 and channels 32)</li>
  <li>Filters: 64 filters of 3x32 (length 3 and depth 32)</li>
  <li>Valid padding and stride 1 to retain the same dimension in the convolution output</li>
  <li>Max pool size: 4</li>
</ul>


## Building and running

You can run this application on Apollo 3 assuming you have set up the SDK 3 or/and the board correctly.


### Apollo 3 Blue

Compile with ```cd gcc && make all```

Note because of relative paths in Makefile, the folder should be placed 4 layers in depth from the SDK home directory (i.e. ApolloSDK/boards/apollo3_evb/examples/<App_root_folder>).

Run by flashing the gcc/bin/CognWorkMon.bin file with JFlashLite. Program starting address should be 0xc000 in JFLash parameters.

## Data files

The input data are in Inc/parameters.h. The data are random and do not influence the computation profile of the application.

## Configuration file

In Inc/main.h you can configure the application (i.e. printing, input/filter dimensions).
