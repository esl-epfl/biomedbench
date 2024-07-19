// Porting to X-Heep : Francesco Poluzzi
/*
Copyright 2022 Mattia Orlandi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef DECOMP_H
#define DECOMP_H

#include "matrix.h"

typedef struct {
    Matrix *emg;
    Matrix *mean_vec;
    Matrix *white_mtx;
    Matrix *sep_mtx;
    Matrix *spike_th;
    Matrix *firings;
} DecompArgs;

void decomp_fn(Matrix *emg_l2);
void decomp_entry(void *args);

#endif
