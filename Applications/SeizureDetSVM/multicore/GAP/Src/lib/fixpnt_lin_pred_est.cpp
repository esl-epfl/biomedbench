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



#include "fixpnt_lin_pred_est.hpp"
#include "global_config.hpp"
//TODO remove this once problem with fixmath is fixed
#include <cstdlib>
extern "C" {
    #include "fixmath.h"
    //TODO remove printfs and library include
    #include "stdio.h"
}

namespace fixpnt_32b {
PI_L1 int32_t accShift = 1;
PI_L1 uint64_t corr0 = 0;
PI_L1 int32_t corr0shift = 33;
PI_L1 fixed_t norm_corr[EDR_LPC_ORDER];
extern int global_sync = NUM_CORES;

void ComputeLinPredCoeffs_helper(int32_t* alpha) {
    fixed_t e = fx_itox(1, LPC_FRAC);
    for (int i = 0; i < EDR_LPC_ORDER; ++i) {
        fixed_t conv = 0;
        for (int j = 0; j < i; ++j) {
            conv += fx_mulx(alpha[j], norm_corr[i-1-j], LPC_FRAC);
        }
        fixed_t k = fx_divx(norm_corr[i] - conv, e, LPC_FRAC);
        for (int j = 0; j < i/2; ++j) {
            fixed_t aux = alpha[j];
            alpha[j] -= fx_mulx(k, alpha[i-1-j], LPC_FRAC);
            alpha[i-1-j] -= fx_mulx(k, aux, LPC_FRAC);
        }
        if (i%2 == 1) {
            alpha[i/2] -= fx_mulx(k, alpha[i/2], LPC_FRAC);
        }
        alpha[i] = k;
        fixed_t factor = fx_itox(1, LPC_FRAC) - fx_mulx(k, k, LPC_FRAC);
        e = fx_mulx(e, factor, LPC_FRAC);
    }
    for (int i = 0; i < EDR_LPC_ORDER; ++i) {
        alpha[i] = -alpha[i];
    }
}

//TODO remove printfs
void ComputeLinPredCoeffs(const int32_t **arg) {
    const int32_t* seq = arg[5];
    int seqSize = (int) *arg[3];
    int32_t* alpha = arg[9];
    fixed_t* feature = (fixed_t *)arg[8];
    
    int id = pi_core_id();
    int i;
    if (id ==0)	{
	while ((1<<accShift) < seqSize) {
	    accShift += 1;
	}
	// printf("pow2 %d\n", accShift);
	int minLeadingZeroes = 64;
	for (i = 0; i < seqSize; ++i) {
	    int lz = fx_clz(labs(seq[i]) | 1);
	    if (lz < minLeadingZeroes) {
		minLeadingZeroes = lz;
	    }
	}
	accShift -= 2*minLeadingZeroes;
	if (accShift < 0) accShift = 0;
    }
    
    pi_cl_team_barrier();
    
    uint64_t corr0_local = 0;
    i = id;
    while (i < seqSize) {
        corr0_local += ((int64_t)seq[i]*seq[i]) >> accShift;
	pi_cl_team_critical_enter();
	i = global_sync++;
	pi_cl_team_critical_exit();
    }
    pi_cl_team_critical_enter();
    corr0 += corr0_local;
    pi_cl_team_critical_exit();
    
    if (id == 0)	{
	while (corr0shift > 0 && (corr0 & (1LL << (30+corr0shift))) == 0) {
	    corr0shift -= 1;
	}
	global_sync = NUM_CORES;
    }
    
    pi_cl_team_barrier();
    
    i = id + 1;
    
    // 9 outer loops -> first core that finishes takes on the last loop
    correlation:
    int64_t corri = 0;
    for (int j = 0; j+i < seqSize; ++j) {
	corri += ((int64_t)seq[j]*seq[j+i]) >> accShift;
    }
    int32_t rcorr0 = corr0 >> corr0shift;
    int32_t rcorri = corri >> corr0shift;
    norm_corr[i-1] = fx_divx(rcorri, rcorr0, LPC_FRAC);
    
    pi_cl_team_critical_enter();
    i = global_sync++;
    pi_cl_team_critical_exit();
    if (i <= EDR_LPC_ORDER)
	goto correlation;
	
    pi_cl_team_barrier();
    
    if (id == 0)	{
	ComputeLinPredCoeffs_helper(alpha);
	for (i = 0; i < EDR_LPC_ORDER; ++i) {
	    feature[15+EDR_NUM_FREQ_FEATS+i] = fx_xtox(alpha[i], LPC_FRAC, SVM_FRAC);
	}
    }
    
    pi_cl_team_barrier();
    
}

}  // namespace fixpnt_32b
