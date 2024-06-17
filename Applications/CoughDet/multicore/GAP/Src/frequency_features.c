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


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>

#include <feature_extraction.h>
#include <frequency_features.h>
#include <welch_psd.h>
#include <mfcc_module.h>
#include <mel_basis.h>
#include <helpers.h>

#include <audio_features.h>

#include <kiss_fftr.h>

#include <errno.h>

#include <profile.h>
#include <pmsis.h>

// CLUSTER
PI_L1 float sum_cl = 0;
PI_L1 float ret = 0;

// CLUSTER END


// Helper function for the RFFT
void _rfft(float *sig, int16_t len, float *real, float *imag);


/*
    Computes the Real FFT of a time signal sig.
    It stores the magnitudes, the frequencies and the sum of all the magnitudes inside
    *mags, *freqs abd *sum_mags, respectively.
    Note that also the sampling_frequency (fs) and the length (len) of the signal are required as inputs
*/
void compute_rfft(float *sig, int16_t len, int16_t fs, float *mags, float *freqs, float *sum_mags)  {

    float *re = (float*)malloc(len * sizeof(float));
    float *im = (float*)malloc(len * sizeof(float));

    _rfft(sig, len, re, im);

    // Compute the magnitude of each FFT output
    for(int16_t i=0; i<(len/2)+1 ; i++)  {
        mags[i] = sqrtf((re[i] * re[i]) + (im[i] * im[i]));
        *sum_mags += mags[i];
    }

    // Get the frequency bins (only the positive one becaus it's a real FFT)
    for(int16_t i=0; i<(len/2)+1; i++)  {
        freqs[i] = (float)(i * fs) / len;
    }

    free(re);
    free(im);
}


/*
    Helper function not callable externally.
    This just computes the RFFT of the input signal and 
    stores the Real and Imaginary parts in two separate arrays
*/
void _rfft(float *sig, int16_t len, float *real, float *imag)  {

    kiss_fftr_cfg cfg = kiss_fftr_alloc(len, 0, 0, 0);
    kiss_fft_cpx *cx_out = (kiss_fft_cpx *) malloc(len * sizeof(kiss_fft_cpx));

    kiss_fftr(cfg, sig, cx_out);

    for(int16_t i=0; i<len; i++)  {
        real[i] = cx_out[i].r;
        imag[i] = cx_out[i].i;
    }

    free(cfg);
    free(cx_out);
}


/*
    Computes the periodogram of a signal using the Welch's method
*/
void compute_periodogram(float *sig, int16_t len, int16_t fs, float *psd, float *freqs)  {

    float freq_step = ((float)fs / 2) / ( (float)NPERSEG / 2);  // the frequency step for eah bin 
    float *win = (float*)malloc(NPERSEG * sizeof(float));   // to keep the data of the current processed window
    float *cumul_sums = (float*)malloc(NPERSEG * sizeof(float));  // To store the cumulative sum of the FFT of each frequency bin
    memset(cumul_sums, 0, NPERSEG * sizeof(float));

    // To store the real and imaginary parts of each FFT
    float *re = (float*)malloc(NPERSEG * sizeof(float));
    float *im = (float*)malloc(NPERSEG * sizeof(float));

    // To store the magnitudes squared after the FFT
    float *mags_squared = (float*)malloc(((NPERSEG/2)+1) * sizeof(float));


    float mean = 0.0;
    float scale = 0.0;
    float sum = 0.0;

    for(int16_t i=0; i<NPERSEG; i++)  {
        sum += hann_window[i] * hann_window[i];
    }

    scale = 1 / (fs * sum); 

    // start and end indexes of the current processed window
    int16_t start = 0;
    int16_t end = start + NPERSEG;

    int16_t steps = (len - NOVERLAP) / (NPERSEG - NOVERLAP);    // Number or windows that will be processed
    for(int16_t i=0; i<steps; i++)  {

        vect_copy(sig, start, NPERSEG, win);    // copies the current window from the signal

        // subtract the mean
        mean = vect_mean(win, NPERSEG);
        sub_constant(win, NPERSEG, mean, win);

        // Apply the window function
        for(int16_t i=0; i<NPERSEG; i++)  {
            win[i] *= hann_window[i];
        }  


        _rfft(win, NPERSEG, re, im);    // Actual Real FFT computation

        for(int16_t i=0; i<(NPERSEG/2)+1; i++)  {
            mags_squared[i] = (re[i] * re[i]) + (im[i] * im[i]);
            mags_squared[i] *= scale;

            if(i != 0 && i != (NPERSEG/2))  {
                mags_squared[i] *= 2;       // Multiply by 2, apart from DC frequency (first element) and last element
            }
            cumul_sums[i] += mags_squared[i];   // Update the cumulative sum (element-wise across FFT result of different windows)
        }

        start = end - NOVERLAP;
        end = start + NPERSEG;
    }

    // Compute the result psd as the avarage of each FFT result and compute the frequencies
    for(int16_t i=0; i<(NPERSEG/2)+1; i++)  {
        psd[i] = cumul_sums[i] / steps;
        freqs[i] = freq_step * i;
    }

    free(win);
    free(cumul_sums);
    free(re);
    free(im);
    free(mags_squared);
}


/*
    Returns the spectral decrease computed from the magnitudes and the frequencies of
    the spectrum of a signal
*/
float compute_spec_decrease(float* mags, float* freqs, int16_t len, float sum_mags)  {

    float sum = 0.0;
    float dc_mag = mags[0];

    for(int16_t i=0; i<len; i++)  {
        sum += (mags[i] - dc_mag) / (i + 1);
    }

    return sum / sum_mags;
}


/*
    Returns the spectral slope computed from the magnitudes and the frequencies of
    the spectrum of a signal
*/
float compute_spectral_slope(float *mags, float *freqs, int16_t len, float sum_mags)  {

    float mean_mag = sum_mags / len;
    float mean_freq = 0.0;

    mean_freq = vect_mean(freqs, len);

    // Numerator and denominator for the final slope computation
    float num = 0.0;
    float den = 0.0;

    for(int16_t i=0; i<len; i++)  {
        num += (freqs[i] - mean_freq) * (mags[i] - mean_mag);
        den += (freqs[i] - mean_freq) * (freqs[i] - mean_freq); 
    }

    return num / den;
}


/*
    Returns the spectral roll off computed from the magnitudes and the frequencies of
    the spectrum of a signal
*/
float compute_rolloff(float *mags, float *freqs, int16_t len, float sum_mags)  {

    float rolloff_energy = 0.95 * sum_mags;
    float sum = 0.0;
    float rolloff = -1.0;   // Error value

    for(int16_t i=0; i<len; i++)  {
        sum += mags[i];

        if(sum >= rolloff_energy)
            return freqs[i];
    }

    return rolloff;
}


/*
    Returns the spectral centroid computed from the magnitudes and the frequencies of
    the spectrum of a signal
*/
float compute_centroid(float *mags, float *freqs, int16_t len, float sum_mags)  {

    float sum = 0.0;

    for(int16_t i=0; i<len; i++)  {
        sum += freqs[i] * mags[i];
    }

    return sum / sum_mags;
}


/*
    Returns the spectral spread computed from the magnitudes and the frequencies of
    the spectrum of a signal.
    Note that this also requires the spectral centroid as an input
*/
float compute_spread(float *mags, float *freqs, int16_t len, float sum_mags, float centroid)  {

    float sum = 0.0;

    for(int16_t i=0; i<len; i++)  {
        sum += (freqs[i] - centroid) * (freqs[i] - centroid) * mags[i];
    }

    return sqrtf(sum / sum_mags);
}


/*
    Returns the spectral kurtosis computed from the magnitudes and the frequencies of
    the spectrum of a signal.
    Note that this also requires the spectral centroid and the spectral spread as inputs
*/
float compute_kurt(float *mags, float *freqs, int16_t len, float sum_mags, float centroid, float spread)  {

    float spread_4 = spread * spread * spread * spread; // spread^4
    float sum = 0.0;

    for(int16_t i=0; i<len; i++){
        register float tmp = (freqs[i] - centroid) * (freqs[i] - centroid);
        sum += tmp * tmp * mags[i];
    }

    return sum / (spread_4 * sum_mags);
}

// parallel version
void compute_kurt_cl(int32_t **arg)  {

    sum_cl = 0.0;

    pi_cl_team_barrier();

    float *mags = (float *)arg[0];
    float *freqs = (float *)arg[1];
    int16_t len = (int16_t)*arg[2];
    float *sum_mags = (float *)arg[3];
    float centroid = *(float *)arg[4];
    float spread = *(float *)arg[5];

    int id = pi_core_id();

    register int chunk = len / NUM_CORES;
    register int start_index = id * chunk;
    if (id == NUM_CORES - 1)
	    chunk += len % NUM_CORES;

    float spread_4 = spread * spread * spread * spread; // spread^4
    float sum_part = 0.0;



    for(int16_t i=start_index; i<(start_index+chunk); i++)  {
        register float tmp = (freqs[i] - centroid) * (freqs[i] - centroid);
        sum_part += tmp * tmp * mags[i];
    }

    pi_cl_team_critical_enter();
    sum_cl += sum_part;
    pi_cl_team_critical_exit();

    pi_cl_team_barrier();

    if (id == 0)
    {
        ret = sum_cl / (spread_4 * (*sum_mags));
        arg[9] = &ret;
    }

}

/*
    Returns the spectral skewness computed from the magnitudes and the frequencies of
    the spectrum of a signal.
    Note that this also requires the spectral centroid and the spectral spread as inputs
*/
float compute_skew(float *mags, float *freqs, int16_t len, float sum_mags, float centroid, float spread)  {

    float spread_3 = spread * spread * spread;
    float sum = 0.0;

    for(int16_t i=0; i<len; i++){
        register float tmp = (freqs[i] - centroid) * (freqs[i] - centroid);
        sum += tmp * (freqs[i] - centroid) * mags[i];
    }

    return sum / (spread_3 * sum_mags);
}

void compute_skew_cl(int32_t **arg)
{
    sum_cl = 0.0;

    pi_cl_team_barrier();

    // Retrieves the parameters
    float *mags = (float *) arg[0];
    float *freqs = (float *) arg[1];
    int16_t len = (int16_t) *arg[2];
    float *sum_mags = (float *) arg[3];
    float centroid = *(float *) arg[4];
    float spread = *(float *) arg[5];

    int id = pi_core_id();

    // static scheduling
    register int chunk = len / NUM_CORES;
    register int start_index = id * chunk;
    if (id == NUM_CORES - 1)
    {
        chunk += len % NUM_CORES;
    }

    float spread_3 = spread * spread * spread;
    float sum_part = 0.0;
    
    for(int16_t i=start_index; i<(start_index+chunk); i++)  {
        register float tmp = (freqs[i] - centroid) * (freqs[i] - centroid);
        sum_part += tmp * (freqs[i] - centroid) * mags[i];
    }
    
    // Update the global summation
    pi_cl_team_critical_enter();
    sum_cl += sum_part;
    pi_cl_team_critical_exit();


    pi_cl_team_barrier();

    if (id == 0)
    {
        ret = sum_cl / (spread_3 * (*sum_mags));
        arg[9] = &ret;
    }
}


/*
    Returns the spectral flatness of the gives signal
*/
float compute_flatness(float *x, int16_t len)  {

    float gmean = 0.0;  // geometric
    float amean = 0.0;  // arithmetic

    float sum_logs = 0.0;
    for(int16_t i=0; i<len; i++)  {
        sum_logs += logf(x[i]);
    }
    sum_logs = sum_logs / len;

    gmean = exp(sum_logs);
    amean = vect_mean(x, len);
    return gmean / amean;
}


/*
    Returns the standard deviation of the given signal
*/
float compute_std(float *x, int16_t len)  {

    return vect_std(x, len);

}


/*
    Returns the frequency at which the maximum psd is found
*/
float get_domiant_freq(float *psd, float *freqs, int16_t len)  {
    
    return freqs[vect_max_index(psd, len)];
}


/*
    Computes the normalized power of each band.
    It requires the psd, the frequencies and also the psd_selector, which is an array
    of 1 or 0. 1 indicates that a specific band has to be computed. The bands
    are defined by the "psd_bands" structure
*/
void normalized_bandpowers(float *psd, float *freqs, int16_t len, int8_t *psd_selector, float *band_powers)  {

    float dx_freq = freqs[1] - freqs[0];
    float total_power = simpson(psd, len, dx_freq);

    int16_t start_ind_freq = 0; // index of the first frequency inside the band
    int16_t n_bins = 0;         // number of frequency bins inside the band
    int8_t start_found = 0;     // 1 if the start frequency was found, useful to minimize the if-statements

    float band_power = 0.0;

    //check which PSD bands are needed
    for(int16_t i=0; i<N_PSD; i++)  {
        if(psd_selector[i] == 1)  {
            start_found = 0;
            start_ind_freq = 0;
            n_bins = 0;
            for(int16_t j=0; j<len; j++)  {
                if(!start_found && freqs[j] >= psd_bands[i].start)  {
                    start_ind_freq = j;
                    start_found = 1;
                }
                if(start_found && freqs[j] <= psd_bands[i].end)  {
                    n_bins++;
                }
            }

            band_power = simpson(&psd[start_ind_freq], n_bins, dx_freq);
            band_powers[i] = band_power / total_power;
        }
    }
}

/*
    Computes the MFCC coefficients of the given signal x
    This function uses the STFT technique, therefore the MFCC are computed
    for every frame in which the RFFT is computed.
    Basically the output will be a matrix having, on i-th row, the i-th coefficient
    for each signal frame.
    The matrix is stored in a 1-Dimentional array, storing each row one after the other:

    coeffs = [... ROW 0 ... | ... ROW 1 ... | ...]
*/
void mfcc_computation(float *x, int16_t len, int16_t n_frames, float *coeffs)  {

    float *db_power = (float*)malloc((MEL_ROWS * n_frames) * sizeof(float));
    memset(db_power, 0.0, (MEL_ROWS * n_frames)*sizeof(float));

    // Mel spectrogram
    #ifdef PROFILE_MODULES
    profile_start();
    #endif
    mel_spectrogram(x, len, n_frames, db_power);
    #ifdef PROFILE_MODULES
    profile_stop();
    #endif

    // Convert the power in dB
    power_to_dB(db_power, (MEL_ROWS * n_frames), db_power);

    // Apply the DCT
    dct_matrix(db_power, MEL_ROWS, n_frames, db_power);

    for(int16_t i=0; i<N_MFCC; i++)  {
        for(int16_t j=0; j<n_frames; j++)  {
            coeffs[(i*n_frames) + j] = db_power[(i * n_frames) + j];
        }
    }

    free(db_power);
}


void get_mfcc_features(float *x, int16_t len, float *mean_mfcc, float *std_mfcc)  {

    int16_t padded_len = (2 * PAD_LEN) + len;                   // lenght of the 0-padded signal
    int16_t n_frames = ((padded_len - N_FFT) / HOP_LEN) + 1;    // number of frames for the stft

    float *coeffs = (float*)malloc((N_MFCC*n_frames) * sizeof(float));
    mfcc_computation(x, len, n_frames, coeffs);


    for(int16_t i=0; i<N_MFCC; i++)  {
        mean_mfcc[i] = vect_mean(&coeffs[i*n_frames], n_frames);
        std_mfcc[i] = vect_std(&coeffs[i*n_frames], n_frames);
    }

    free(coeffs);

}
