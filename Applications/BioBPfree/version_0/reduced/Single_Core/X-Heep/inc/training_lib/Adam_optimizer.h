/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */



///////////////////////////////////////////////////////////////////////////////////////////
// Title:   Adam Optimizer                                                              //
// Author:  Dimitrios Samakovlis                                                        //
// Date:    November 2023                                                               //
// Description:                                                                         //
//      Adam optimizer for training neural networks                                     //
//      Multiple instances of Adam can be instantiated through multiple Adam structs    //
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ADAM_OPTIMIZER_H_
#define _ADAM_OPTIMIZER_H_

#include "defines.h"

typedef struct Adam_struct {
    my_type beta1;      // decay rate 1 (typically 0.9)
    my_type beta1_t;    // keep track of the decay of beta1(t) without recalculating
    my_type beta2;      // decay rate 2 (typically 0.999)
    my_type beta2_t;    // keep track of the decay of beta2(t) without recalculating
    my_type alpha;      // learning rate (typically 0.001)
    my_type epsilon;    // constant to avoid 0 division (typically 10^-8)
    my_type *m;         // 1st moment vector
    my_type *u;         // 2nd raw moment vector
    int m_size;         // size of vectors
} Adam_parameters;


// Initialization of Adam parameters to start
// Involves allocation of array for momentum with size elements
void Adam_optimizer_init(Adam_parameters *Adam_params, my_type beta1, my_type beta2, my_type alpha, my_type epsilon, int size);

// Free the momentum vector memory
void Adam_optimizer_free(Adam_parameters *Adam_);

// Do one update with Adam optimizer
void Adam_optimizer_step(Adam_parameters *Adam_params, my_type *w, my_type *dLdw);

// Print the internal variables of Adam - useful for debugging
void Adam_optimizer_status(Adam_parameters *Adam_);



#endif