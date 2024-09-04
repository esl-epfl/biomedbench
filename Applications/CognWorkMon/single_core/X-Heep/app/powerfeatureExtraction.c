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



#include <fx_fft.h>
#include <string.h>
#include <utils_functions.h>
#include <powerfeatureExtraction.h>
#include <fx_fft.h>
#include <stdio.h>

typedef struct{
	uint8_t 	batches_exec,
				batches_total;
	uint16_t 	fs;
	uint16_t 	fftSize; // it need to be of the same size as the signal as arm_rfft_fast_f32 return an interleaved fashion representation (real, imag, real, imag, ...)
	my_int 		total_power;
	my_int 		*power_density; //store psd
} _power_features_t;

_power_features_t *power;
uint8_t ifftFlag = 0;	// might need to delete this in the future


int8_t PowerFeatureExtractionInit(uint16_t fftSize, unsigned short samp_freq, uint8_t nBatches, uint8_t nchannels){
	int8_t status=0, i=0;

    power = (_power_features_t*) malloc (nchannels*sizeof (_power_features_t));
	

	if(power){
		while(i<nchannels){
			power[i].fftSize = fftSize;
			power[i].batches_exec = 0;
			power[i].batches_total = nBatches;
			power[i].fs = samp_freq;
			power[i].total_power = 0;
            
            power[i].power_density = (my_int *) malloc ((fftSize/2+1)*sizeof (my_int));

			if(power[i].power_density){
				memset(power[i].power_density, 0, (fftSize/2+1)* sizeof (my_int));
				status=1;
			}
			else {
				printf("CANNOT ALLOCATE MEM!\n");
			}

			i++;
		}
	}

	return status;
}


int8_t PowerFeatures_BatchCalc(my_int *sig, uint8_t channel){
	my_int *sig_periodogram;
	my_int tp;
	int8_t status = 0;
    
    sig_periodogram = (my_int *) malloc ((power[channel].fftSize/2+1) * sizeof (my_int));

    my_int *sig_tp = (my_int*)malloc(power[channel].fftSize*sizeof(my_int));	// used only for total_power computation

	change_bit_depth(sig, sig_tp, power[channel].fftSize, N_DEC_POW, N_DEC_TP);
	vect_power(sig_tp, power[channel].fftSize, &tp, N_DEC_TP);
	power[channel].total_power += tp;
    
	free(sig_tp);

	//Periodogram function has to be executed after calculating p_tot (direct from the signal samples)
	Periodogram(&sig[0], sig_periodogram, power[channel].fftSize);

	vect_add(sig_periodogram, power[channel].power_density, power[channel].power_density, power[channel].fftSize/2+1);
	power[channel].batches_exec++;

	if(power[channel].batches_exec >= power[channel].batches_total){
		
		my_int scaling = fx_divx(fx_itox(1, N_DEC_PER), fx_itox(power[channel].batches_total, N_DEC_PER), N_DEC_PER);
		vect_scale(power[channel].power_density, scaling, power[channel].power_density, power[channel].fftSize/2+1, N_DEC_PER);
		
		change_bit_depth(power[channel].power_density, power[channel].power_density, power[channel].fftSize/2+1, N_DEC_PER, N_DEC_PSD);

		Psd(power[channel].power_density, power[channel].power_density, power[channel].fs, power[channel].fftSize/2+1);

		status = 1;
	}

	free(sig_periodogram);

	return status;
}

void PowerFeatures_GetFeat(my_int *features, uint8_t channel){
	my_int frequency_step = fx_divx(fx_itox(power[channel].fs, N_DEC_PSD), fx_itox(power[channel].fftSize, N_DEC_PSD), N_DEC_PSD);

	my_int denum = fx_mulx(fx_itox(power[channel].fftSize, N_DEC_TP), fx_itox(power[channel].batches_total, N_DEC_TP), N_DEC_TP);

	my_int p_tot = fx_divx(power[channel].total_power, denum, N_DEC_TP);
	change_bit_depth(&p_tot, &p_tot, 1, N_DEC_TP, N_DEC_PSD);	// change to N_DEC_PSD so it can be used in computations

	// //theta 4 a 8 Hz
	features[p_theta] = Bpower(power[channel].power_density, 4, 8, frequency_step);
	features[p_theta_rel] = fx_divx(features[p_theta], p_tot, N_DEC_PSD);

	// //alfa 8-13Hz
	features[p_alfa] = Bpower(power[channel].power_density, 8, 12, frequency_step);
	features[p_alfa_rel] = fx_divx(features[p_alfa], p_tot, N_DEC_PSD);

	// //beta 13-30Hz
	features[p_beta] = Bpower(power[channel].power_density, 12, 16, frequency_step);
	features[p_beta_rel] = fx_divx(features[p_beta], p_tot, N_DEC_PSD);

	features[p_alpha_beta] = fx_divx(features[p_alfa], features[p_beta], N_DEC_PSD);
	features[p_theta_alpha] = fx_divx(features[p_theta], features[p_alfa], N_DEC_PSD);

	memset(power[channel].power_density, 0, (power[channel].fftSize/2+1)* sizeof (my_int));
	power[channel].batches_exec=0;
	power[channel].total_power = 0.0;

}


void Periodogram(my_int *sig, my_int *periodogram, int numel_sig){
	my_int nyk;

	//hamming window convolution
	vect_mult(sig, hamming_win, sig, numel_sig, N_DEC_POW);

    my_int oR [WINDOW_LENGTH];
	my_int oI [WINDOW_LENGTH];
	
	init_fft();
	fft(sig, oR, oI);
	
    my_int out[WINDOW_LENGTH];	// output in interleaved way [real, imag, real, imag ...]

	// order the final vectors into one with only half the results (symmetry)
	for (size_t i = 0; i < WINDOW_LENGTH/2; i++)
	{
		out[2*i] = oR[i];
		out[(2*i)+1] = oI[i];
	}
	out[1] = oR[511];	// Nyquist frequency
	
	nyk = oR[513];	// take the nyquist freq
	out[1] = 0;	//keep the DC freq as Real number


	change_bit_depth(out, out, WINDOW_LENGTH, N_DEC_POW, N_DEC_CMPLX);

	//sig_power have the power per frequency bin (needs normalization)
	cmplx_mag_squared(out, periodogram, numel_sig/2, N_DEC_CMPLX);

	periodogram[numel_sig/2] = fx_mulx(nyk, nyk, N_DEC_POW);

	change_bit_depth(&periodogram[numel_sig/2], &periodogram[numel_sig/2], 1, N_DEC_POW, N_DEC_CMPLX);	// whole periodogram represented with N_DEC_CMPLX

	my_int scale = fx_ftox(WINDOW_SCALING_FACTOR, N_DEC_PER);
	change_bit_depth(periodogram, periodogram, numel_sig/2+1, N_DEC_CMPLX, N_DEC_PER);
	vect_scale(periodogram, scale, periodogram, numel_sig/2+1, N_DEC_PER);
    
	return;
}

void Psd(my_int *periodogram, my_int *power_density, int sampling_frequency, int numel_power){

	power_density[0] = fx_divx(periodogram[0], fx_itox(sampling_frequency, N_DEC_PSD), N_DEC_PSD);
	power_density[numel_power-1] = fx_divx(periodogram[numel_power-1], fx_itox(sampling_frequency, N_DEC_PSD), N_DEC_PSD);
	my_int scale = fx_divx(fx_itox(2, N_DEC_PSD), fx_itox(sampling_frequency, N_DEC_PSD), N_DEC_PSD);
	vect_scale(&periodogram[1], scale, &power_density[1], numel_power-2, N_DEC_PSD);	
}


my_int Bpower(my_int *power_density, my_int f1, my_int f2, my_int frequency_resolution){
	my_int power=0, in1, in2, in3, in4;
	unsigned short len, i=0;

	my_int idx1 = fx_divx(fx_itox(f1, N_DEC_PSD), frequency_resolution, N_DEC_PSD);
	my_int idx2 = fx_divx(fx_itox(f2, N_DEC_PSD), frequency_resolution, N_DEC_PSD);
	idx1 = fx_xtoi(idx1, N_DEC_PSD);	// need to convert them back to int because they are used as indexes
	idx2 = fx_xtoi(idx2, N_DEC_PSD);

	len = (idx2 - idx1 + 1) >> 2u; //loop unrolling;
	//								considering that the vector starts at 0, 1 should be add to account for this vector position

	while(len > 0u)
	{
		/* C = A * scale */
		/* Scale the input and then store the results in the destination buffer. */
		/* read input samples from source */
		in1 = power_density[idx1+i];
		in2 = power_density[idx1+i+1];

		/* multiply with scaling factor */
		in1 = fx_mulx(in1, frequency_resolution, N_DEC_PSD);

		/* read input sample from source */
		in3 = power_density[idx1+i+2];

		/* multiply with scaling factor */
		in2 = fx_mulx(in2, frequency_resolution, N_DEC_PSD);

		/* read input sample from source */
		in4 = power_density[idx1+i+3];

		/* multiply with scaling factor */
		in3 = fx_mulx(in3, frequency_resolution, N_DEC_PSD);
		in4 = fx_mulx(in4, frequency_resolution, N_DEC_PSD);

		/* store the result to destination */
		power = power + in1+ in2+ in3+ in4;

		/* update pointers to process next samples */
		i += 4u;

		/* Decrement the loop counter */
		len--;
	}

	len = (idx2-idx1+1) % 4; // reinitialize len for getting the rest of items

	/* If the blockSize is not a multiple of 4, compute any output samples here.
	 ** No loop unrolling is used. */
	while(len > 0u)
	{
		// power = power + power_density[idx1+i] * frequency_resolution;
		power = power + fx_mulx(power_density[idx1+i], frequency_resolution, N_DEC_PSD);

		/* Decrement the loop counter */
		i++;
		len--;
	}

	return power ;
}

void endPowerModule(){
	free(power);
    
	for(int i=0; i<N_CHANNEL_USED; i++){
		free(power[i].power_density);
	}
}
