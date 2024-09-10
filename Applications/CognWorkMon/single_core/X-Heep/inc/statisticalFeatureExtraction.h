
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

#ifndef STATISTICAL_FEATURE_EXTRACTION_H
#define STATISTICAL_FEATURE_EXTRACTION_H

#include <featureExtraction.h>

#define STATS_MAX_CH	N_CHANNEL_USED

typedef struct{
	uint8_t     starting_flag;
	uint16_t    samples_counter;
	my_int      avrg,
		        avrg_old,
		        var,
		        var_old;
} _running_statistcs_data_t;

typedef struct{
	uint8_t     starting_flag;
	uint8_t     hist_bins;
	uint16_t    samples_counter;
	uint32_t    *histogram;
	my_int      *hist_ranges;
	my_int      hist_min, hist_max;
} _histogram_t;

typedef struct{
	_running_statistcs_data_t   *rs_data;
	_running_statistcs_data_t   *rs_1stDiff;
	_running_statistcs_data_t   *rs_2ndDiff;
	_histogram_t                *histogram;

	my_int  *skew,
		  	*kurt;

	uint16_t 	batches_exec;
	uint16_t 	channels;

} _statistcs_per_channel_t;

typedef struct{
	uint16_t 	batches_total;
	uint16_t 	channels;

	_statistcs_per_channel_t * stats;

} _statistcs_data_t;


int8_t StatisticalFeatures_Init(uint8_t nBatches, uint8_t nchannels);
int8_t StatisticalFeatures_InitHistogram (uint8_t nbins, my_int hist_min, my_int hist_max);
void StatisticalFeatures_ReInitiate(uint8_t channel);

int8_t StatisticalFeaturesExtraction(my_int *data, uint16_t len, uint8_t channel);
void StatisticalFeatures_GetFinalValues(my_int *features, uint8_t channel);
void StatisticalFeatures_GetHistogram(my_int *hist, uint8_t channel);
uint8_t StatisticalFeatures_ReturnHistogramNBins(void);

void endStatsModule();

#endif
