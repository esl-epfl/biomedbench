# Bio-BPfree - Version 1

## Short description

This is the optimized version of BioBPfree.

## State of optimizations
We apply following optimizations compared to version 0:
1. The forward pass is calculated in only one loop for all the layers involved and for the 4 input samples (aggressive loop fusion). 
2. The gradient of the loss function with respect to the layer output is calculated in the same loop and we avoid storing the 4 output vectors.
3. We exploit the sparsity of the gradients of the ReLU and MaxPool layer by storing only the index of the neuron that was selected through both this layers. 
4. We do not calculate the full gradient of the batch normalization layer, but rather directly divide with the normalization value the selected neurons (batch normalization layer is frozen).
5. We do not do vector-vector multiplication for the convolution gradient but rather do sum of rows exploiting the sparsity of the gradient of the loss function after applying the ReLU/MaxPool gradients.
6. We calculate the Adam step in a single loop and avoid saving intermediate arrays.

All the above optimizations combined (opt. 1, 3, 4, 5, 6) result in huge performance gains  and extra memory saving stemming (opt. 2, 6).


