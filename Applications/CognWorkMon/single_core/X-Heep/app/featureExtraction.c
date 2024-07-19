// Porting to X-Heep : Francesco Poluzzi
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

#include <featureExtraction.h>
#include <statisticalFeatureExtraction.h>
#include <powerfeatureExtraction.h>
#include <nonLinearFeatureExtraction.h>
#include <utils_functions.h>

#define N_VOTING_SECTIONS 3

_features_t features_eeg;

unsigned int FeatureExtraction(my_int *features, my_int *sig, uint8_t channel)
{
	static unsigned char status = 0;
	static unsigned short sampling_frequency = SAMPLING_FREQ;
	static uint8_t num_bins; // used for histogram
	my_int  *hist;

	num_bins = StatisticalFeatures_ReturnHistogramNBins();
	my_int cpy_data_feat[WINDOW_LENGTH];
	hist = (my_int *)malloc(num_bins * sizeof(my_int));

	if (!cpy_data_feat || !hist)
	{
		printf("ERROR!
");
	}

	vect_copy(sig, cpy_data_feat, WINDOW_LENGTH);
	change_bit_depth(cpy_data_feat, cpy_data_feat, WINDOW_LENGTH, N_DEC_BIQ, N_DEC_STAT);

	/* Statistics */
	status = StatisticalFeaturesExtraction(cpy_data_feat, WINDOW_LENGTH, channel);
	if (status)
	{
		/* Complete window was processed. Get the values */ // should occour at window 28
		StatisticalFeatures_GetFinalValues(features, channel);
		StatisticalFeatures_GetHistogram(hist, channel);
		StatisticalFeatures_ReInitiate(channel);

		// change the representation for the statistical features
		change_bit_depth(&features[std], &features[std], 1, N_DEC_STAT, N_DEC_ENTR);
		change_bit_depth(&features[skewness], &features[skewness], 5, N_DEC_STAT, N_DEC_ENTR);

		change_bit_depth(hist, hist, num_bins, N_DEC_STAT, N_DEC_ENTR);

		/* Entropy features */
		EntropyFeaturesCalcWorkload(features, hist, num_bins);

	}

	change_bit_depth(sig, sig, WINDOW_LENGTH, N_DEC_BIQ, N_DEC_POW);

	status = PowerFeatures_BatchCalc(sig, channel);
	if (status)
	{
		PowerFeatures_GetFeat(features, channel);
	}
	free(hist);

	return status;
}

// <-------------- DECISION TREE --------------->
int decisionTreeFun(my_int x[]) {
	static uint8_t  idx_count_voting=0,  first_execution = 1;
	static int16_t previous_nBags_voting_firstLbl[N_VOTING_SECTIONS][NO_CLASSES+1], count_out=0; //memory is expect to contain: previous_nBags_voting_firstLbl = nVotes LBL0 | ... | nVotes LBLx | output
	uint8_t out[NO_BAGS];
	uint8_t test[6];
	uint8_t result[18];

	uint16_t i, j, bagi, bi, ni;
	uint8_t  output = 0, lbl;

	// now provide the class estimate that is most popular
	uint16_t count[NO_CLASSES];
	uint16_t ClassTest[NO_CLASSES];

	uint16_t max_el = 0;
	uint8_t countave=0, in;
	uint8_t storeMaxclasses[NO_CLASSES];


	for (bagi = 0; bagi < NO_BAGS; bagi++)
		out[bagi] = 0;

	for ( bagi = 0; bagi < NO_BAGS; bagi++)
	{
		out[bagi] = 0; //output for each tree
		for (bi = 0; bi < NO_BRANCHES[bagi]; bi++)
		{
			result[bi] = 1;
			for (ni = 0; ni < BRANCH_LENGTHS[bagi][bi]; ni++)
			{
				test[ni] = 0;
				if (BRANCH_LOGIC[bagi][bi][ni] == 1)
				{
					if (x[BRANCH_VECTOR_INDEX[bagi][bi][ni] - 1] < fx_ftox(BRANCH_VALUES[bagi][bi][ni], N_DEC_PSD))
					{
						test[ni] = 1;
					}
				}

				if (BRANCH_LOGIC[bagi][bi][ni] == 0)
				{
					if (x[BRANCH_VECTOR_INDEX[bagi][bi][ni] - 1] >= fx_ftox(BRANCH_VALUES[bagi][bi][ni], N_DEC_PSD))
					{
						test[ni] = 1;
					}
				}
				result[bi] = result[bi] * test[ni];
			}
		}
		for ( j = 0; j < NO_BRANCHES[bagi]; j++)
		{
			if (result[j] == 1)
			{
				out[bagi] = CLASS_LABELS[bagi][j];
			}
		}
	}


	for (j = 0; j < NO_CLASSES; j++)
		ClassTest[j] = j;

	for (i = 0; i < NO_CLASSES; i++)
		count[i] = 0;

	for (i = 0; i < NO_BAGS; i++) {
		for (j = 0; j < NO_CLASSES; j++) {
			if (out[i] == ClassTest[j])
				count[j] = count[j] + 1;
		}
	}

	for (j = 0; j < NO_CLASSES; j++) {
		if (count[j] > max_el) {
			output = j;
			max_el = count[j];
		}
	}

	countave = 0;
	for (j = 0; j < NO_CLASSES; j++) {
		if (count[j] == max_el) {
			storeMaxclasses[countave]=j;
			countave++;
		}
	}

	if (countave == 1)
	{
	} else if (countave ==0)
	{
	}
	else if (countave % 2 ==1) {
		in=(countave-1)/2;
		output=storeMaxclasses[in+1];
	}
	else if (countave % 2 ==0) {
		in=(countave/2);
		output=storeMaxclasses[in];
	}

	return output;

}
