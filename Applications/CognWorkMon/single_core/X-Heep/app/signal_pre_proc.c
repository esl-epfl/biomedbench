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

#include <stdint.h>
#include <signal_pre_proc.h>
#include <utils_functions.h>


my_int blink_filter_buff[BLINK_N_POINTS_USED + SGFilter_NCOEF - 1];
my_int pks[MAX_N_PEAKS_EXPECTED];
uint16_t locks[MAX_N_PEAKS_EXPECTED];

// includes all the preprocessing (but not filtering yet)
uint8_t PreProc_BlinkRemoval(my_int *signal, uint16_t npeaks)
{
	uint8_t status = 1;
	uint16_t idx_b;

	my_int new_sgFilt_b[SGFilter_NCOEF];

	convert_to_x(sgFilt_b, new_sgFilt_b, SGFilter_NCOEF, N_DEC_BIQ);

	while (npeaks > 0) // let's apply SGFilter to remove blinks for each found peak
	{
        idx_b = locks[npeaks - 1] - BLINK_N_POINTS_BEFORE_PEAK;
		if (idx_b > ((SGFilter_NCOEF - 1) / 2) && idx_b < 1024 - (BLINK_N_POINTS_USED + SGFilter_NCOEF - 1))
		{
			vect_conv(&signal[idx_b], BLINK_N_POINTS_USED, new_sgFilt_b, SGFilter_NCOEF, blink_filter_buff, N_DEC_BIQ);

			idx_b = idx_b - ((SGFilter_NCOEF - 1) / 2);
			vect_sub(&signal[idx_b], blink_filter_buff, &signal[idx_b], BLINK_N_POINTS_USED + SGFilter_NCOEF - 1);
		}
		npeaks--;
	}
	return status;
}

// return the number of peaks found
// place the peak values in 'pks' input vector and location in 'locks'
uint8_t PreProc_FindPeaks(my_int *data, uint16_t len)
{
	my_int aux, thrs_h, thrs_l, *d, *p;
	uint8_t status = 0, peak_detc_state;
	uint16_t idx_h, idx_l, counter, *l;

	//standard deviation of RelativeEnergy coefficients to be used to calculate the threshold
	vect_std(data, len, &aux, N_DEC_BLINK);

	thrs_h = fx_mulx(fx_itox(4, N_DEC_BLINK), aux, N_DEC_BLINK);	// thresholds for detecting peaks 
	thrs_l = fx_mulx(fx_itox(3, N_DEC_BLINK), aux, N_DEC_BLINK);

	counter = len;

	peak_detc_state = 0;
	d = data;
	p = pks;
	l = locks;
	while (counter > 0)
	{
		// peak detection state-machine
		switch (peak_detc_state)
		{
		case 0: // finds first point above higher threshold
			if (*d++ > thrs_h)
			{
				peak_detc_state = 1;
				idx_h = len - counter;
			}
			break;
		case 1: // finds first point under lower threshold, delimiting a search zone for the peak
			if (*d++ < thrs_l)
			{
				idx_l = len - counter;

				if (idx_l - idx_h > PEAK_WIDTH_MINIMUM) // checks for spurious peaks
					peak_detc_state = 2;
				else
					peak_detc_state = 0;
			}
			break;
		case 2: // find the maximum value in the delimited zone and return its location
			vect_max(&data[idx_h], idx_l - idx_h + 1, p++, l);
			*l++ += idx_h;
			status++; // at least one peak has been detected
			peak_detc_state = 0;
			break;
		}
		counter--;
	}

	return status;
}
