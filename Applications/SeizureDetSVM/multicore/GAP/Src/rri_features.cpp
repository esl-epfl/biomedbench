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



#include "rri_features.hpp"
extern "C" {
    #include <math.h>
}

#include "fastlomb.hpp"

PI_L1 int sum_global = 0, sqSum_global = 0, prodSum_global = 0, irregs_global = 0; 
// Statistical and Lorenz Plot Features ───────────────────────────────────────

void ExtractRriFeatures(const int32_t **arg) {
    
    // Parsing the parameters
    const int16_t* rri = (int16_t *)arg[4];
    int size = * ( (int *) arg[6]);
    RriFeats (*fts) = (RriFeats *) arg[7]; 
    fixed_t* feature = (fixed_t *) arg[8];
    
    // Handy alias
    const int FREQ = RRI_FEAT_ECG_FREQ;
    const int FREQ_SQ = FREQ*FREQ;
    
    // We try to work with integers for precision and speed.
    // Recall that rri contains very small numbers.
    
    // Dimitrios Samakovlis Parallelization
    
    // classic static scheduling partition
    int id = pi_core_id();
    register int chunk = size / NUM_CORES;
    register int start_index = id * chunk;
    if (id == NUM_CORES - 1)
	chunk += size % NUM_CORES;
    
    rri += start_index;
    
    constexpr int threshold = (RRI_FEAT_IRR_DIFF*FREQ);
    int sum = 0, sqSum = 0, prodSum = 0, irregs = 0, i;
    if (id != 0)
	prodSum += rri[i-1]*rri[i];
    for (i = 0; i < chunk - 1; ++i) {
        sum += rri[i];
	sqSum += rri[i]*rri[i];
	prodSum += rri[i+1]*rri[i];
	if (abs(rri[i+1] - rri[i]) > threshold) {
            irregs += 1;
        }
    }
    sum += rri[i];
    sqSum += rri[i]*rri[i];
    
    pi_cl_team_critical_enter();
    sum_global += sum;
    sqSum_global += sqSum;
    prodSum_global += prodSum;
    irregs_global += irregs;
    pi_cl_team_critical_exit();

    int dsize = size - 1;
    
    pi_cl_team_barrier();
    
    if (id == NUM_CORES - 1)	{
	(*fts).stats.mean = fx_divx(sum_global, size * FREQ, RRI_FRAC);
	(*fts).stats.variance = fx_divx(size*sqSum_global - sum_global*sum_global, size*dsize*FREQ_SQ, RRI_FRAC);
	(*fts).stats.stdDev = fx_sqrtx((*fts).stats.variance, RRI_FRAC);
	feature[0] = fx_xtox((*fts).stats.mean, RRI_FRAC, SVM_FRAC);
	feature[1] = fx_xtox((*fts).stats.stdDev, RRI_FRAC, SVM_FRAC);
    }
    if (id == 0)	{
	int diffSqSum = 2*(sqSum_global - prodSum_global) - (rri[0]*rri[0] + rri[dsize]*rri[dsize]);
	int sumSqSum = 2*(sqSum_global + prodSum_global) - (rri[0]*rri[0] + rri[dsize]*rri[dsize]);
	fixed_t o2Derivative = fx_divx(diffSqSum, dsize*FREQ_SQ, RRI_FRAC);
	(*fts).stats.sqrtOrder2Derivative = fx_sqrtx(o2Derivative, RRI_FRAC);

	(*fts).stats.percIrregularities = fx_divx(irregs_global, dsize, RRI_FRAC);

	int sum1 = rri[0] - rri[dsize];
	int sum2 = 2*sum_global - rri[0] - rri[dsize];
	fixed_t var1 = fx_divx(
	    dsize*diffSqSum - sum1*sum1, dsize*(dsize - 1), RRI_FRAC
	);
	fixed_t var2 = fx_divx(
	    dsize*sumSqSum - sum2*sum2, dsize*(dsize - 1), RRI_FRAC
	);

	(*fts).lorenz.std1 = fx_sqrtx(var1/2, RRI_FRAC) / FREQ;
	(*fts).lorenz.std2 = fx_sqrtx(var2/2, RRI_FRAC) / FREQ;
	(*fts).lorenz.T = 4* (*fts).lorenz.std1;
	(*fts).lorenz.L = 4* (*fts).lorenz.std2;
	(*fts).lorenz.csi = fx_divx((*fts).lorenz.std2, (*fts).lorenz.std1, RRI_FRAC);
	(*fts).lorenz.modCsi = fx_mulx((*fts).lorenz.L, (*fts).lorenz.csi, RRI_FRAC);
	fixed_t mul = fx_mulx((*fts).lorenz.L, (*fts).lorenz.T, RRI_FRAC);
	(*fts).lorenz.cvi = fx_log10x(mul, RRI_FRAC);
	
	feature[2] = fx_xtox((*fts).stats.sqrtOrder2Derivative, RRI_FRAC, SVM_FRAC);
	feature[3] = fx_xtox((*fts).stats.percIrregularities, RRI_FRAC, SVM_FRAC);
	feature[8] = fx_xtox((*fts).lorenz.std1, RRI_FRAC, SVM_FRAC);
	feature[9] = fx_xtox((*fts).lorenz.std2, RRI_FRAC, SVM_FRAC);
	feature[10] = fx_xtox((*fts).lorenz.L, RRI_FRAC, SVM_FRAC);
	feature[11] = fx_xtox((*fts).lorenz.T, RRI_FRAC, SVM_FRAC);
	feature[12] = fx_xtox((*fts).lorenz.csi, RRI_FRAC, SVM_FRAC);
	feature[13] = fx_xtox((*fts).lorenz.modCsi, RRI_FRAC, SVM_FRAC);
	feature[14] = fx_xtox((*fts).lorenz.cvi, RRI_FRAC, SVM_FRAC);
    }
    
    pi_cl_team_barrier();
    
    return;
}

// Frequency Features ─────────────────────────────────────────────────────────
PI_L1 fixed_t *power;
PI_L1 fixed_t maxFreq; // Hz
PI_L1 fixed_t timeSpan; // s
PI_L1 fixed_t ofac;

void ExtractRriFreqFeatures(const int32_t **arg) {
    // Parsing the parameters
    const int16_t* rPeak = (int16_t *)arg[2];
    const int16_t* rri = (int16_t *)arg[4];
    int rriSize = * ( (int *)arg[6]);
    RriFreqFeats *feats = (RriFreqFeats *) arg[7];
    fixed_t* feature = (fixed_t *) arg[8];
    fixed_t *hrv = (fixed_t *)arg[9];
    fixed_t *rriTime = (fixed_t *)arg[10];
    fx_rdiv_t *ifreq = (fx_rdiv_t *)arg[11];
    
    // Dimitrios Samakovlis Parallelization
    // classic static scheduling partition
    int nfs;
    int id = pi_core_id();
    register int chunk = rriSize / NUM_CORES;
    register int start_index = id * chunk;
    if (id == NUM_CORES - 1)
	chunk += rriSize % NUM_CORES;
    
    rri += start_index;
    rPeak += start_index;
    hrv += start_index;
    rriTime += start_index;
    
    fixed_t totPow = 0;
    // computations from FC in procedure.cpp integrated here
    fx_invx(fx_itox(ECG_FREQ, FLOMB_FRAC), FLOMB_FRAC, ifreq);
    for (int i = 0; i < chunk; ++i) {
        hrv[i] = fx_itox(ECG_FREQ, FLOMB_FRAC) / rri[i];
        rriTime[i] = fx_rdivx(fx_itox(rPeak[i], FLOMB_FRAC), ifreq);
	totPow += fx_mulx(hrv[i], hrv[i], FLOMB_FRAC);
    }
    
    pi_cl_team_critical_enter();
    (*feats).totPow += totPow;
    pi_cl_team_critical_exit();
    
    pi_cl_team_barrier();
    
    if (id == 0)	{
	maxFreq = fx_itox(1, FLOMB_FRAC); // Hz
	timeSpan = FastLomb_timeSpan(rriTime, rriSize); // s
	ofac = fx_itox(2, FLOMB_FRAC);
	
	rriTime[rriSize] = fx_rdivx(fx_itox(rPeak[rriSize], FLOMB_FRAC), ifreq);
	(*feats).totPow = fx_divx((*feats).totPow, timeSpan, FLOMB_FRAC);
    
	nfs = FastLomb_numFreqs(maxFreq, timeSpan, ofac);
	power = (fixed_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(fixed_t) * (FLOMB_MAX_SIZE/2));
	#ifdef L1_memory_analysis
	printf("Address of power: %p\n", power);
	#endif
    }
    else	{
	hrv -= start_index;
	rriTime -= start_index;
    }
    pi_cl_team_barrier();

    FastLomb_(hrv, rriTime, rriSize, timeSpan, maxFreq, power, ofac, id);
    
    pi_cl_team_barrier();

    if (id ==0)	{
		fixed_t deltaFreq = FastLomb_deltaFreq(timeSpan, ofac);
	
		// Sum frequencies
		fixed_t sumf = 0;
		fixed_t lf = 0;
		fixed_t hf = 0;
		int i = 0;
		while (deltaFreq*(i+1) < RRI_FREQ_LF_BORDER_LOW) {
	    	sumf += power[i];
	    	++i;
		}
		while (deltaFreq*(i+1) < RRI_FREQ_HF_BORDER_LOW) {
	    	lf += power[i];
	    	++i;
		}
		while (deltaFreq*(i+1) < RRI_FREQ_HF_BORDER_HIGH) {
	    	hf += power[i];
	    	++i;
		}
		sumf += lf + hf;
		while (i < nfs) {
	    	sumf += power[i];
	    	++i;
		}

		(*feats).lowFreqRatio = fx_divx(lf, sumf, FLOMB_FRAC);
		(*feats).highFreqRatio = fx_divx(hf, sumf, FLOMB_FRAC);
		(*feats).lowToHighFreqRatio = fx_divx(lf, hf, FLOMB_FRAC);
	
		feature[4] = fx_xtox((*feats).totPow, FLOMB_FRAC, SVM_FRAC);
		feature[5] = fx_xtox((*feats).lowFreqRatio, FLOMB_FRAC, SVM_FRAC);
		feature[6] = fx_xtox((*feats).highFreqRatio, FLOMB_FRAC, SVM_FRAC);
		feature[7] = fx_xtox((*feats).lowToHighFreqRatio, FLOMB_FRAC, SVM_FRAC);
		//free space
		pi_cl_l1_free(&cluster_dev, (void *) power, sizeof(fixed_t) * (FLOMB_MAX_SIZE/2));
    }
    pi_cl_team_barrier();
    
    return;
}

