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


//////////////////////////////////////////////////////
// Author:          Stefano Albini                  //
// Contributions:   Dimitrios Samakovlis            //
// Date:            September 2023                  //
//////////////////////////////////////////////////////

#include <nonLinearFeatureExtraction.h>

// #define LOG_MIN_INPUT 1e-10
#define LOG_MIN_INPUT   1  // smallest positive value representable with N_DEC_ENTR bits (2^-N_DEC_ENTR)

void shannon_renyi_tsallis_entropy(my_int *data, my_int *sh_entropy, my_int *ren_entropy, my_int *ts_entropy, uint16_t num_bins, uint16_t value) {
	my_int d1, l1=0;
	uint16_t i=0;

    // Non need to convert LOG_MIN_INPUT otherwise it will become 1.0
	*sh_entropy = LOG_MIN_INPUT; //guarantee there is not final zero value
	*ren_entropy = LOG_MIN_INPUT;
	*ts_entropy = LOG_MIN_INPUT;

	while(i<num_bins){
		d1 = data[i++];
            *ren_entropy += fx_powx(d1, N_DEC_ENTR, fx_itox(value, N_DEC_ENTR), N_DEC_ENTR);

			l1 = fx_log2x(d1 + LOG_MIN_INPUT, N_DEC_ENTR);
			*sh_entropy += fx_mulx(l1, d1, N_DEC_ENTR);
	}

	*ts_entropy = fx_divx( (fx_itox(1, N_DEC_ENTR) - *ren_entropy), (fx_itox(value, N_DEC_ENTR) - fx_itox(1, N_DEC_ENTR)), N_DEC_ENTR);

	*ren_entropy = fx_divx(fx_log2x((*ren_entropy + LOG_MIN_INPUT), N_DEC_ENTR), (fx_itox(1, N_DEC_ENTR) - fx_itox(value, N_DEC_ENTR)), N_DEC_ENTR);

	*sh_entropy = fx_mulx( fx_itox(-1, N_DEC_ENTR), *sh_entropy, N_DEC_ENTR);

	return;

}


//calculates 37 non-linear features (entropy) and save it according to e-Glass paper feature order
//requires execution of 'NL_InitStructures' prior to utilization and 'NL_DeInitStructures' after for mem. management
void EntropyFeaturesCalcWorkload(my_int *features, my_int *hist, uint8_t num_bins){

	uint8_t sample_wlength = 2, value = 2;  // parameters for sample entropy and renyi_entropy

	//return Shannon, Renyi, and Tsallis entropy based on a histogram shaped input (values between 0~1)
	shannon_renyi_tsallis_entropy(hist, &features[shannon_en_sig], &features[renyi_en_sig], &features[tsallis_en_sig], num_bins, value);

	return;

}
