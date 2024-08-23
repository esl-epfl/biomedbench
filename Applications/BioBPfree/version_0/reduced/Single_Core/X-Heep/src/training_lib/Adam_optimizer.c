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
//////// Mapping to X-Heep: Francesco Poluzzi  \\\\


///////////////////////////////////////////////////////////////////////////////////////////
// Title:   Adam Optimizer                                                              //
// Author:  Dimitrios Samakovlis                                                        //
// Date:    November 2023                                                               //
// Description:                                                                         //
//      Adam optimizer for training neural networks                                     //
//      Multiple instances of Adam can be instantiated through multiple Adam structs    //
//////////////////////////////////////////////////////////////////////////////////////////

// Library includes
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Local includes
#include "utils.h"
#include "Adam_optimizer.h"



void Adam_optimizer_init(Adam_parameters *Adam_, my_type beta1, my_type beta2, my_type alpha, my_type epsilon, int size)    {

    // Allocate and initialize to 0 momentum vector, and squared momentum vector
    Adam_->m = (my_type *) calloc(size, sizeof(my_type));
    Adam_->u = (my_type *) calloc(size, sizeof(my_type));

    // Set the other parameters to the specified value
    Adam_->beta1 = beta1;
    Adam_->beta2 = beta2;
    Adam_->epsilon = epsilon;
    Adam_->alpha = alpha;
    Adam_->m_size = size;
    Adam_->beta1_t = beta1;
    Adam_->beta2_t = beta2;

}


void Adam_optimizer_free(Adam_parameters *Adam_)    {
    free(Adam_->m);
    free(Adam_->u);
}


void Adam_optimizer_step(Adam_parameters *Adam_, my_type *w, my_type *dLdw) {

    // Assign params to local variables - simplify expressions and improve readability
    my_type beta1 = Adam_->beta1, beta2 = Adam_->beta2, epsilon = Adam_->epsilon, alpha = Adam_->alpha;
    int size = Adam_->m_size;

    // Update 1st moment vector
    for (int i = 0; i < size; i++)   {

        Adam_->m[i] = beta1 * Adam_->m[i] + (1 - beta1) * dLdw[i];

    }

    // Update 2nd raw moment vector
    my_type *vec_temp = (my_type *)malloc(size * sizeof(my_type));

    vector_by_vector_mul_elementwise(dLdw, dLdw, vec_temp, size);

    for (int i = 0; i < size; i++)   {

        Adam_->u[i] = beta2 * Adam_->u[i] + (1 - beta2) * vec_temp[i];

    }

    // Calculate bias-corrected 1st moment
    my_type *m_hat = (my_type *)malloc(size * sizeof(my_type));

    scalar_by_vector_mul(1 / (1 - Adam_->beta1_t), Adam_->m, m_hat, size);

    Adam_->beta1_t *= beta1;    // update beta1(t)

    // Calculate bias-corrected 2nd moment
    my_type *u_hat = (my_type *)malloc(size * sizeof(my_type));

    scalar_by_vector_mul(1 / (1 - Adam_->beta2_t), Adam_->u, u_hat, size);
    vector_sqrt(u_hat, size);

    Adam_->beta2_t *= beta2;    // update beta2(t)

    // Update weights
    vector_by_vector_div_elementwise(m_hat, u_hat, vec_temp, size, epsilon);

    for (int i = 0; i < size; i++)   {

        w[i] -= alpha * vec_temp[i];

    }

    // Free dynamic memory
    free(m_hat);
    free(u_hat);
    free(vec_temp);

}


void Adam_optimizer_status(Adam_parameters *Adam_)    {

    printf("\n--- Adam optimizer status --- \n\n");

    // Print 1st moment vector
    printf("1st moment vector: \n");
    for (int i = 0; i < Adam_->m_size; i++) {
        printf("%.7e, ", Adam_->m[i]);
    }
    printf("\n\n");

    // Print 2nd moment vector
    printf("2nd moment vector: \n");
    for (int i = 0; i < Adam_->m_size; i++) {
        printf("%.7e, ", Adam_->u[i]);
    }
    printf("\n\n");

    // Print beta^t
    printf("Beta1_t: %.7e\n", Adam_->beta1_t);
    printf("Beta2_t: %.7e\n", Adam_->beta2_t);

    printf("\n---------------------------- \n\n");

}

