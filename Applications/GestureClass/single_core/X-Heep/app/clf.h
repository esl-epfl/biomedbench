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

#ifndef CLF_H
#define CLF_H

#include "matrix.h"

typedef struct {
    Matrix *firings;
    Matrix *mlp_light1_w;
    Matrix *mlp_light1_b;
    Matrix *mlp_light2_w;
    Matrix *mlp_light2_b;
    Matrix *mlp_light3_w;
    Matrix *mlp_light3_b;
    uint8_t *class;
} MLPLightArgs;

void clf_entry(void *args);

#endif
