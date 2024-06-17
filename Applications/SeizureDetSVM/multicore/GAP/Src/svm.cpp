/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////
// Author:          ESL team                    //  
// Optimizations:   Dimitrios Samakovlis        //
/////////////////////////////////////////////////



#include "svm.hpp"
extern "C" {
    #include "math.h"
}
#include "global_config.hpp"
#include "rri_features.hpp"
#include "edr_features.hpp"

#include <stdio.h>
// used for dynamic parallelization sync amongst cores
PI_L1 int idx_global_scale = NUM_CORES, idx_global_svm = NUM_CORES;

void predict(int32_t **arg) {
    fixed_t *feat = (fixed_t *) arg[8];
    fixed_t *sum_global = (fixed_t *) arg[11];
    
    int32_t *svmVect = arg[0], *alpha = arg[1], *scaleFeat = arg[2], *meanFeat = arg[3], *bias = arg[4]; 
    
    // DYNAMIC PARALLELIZATION
    int id = pi_core_id();
    
    // Normalize
    for (int i = id; i < NUM_FEATURES;) {
        feat[i] -= meanFeat[i];
        feat[i] = fx_mulx(feat[i], scaleFeat[i], SVM_FRAC);
	    pi_cl_team_critical_enter();
	    i = idx_global_scale++;
	    pi_cl_team_critical_exit();
    }
    
    pi_cl_team_barrier();

    // Find compound decision
    fixed_t sum = 0;
    for (int i = id; i < SVM_SIZE;) {
        fixed_t norm = 0;
        for (int j = 0; j < NUM_FEATURES; ++j) {
            fixed_t aux = feat[j] - svmVect[i*NUM_FEATURES + j];
            norm += fx_mulx(aux, aux, SVM_FRAC);
        }
        fixed_t exp = fx_expx(-norm, SVM_FRAC);
        sum += fx_mulx(alpha[i], exp, SVM_FRAC);
	    pi_cl_team_critical_enter();
	    i = idx_global_svm++;
	    pi_cl_team_critical_exit();
    }
    
    pi_cl_team_critical_enter();
    *sum_global += sum;
    pi_cl_team_critical_exit();
    
    pi_cl_team_barrier();

    if (id == 0)	{
	*sum_global += *bias;
    }
}

