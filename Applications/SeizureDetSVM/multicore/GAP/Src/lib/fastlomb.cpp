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



#include "fastlomb.hpp"
#include "utils.hpp"
extern "C" {
    #include <stdint.h>
    #include <math.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include "fixmath.h"
}

#define MACC 4

PI_L1 static int FastLomb_nck[MACC] = {1, 3, 3, 1};
PI_L1 int FastLomb_fact = 6;
PI_L1 static cn *w;
PI_L1 int64_t mean_sum, var_sum;
PI_L1 fixed_t mean, var;

inline void Mean_and_Variance(const fixed_t* input, int len, int frac, int id) {
    int64_t mean_local = 0;
    int64_t var_local = 0;
    
    // STATIC SCHEDULING
    int chunk = len / NUM_CORES;
    int start_index = id * chunk;
    if (id == NUM_CORES - 1)
	chunk += len % NUM_CORES;
    
    for (int i = start_index; i < start_index + chunk; ++i) {
        mean_local += input[i];
	var_local += ((int64_t)input[i]*input[i]) >> frac;
    }
    pi_cl_team_critical_enter();
    mean_sum += mean_local;
    var_sum += var_local;
    pi_cl_team_critical_exit();
    
    pi_cl_team_barrier();
    if (id == 0)	{
	mean = mean_sum/len;
	var = (var_sum - len*(((int64_t)mean*mean) >> frac))/(len-1);
    }
}

//Initialization integrated at Fastlomb_
fixed_t FastLomb_timeSpan(const fixed_t* t, int N) {
    return ((int64_t)(t[N-1] - t[0]))*N/(N-1);
}

int FastLomb_numFreqs(fixed_t maxFreq, fixed_t timeSpan, fixed_t ofac) {
    //TODO Id like this to be ceil
    fixed_t aux1 = fx_mulx(maxFreq, ofac, FLOMB_FRAC);
    fixed_t aux2 = fx_mulx(aux1, timeSpan, FLOMB_FRAC);
    return fx_roundx(aux2, FLOMB_FRAC);
}

fixed_t FastLomb_deltaFreq(
    fixed_t timeSpan, fixed_t ofac
) {
    fixed_t aux = fx_mulx(ofac, timeSpan, FLOMB_FRAC);
    return fx_invx(aux, FLOMB_FRAC, nullptr);
}


template<bool VALS_PART>
void FastLomb_spread(fixed_t x, fixed_t y) {
    fixed_t floorX = fx_floorx(x, FLOMB_FRAC);
    if (x == fx_itox(floorX, FLOMB_FRAC)) {
        if (VALS_PART) {
                w[floorX].real += y;
        } else {
                w[floorX].imag += y;
        }
        return;
    }

    int l = floorX - (MACC/2 - 1);
    int r = l + MACC;

    fixed_t ctantTerm = fx_itox(1, FLOMB_FRAC);
    for (int i = l; i < r; ++i) {
        ctantTerm = fx_mulx(ctantTerm, x - fx_itox(i, FLOMB_FRAC), FLOMB_FRAC);
    }
    ctantTerm /= FastLomb_fact;
    ctantTerm = fx_mulx(ctantTerm, y, FLOMB_FRAC);

    for (int i = r-1; i >= l; --i) {
        fixed_t fac1 = fx_divx(ctantTerm, x - fx_itox(i, FLOMB_FRAC), FLOMB_FRAC);
        fixed_t add = fac1*FastLomb_nck[i-l];

        if (VALS_PART) {
                w[i].real += add;
        } else {
                w[i].imag += add;
        }
        ctantTerm = -ctantTerm;
    }
}

inline void FastLomb_spread_vals(
    fixed_t x, fixed_t y
) {
     FastLomb_spread<true>(x, y);
}


inline void FastLomb_spread_time(
    fixed_t x, fixed_t y
) {
    FastLomb_spread<false>(x, y);
}

PI_L1 fx_rdiv_t iFftDeltaFq, istd;
PI_L1 int global_sync;
PI_L1 int fftSize;

void FastLomb_(
    const fixed_t* x,
    const fixed_t* t,
    int len,
    fixed_t timeSpan,
    fixed_t maxFreq,
    fixed_t* output,
    fixed_t ofac,
    int id = 0
) {
    if (id == 0)	{
	
	w = (cn *) pi_cl_l1_malloc(&cluster_dev, sizeof(cn) * FLOMB_MAX_SIZE); 
	
	#ifdef L1_memory_analysis
	printf("Address of w (FastLomb): %p\n", w);
	#endif
	
	fixed_t aux0 = fx_mulx(ofac, timeSpan, FLOMB_FRAC);
	fixed_t aux1 = fx_mulx(aux0, maxFreq, FLOMB_FRAC);
	int minFftSize = 4*MACC*fx_ceilx(aux1, FLOMB_FRAC);
	fftSize = 64;
	while (fftSize < minFftSize) fftSize *= 2;
	
	// Extarpolation
	for (int i = 0; i < fftSize; ++i) {
		w[i] = {0, 0};
	}
	
	mean_sum = 0;
	var_sum = 0;
    }
    
    pi_cl_team_barrier();
    Mean_and_Variance(x, len, FLOMB_FRAC, id);
    pi_cl_team_barrier();
    
    if (id == 0)	{
	fixed_t aux = fx_mulx(ofac, timeSpan, FLOMB_FRAC);
	fx_invx(aux, FLOMB_FRAC, &iFftDeltaFq);
	fixed_t std = fx_sqrtx(var, FLOMB_FRAC);
	fx_invx(std, FLOMB_FRAC, &istd);
	global_sync = NUM_CORES;
    }
    
    pi_cl_team_barrier();
    
    for (int i = id; i < len;) {
        fixed_t fftIndex = fx_itox(MACC/2-1, FLOMB_FRAC) + fx_rdivx((t[i] - t[0]), &iFftDeltaFq)*fftSize;
        fixed_t normalized = fx_rdivx(x[i] - mean, &istd);
        FastLomb_spread_vals(fftIndex, normalized);
        fftIndex *= 2;
        FastLomb_spread_time(fftIndex, fx_itox(1, FLOMB_FRAC));
	pi_cl_team_critical_enter();
	i = global_sync++;
	pi_cl_team_critical_exit();
    }
    
    pi_cl_team_barrier();
    // Fast Fourrier Transform
   
    Fft(w, fftSize, id);
    global_sync = NUM_CORES;
    
    pi_cl_team_barrier();
    
    int nFqs = FastLomb_numFreqs(maxFreq, timeSpan, ofac);
    
    //dynamic scheduling
    for (int i = id; i < nFqs;) {
	fixed_t w2real = (w[fftSize-i-1].imag + w[i+1].imag)/2;
	fixed_t w2imag = (w[fftSize-i-1].real - w[i+1].real)/2;

        fixed_t hypo = fx_sqrtx(fx_mulx(w2real, w2real, FLOMB_FRAC) +
                                fx_mulx(w2imag, w2imag, FLOMB_FRAC),
                                FLOMB_FRAC);
        fixed_t cos2wt = fx_divx(w2real, hypo, FLOMB_FRAC);
        fixed_t sin2wt = fx_divx(w2imag, hypo, FLOMB_FRAC);
        
        fixed_t coswt_sq = (fx_itox(1, FLOMB_FRAC) + cos2wt)/2;
        fixed_t sinwt_sq = (fx_itox(1, FLOMB_FRAC) - cos2wt)/2;


	fixed_t w1real = (w[i+1].real + w[fftSize-i-1].real)/2;
	fixed_t w1imag = (w[i+1].imag - w[fftSize-i-1].imag)/2;

        fixed_t w1real_sq = fx_mulx(w1real, w1real, FLOMB_FRAC);
        fixed_t real_imag = fx_mulx(w1real, w1imag, FLOMB_FRAC);
        fixed_t w1imag_sq = fx_mulx(w1imag, w1imag, FLOMB_FRAC);
        
        fixed_t cterm = fx_mulx(coswt_sq, w1real_sq, FLOMB_FRAC)
                      + fx_mulx(sin2wt, real_imag, FLOMB_FRAC)
                      + fx_mulx(sinwt_sq, w1imag_sq, FLOMB_FRAC);
        fixed_t sterm = fx_mulx(sinwt_sq, w1real_sq, FLOMB_FRAC)
                      - fx_mulx(sin2wt, real_imag, FLOMB_FRAC)
                      + fx_mulx(coswt_sq, w1imag_sq, FLOMB_FRAC);

        cterm = fx_divx(cterm, fx_itox(len, FLOMB_FRAC) + hypo, FLOMB_FRAC);
        sterm = fx_divx(sterm, fx_itox(len, FLOMB_FRAC) - hypo, FLOMB_FRAC);
        
        output[i] = cterm + sterm;
        //TODO
	pi_cl_team_critical_enter();
	i = global_sync++;
	pi_cl_team_critical_exit();
    }
    
    pi_cl_team_barrier();
    if (id == 0)
	pi_cl_l1_free(&cluster_dev, (void *) w, sizeof(cn) * FLOMB_MAX_SIZE);
}

void FastLomb(
    const fixed_t* x,
    const fixed_t* t,
    int len,
    fixed_t maxFreq,
    fixed_t* output,
    fixed_t ofac
) {
    return FastLomb_(x, t, len, FastLomb_timeSpan(t, len), maxFreq, output, ofac);
}


