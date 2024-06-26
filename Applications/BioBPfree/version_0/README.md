# Bio-BPfree - Version 0

## Short description

This is the first version of BioBPfree. The training library is not aggressively optimized for performance/memory.

## State of optimizations
Calculating the gradients involves 4 different forward passes and vector-vector multiplications while computing the chain rule. We avoid matrix-vector multiplications for memory saving by calculating each column on-the-fly for the gradients of each layer. <br>
Check version 1 for more optimizations (i.e., loop fusion, exploiting sparsity of gradient matrices).
