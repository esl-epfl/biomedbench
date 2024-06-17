# Bio-BPfree - Applied on SeizureDetCNN network

## Short description

This applications performs the Bio-BPfree training ([paper](https://www.researchgate.net/publication/376583511_Layer-Wise_Learning_Framework_for_Efficient_DNN_Deployment_in_Biomedical_Wearable_Systems)) on a pre-trained full SeizureDetCNN network ([paper](https://www.nature.com/articles/s41598-020-78784-3)).



SeizureDetCNN main characteristics:
<ul>
  <li>Input: 18 channels of 1024 samples</li>
  <li>3 convolutional block layers: (Conv1D --> Batch Normalization --> ReLU --> Max Pooling)</li>
  <li>Convolution: stride 1 and zero-padding</li>
  <li>Filters: 128 filters of length 3</li>
  <li>Max pooling kernel: Size 4 and stride 4</li>
  <li>2 fully-connected layers</li>
  <li>Output: 2 neurons with class probabilities after softmax</li>
</ul>

Bio-BPfree main characteristics:
<ul>
  <li>Per-layer training</li>
  <li>Loss function: Minimize distance between outputs of same class + Maximize distance between outputs of different classes</li>
  <li>Requires 4 forward passes: 2 healthy samples (class 0) + 2 unhealthy samples (class 1)</li>
  <li>Loss function on output of max pooling layer for conv blocks</li>
  <li>Trainable parameters: convolution parameters (batch normalization parameters are frozen)</li>
</ul>

## Input - FCN pretrained parameters

The input data are in Inc/inputs.h and the pretrained parameters in Inc/layer_N.h where N the number of the layer 

## Configuration file

In Inc/defines.h you can configure the application (i.e., printing, floats/doubles).
