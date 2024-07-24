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

#include <string.h>
#include <statisticalFeatureExtraction.h>
#include <utils_functions.h>

_statistcs_data_t   *stats;

int8_t hist_search (_histogram_t *stats, my_int *xdata, uint16_t * pos);
void StatisticalFeatures_HistogramInsert(_histogram_t *stats, my_int *data, uint16_t len);
void StatisticalFeatures_Insert(_running_statistcs_data_t *stats, my_int *data, uint16_t len);

int8_t StatisticalFeatures_Init(uint8_t nBatches, uint8_t nchannels){
	uint8_t i=0, status=0;

	if (nchannels > STATS_MAX_CH){
		return status;
	}
	   
	stats = (_statistcs_data_t*) malloc(sizeof (_statistcs_data_t));

	if(stats)
	{
		status = 1;
		stats->batches_total = nBatches;
		stats->channels = nchannels;
		stats->stats = (_statistcs_per_channel_t*)malloc (nchannels*sizeof (_statistcs_per_channel_t));
		if(stats)
		{
			while(i<nchannels)
			{
				memset(&stats->stats[i], 0, sizeof(_statistcs_per_channel_t));
				stats->stats[i].rs_data = (_running_statistcs_data_t *) malloc(sizeof (_running_statistcs_data_t));
				stats->stats[i].rs_1stDiff = (_running_statistcs_data_t *) malloc(sizeof (_running_statistcs_data_t));
				stats->stats[i].rs_2ndDiff = (_running_statistcs_data_t *) malloc(sizeof (_running_statistcs_data_t));
				stats->stats[i].histogram = (_histogram_t *) malloc(sizeof (_histogram_t));
				stats->stats[i].skew = (my_int *) malloc(nBatches * sizeof (my_int));
				stats->stats[i].kurt = (my_int *) malloc(nBatches * sizeof (my_int));
				stats->stats[i].batches_exec=0;

				if(stats->stats[i].rs_data && stats->stats[i].rs_1stDiff &&
						stats->stats[i].rs_2ndDiff && stats->stats[i].histogram &&
						stats->stats[i].kurt && stats->stats[i].skew){

					stats->stats[i].rs_data->starting_flag=1;
					stats->stats[i].rs_data->samples_counter=0;

					stats->stats[i].rs_1stDiff->starting_flag=1;
					stats->stats[i].rs_1stDiff->samples_counter=0;

					stats->stats[i].rs_2ndDiff->starting_flag=1;
					stats->stats[i].rs_2ndDiff->samples_counter=0;

					stats->stats[i].histogram->starting_flag=1;
					stats->stats[i].histogram->samples_counter=0;

				}
				else{
					status=0;
                    printf("ERROR!\n");
					break;
				}
				i++;
			}
		}
		else{
			status=0;
            printf("ERROR!\n");
		}
	}
	else{
		status=0;
        	printf("ERROR!\n");
	}

	return status;
}

//must be called after StatisticalFeatures_Init
int8_t StatisticalFeatures_InitHistogram (uint8_t nbins, my_int hist_min, my_int hist_max){
	uint8_t i=0, j=0, status=1;
	my_int f1, f2, eps;
	int32_t delta;

	while(i < stats->channels)
	{
		stats->stats[i].histogram->histogram = (uint32_t *)malloc (nbins*sizeof (uint32_t));
		stats->stats[i].histogram->hist_ranges = (my_int *)malloc ((nbins+1)*sizeof (my_int));

		if(stats->stats[i].histogram->histogram && stats->stats[i].histogram->hist_ranges)
		{
			memset(stats->stats[i].histogram->histogram, 0, nbins*sizeof(uint32_t));//initialize histogram count
			stats->stats[i].histogram->hist_bins = nbins;
			stats->stats[i].histogram->hist_min = hist_min;
			stats->stats[i].histogram->hist_max = hist_max;

			for (j = 0; j <= nbins; j++) //set ranges
			{
				f1 = fx_divx(fx_itox(nbins-j, N_DEC_STAT), fx_itox(nbins, N_DEC_STAT), N_DEC_STAT);

				f2 = fx_divx(fx_itox(j, N_DEC_STAT), fx_itox(nbins, N_DEC_STAT), N_DEC_STAT);

				f1 = fx_mulx(f1, hist_min, N_DEC_STAT) + fx_mulx(f2, hist_max, N_DEC_STAT);
				
				//Shift bins so the interval is ( ] instead of [ )
				//calculates the epsilon for the representation
				//				epsilon = EPS(X) in Matlab, which is the positive distance from ABS(X) to the next larger in
				//				magnitude floating point number of the same precision as X.
				memcpy(&delta, &f1, 4);
				if(f1<0)
					delta = delta-1;
				else
					delta = delta+1;

				memcpy(&eps, &delta, 4);

				eps = eps - f1; //epsilon
				f1 = f1+eps;

				stats->stats[i].histogram->hist_ranges[j] = (my_int)f1;
			}

			// I modified these from 0xFFFFFFFF beacuse it's 2's complement
			stats->stats[i].histogram->hist_ranges[0] = (my_int)0x7FFFFFFF *(-1); //big negative number -Inf
			stats->stats[i].histogram->hist_ranges[nbins] = (my_int)0x7FFFFFFF; //big positive number as +Inf
		}
		else{
			status=0;
			printf("ERROR!\n");
			break;
		}

		i++;
	}

	return status;
}

//must be called after StatisticalFeatures_Init
uint8_t StatisticalFeatures_ReturnHistogramNBins(void){

	if(stats->stats) //get the biggest number of bins among channels
		return stats->stats[0].histogram->hist_bins;

	return 0;
}

int8_t hist_search (_histogram_t *stats, my_int *xdata, uint16_t * pos)
{
	uint16_t lower, upper, mid;

	/* perform binary search */
	upper = stats->hist_bins;
	lower = 0 ;

	while (upper - lower > 1)
	{
		my_int denom = fx_itox(2, N_DEC_STAT);
		mid = fx_divx((upper + lower), denom, N_DEC_STAT);

		if (*xdata >= stats->hist_ranges[mid])
		{
			lower = mid ;
		}
		else
		{
			upper = mid ;
		}
	}

	*pos = lower ;

	return 0;
}

void StatisticalFeatures_HistogramInsert(_histogram_t *stats, my_int *data, uint16_t len){
	my_int *pSrc;
	uint16_t pos, l;

	l=len;
	pSrc = &data[0]; //has to be this way, otherwise 'data' pointer changes as well

	if(stats->starting_flag){
		stats->starting_flag=0;

		stats->samples_counter=1;
		pSrc = &data[1];
	}

	while((l--)>1){
		hist_search(stats, pSrc++, &pos);
		stats->histogram[pos]++; //increment histogram count
		stats->samples_counter++;
	}

	return;
}

void StatisticalFeatures_Insert(_running_statistcs_data_t *stats, my_int *data, uint16_t len){
	
	/* avrg and var will be stored with different representations (N_STAT_AVRG and N_STAT_VAR) */
	
	my_int *pSrc;
	my_int in1;
	uint16_t counter = 0;

	/* These variables are used to store the sample and average for each iteration, converted to a suitable
	   number of decimal bits, to compute avrg and var. Depending of what they will be used for, they are 
	   converted with a different number of bits for the decimal part */
	my_int smpl_for_avrg = 0;		// 17 dec bits
	my_int smpl_for_var = 0;		// 7 dec bits
	my_int avrg_old_for_avrg = 0;	// 17 dec bits
	my_int avrg_old_for_var = 0;	// 7 dec bits
	my_int avrg_for_var = 0;		// 7 dec bits 

	pSrc = &data[0];

	if(stats->starting_flag){
		stats->starting_flag=0;

		stats->avrg_old=data[0];

		// change representation to be used for avrg and var
		change_bit_depth(&stats->avrg_old, &avrg_old_for_avrg, 1, N_DEC_STAT, N_STAT_AVRG);
		change_bit_depth(&stats->avrg_old, &avrg_old_for_var, 1, N_DEC_STAT, N_STAT_VAR);

		stats->var_old = 0;

		stats->samples_counter=1;
		counter++;
		pSrc = &data[1];
	} else {
		avrg_old_for_avrg = stats->avrg_old;
		change_bit_depth(&stats->avrg_old, &avrg_for_var, 1, N_STAT_AVRG, N_STAT_VAR);
	}

	while(counter++<len){ //limit of a batch window
		in1 = *pSrc++;
		stats->samples_counter++; //goes up to the total batches sample size

		change_bit_depth(&in1, &smpl_for_avrg, 1, N_DEC_STAT, N_STAT_AVRG);	// load the sample for avrg computation

		stats->avrg = avrg_old_for_avrg + fx_divx((smpl_for_avrg - avrg_old_for_avrg), (fx_itox(stats->samples_counter, N_STAT_AVRG)), N_STAT_AVRG);

		change_bit_depth(&in1, &smpl_for_var, 1, N_DEC_STAT, N_STAT_VAR);			// sample for var computation
		change_bit_depth(&stats->avrg, &avrg_for_var, 1, N_STAT_AVRG, N_STAT_VAR);
		
		stats->var = stats->var_old + fx_mulx((smpl_for_var - avrg_old_for_var), (smpl_for_var - avrg_for_var), N_STAT_VAR);

		avrg_old_for_avrg = stats->avrg;
		change_bit_depth(&avrg_old_for_avrg, &avrg_old_for_var, 1, N_STAT_AVRG, N_STAT_VAR);

		stats->var_old = stats->var;
	}

	stats->avrg_old = avrg_old_for_avrg;

	return;

}

//unbiased var
void StatisticalFeatures_GetVAR(_running_statistcs_data_t *stats, my_int *var){

	my_int denom = fx_itox(stats->samples_counter, N_STAT_VAR) - fx_itox(1, N_STAT_VAR);
	*var = fx_divx(stats->var, denom, N_STAT_VAR);

	return;
}

void StatisticalFeatures_GetHistogram(my_int *hist, uint8_t channel)
{
	uint8_t i=0;

	while(i<stats->stats[channel].histogram->hist_bins)	{
		my_int num = fx_itox(stats->stats[channel].histogram->histogram[i], N_DEC_STAT);
		my_int denum = fx_itox(stats->stats[channel].histogram->samples_counter, N_DEC_STAT);
		hist[i] = fx_divx(num, denum, N_DEC_STAT);

		i++;
	}

	return;

}

void StatisticalFeatures_GetSTD(_running_statistcs_data_t *stats, my_int *std){
	my_int var;

	StatisticalFeatures_GetVAR(stats, &var);

	*std = (my_int)fx_sqrtx(var, N_STAT_VAR);

	return;

}


int8_t StatisticalFeaturesExtraction(my_int *data, uint16_t len, uint8_t channel){
	static my_int last1stdif[STATS_MAX_CH], last_last2nddiff[STATS_MAX_CH];

	my_int avrg, var, m3, m4, aux, aux2, last2nddiff;
	my_int  cpy_data[WINDOW_LENGTH];
	volatile uint8_t status=0;

	if(channel>=stats->channels){
		return -1;
	}

	StatisticalFeatures_HistogramInsert(stats->stats[channel].histogram, data, len); //histogram only for data

	vect_copy(data, cpy_data, len);

	//used to accumulate among various batches
	StatisticalFeatures_Insert(stats->stats[channel].rs_data, data, len);

	if(stats->stats[channel].batches_exec){ //edge workaround - 1st diff
		last1stdif[channel] = data[0]-last1stdif[channel];
		StatisticalFeatures_Insert(stats->stats[channel].rs_1stDiff, &last1stdif[channel], 1);
	}
	aux = data[len-1];

	vect_sub(&data[1], data, data, len-1);

	StatisticalFeatures_Insert(stats->stats[channel].rs_1stDiff, data, len-1);

	aux2 = data[len-2];

	if(stats->stats[channel].batches_exec){ //edge workaround - 2nd diff
		last_last2nddiff[channel] = last1stdif[channel]-last_last2nddiff[channel]; //edge workaround
		StatisticalFeatures_Insert(stats->stats[channel].rs_2ndDiff, &last_last2nddiff[channel], 1);
		last2nddiff = data[0]-last1stdif[channel]; //edge workaround
		StatisticalFeatures_Insert(stats->stats[channel].rs_2ndDiff, &last2nddiff, 1);
	}

	last1stdif[channel] = aux;
	last_last2nddiff[channel] = aux2;

	//differentiate
	vect_sub(&data[1], data, data, len-2);

	StatisticalFeatures_Insert(stats->stats[channel].rs_2ndDiff, data, len-2);
	//from here, everything is calculated per batch
	
	//has to be done per batch as final skewness and kurtosis are averaged values
	vect_mean(cpy_data, len, &avrg, N_DEC_STAT);

	//(x-u)
	vect_offset(cpy_data, -1*avrg, cpy_data, len);

	vect_copy(cpy_data, data, len);

	//variance
	vect_power(data, len, &var, N_DEC_STAT);

	//biased variance
	var = fx_divx(var, fx_itox(len, N_DEC_STAT), N_DEC_STAT);

	//(x-u)^2
	vect_mult(data, data, data, len, N_DEC_STAT);

	//sum((x-u)^3)
	vect_dot_prod(cpy_data, data, len, &m3, N_DEC_STAT);

	// m3 = m3/len;
	m3 = fx_divx(m3, fx_itox(len, N_DEC_STAT), N_DEC_STAT);

	stats->stats[channel].skew[stats->stats[channel].batches_exec] = fx_divx(m3, fx_powx(var, N_DEC_STAT, fx_ftox(1.5, N_DEC_STAT), N_DEC_STAT), N_DEC_STAT);

	/* Copy of data represented as unsigend int32_t to be used for m4 computation */
	my_int data_cpy [WINDOW_LENGTH]; //added

	change_bit_depth(data, data_cpy, len, N_DEC_STAT, 0);

	/* 
	  m4 must use 32 bits usnigened because it will be very large (2.8.. 10^9).
	  So first I use um4 (unsigend) performing the computation with 0 decimal bits.
	  Then, afetr dividing um4, I convert it back to fixpoint with N_DEC_STAT decimal bits.
	 */
	uint32_t um4;
	vect_dot_prod(data_cpy, data_cpy, len, &um4, 0);

	um4 = um4 / (uint32_t)len;

	m4 = fx_itox(um4, N_DEC_STAT);

	stats->stats[channel].kurt[stats->stats[channel].batches_exec] = fx_divx(m4, fx_mulx(var, var, N_DEC_STAT), N_DEC_STAT);

	stats->stats[channel].batches_exec++;

	if(stats->stats[channel].batches_exec>=stats->batches_total){ //re-start for next round
		status = 1;
	}

	return status;
}

void StatisticalFeatures_GetFinalValues(my_int *features, uint8_t channel){
	my_int aux1=0, aux2=0;
	uint8_t i=0;

	StatisticalFeatures_GetSTD(stats->stats[channel].rs_data, &features[std]); //std(data)
	change_bit_depth(&features[std], &features[std], 1, N_STAT_VAR, N_DEC_STAT);	//readjust the bit depth to the general one for this module

	while (i<stats->batches_total){
		aux1 += stats->stats[channel].skew[i];
		aux2 += stats->stats[channel].kurt[i];
		i++;
	}

	features[skewness] = fx_divx(aux1, fx_itox(stats->batches_total, N_DEC_STAT), N_DEC_STAT);
	features[kurtosis] = fx_divx(aux2, fx_itox(stats->batches_total, N_DEC_STAT), N_DEC_STAT);

	StatisticalFeatures_GetVAR(stats->stats[channel].rs_data, &features[hA]);
	change_bit_depth(&features[hA], &features[hA], 1, N_STAT_VAR, N_DEC_STAT);	//readjust the bit depth

	StatisticalFeatures_GetSTD(stats->stats[channel].rs_1stDiff, &aux1); //std(diff(data))
	change_bit_depth(&aux1, &aux1, 1, N_STAT_VAR, N_DEC_STAT);

	features[hM] = fx_divx(aux1, features[std], N_DEC_STAT);

	StatisticalFeatures_GetSTD(stats->stats[channel].rs_2ndDiff, &aux2); //std(diff2)
	change_bit_depth(&aux2, &aux2, 1, N_STAT_VAR, N_DEC_STAT);

	features[hC] = fx_divx(fx_divx(aux2, aux1, N_DEC_STAT), features[hM], N_DEC_STAT);

	return;
}


void StatisticalFeatures_ReInitiate(uint8_t channel)
{
	stats->stats[channel].rs_data->starting_flag=1;
	stats->stats[channel].rs_1stDiff->starting_flag=1;
	stats->stats[channel].rs_2ndDiff->starting_flag=1;

	stats->stats[channel].rs_data->samples_counter=0;
	stats->stats[channel].rs_1stDiff->samples_counter=0;
	stats->stats[channel].rs_2ndDiff->samples_counter=0;

	stats->stats[channel].batches_exec = 0;

	stats->stats[channel].histogram->samples_counter=0;
	stats->stats[channel].histogram->starting_flag=1;
	memset(stats->stats[channel].histogram->histogram, 0, stats->stats[channel].histogram->hist_bins*sizeof(uint32_t));//initialize histogram count

	return;
}

void endStatsModule(){

	int i=0;
	while(i<N_CHANNEL_USED){
		free(stats->stats[i].histogram->histogram);
		free(stats->stats[i].histogram->hist_ranges);

		free(stats->stats[i].rs_data);
		free(stats->stats[i].rs_1stDiff);
		free(stats->stats[i].rs_2ndDiff);
		free(stats->stats[i].histogram);
		free(stats->stats[i].skew);
		free(stats->stats[i].kurt);

		i++;
	}
	
	free(stats->stats);
	free(stats);
	
}
