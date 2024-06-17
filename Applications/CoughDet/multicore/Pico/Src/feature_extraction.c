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
#include <inttypes.h>

#include <time_domain_feat.h>
#include <feature_extraction.h>
#include <frequency_features.h>
#include <helpers.h>

#include <audio_features.h>
#include <imu_features.h>

// multicore custom and Pico library
#include "PICO/multicore.h"
#include <pico/multicore.h>

// Multicore helper functions
static void helper_kurt_multicore(void)
{
    compute_kurt_multicore(multicore_args, 1);
}

static void helper_skew_multicore(void)
{
    compute_skew_multicore(multicore_args, 1);
}

void fft_based_features(int8_t *features_selector, float *sig, int16_t len, int16_t fs, float *feats);
void periodogram_based_features(int8_t *features_selector, float *sig, int16_t len, int16_t fs, float *feats);
void mfcc_features(int8_t *features_selector, float *sig, int16_t len, float *feats);
void mean_based_features(int8_t *features_selector, float *sig, int16_t len, float *feats);

void imu_signal_features(int8_t *features_selector, float *sig, int16_t len, float *feats);
    

/* 
    Given the features selector vector and two indexes (start and end), it
    returns 1 if feature_selector has at least a 1 in the range specified 
    by the indexes, 0 otherwise
*/
int is_required(int8_t *features_selector, int8_t start_index, int8_t end_index)  {
    for(int8_t i=start_index; i<=end_index; i++)  {
        if(features_selector[i] == 1)  {
            return 1;   // It is sufficient to check if one is needed
        }
    }
    return 0;
}



/*
    Computes the required FFT-based features of the audio signal
*/
void fft_based_features(int8_t *features_selector, float *sig, int16_t len, int16_t fs, float *feats)  {
    int32_t size;
    // FFT-dependent features' indexes
    // 0  : 6  the singular ones

    if(is_required(features_selector, SPECTRAL_DECREASE, SPECTRAL_SKEW))  {

        int16_t fft_size = (len / 2) + 1;
        float *magnitudes = (float*) malloc(fft_size * sizeof(float));
        float *frequencies = (float*) malloc(fft_size *sizeof(float));
        float sum_mags = 0.0;

        compute_rfft(sig, len, fs, magnitudes, frequencies, &sum_mags);


        if(features_selector[SPECTRAL_DECREASE])  {
            // compute SPEC_DECR
            float spectral_decrease = compute_spec_decrease(magnitudes, frequencies, (len/2)+1, sum_mags);
            feats[SPECTRAL_DECREASE] = spectral_decrease;
        }

        if(features_selector[SPECTRAL_SLOPE])  {
            // compute SPEC_SLOPE
            float spectral_slope = compute_spectral_slope(magnitudes, frequencies, (len/2)+1, sum_mags);
            feats[SPECTRAL_SLOPE] = spectral_slope;
        }
        
        if(features_selector[SPECTRAL_ROLLOFF])  {
            // compute SPEC_ROLL
            float spectral_rolloff = compute_rolloff(magnitudes, frequencies, (len/2)+1, sum_mags);
            feats[SPECTRAL_ROLLOFF] = spectral_rolloff;
        }
        
        if(is_required(features_selector, SPECTRAL_CENTROID, SPECTRAL_SKEW))  {
            // compute SPEC_CENTROID
            float spectral_cetroid = compute_centroid(magnitudes, frequencies, (len/2)+1, sum_mags);

            if(features_selector[SPECTRAL_CENTROID])  {
                // append SPEC_CENTROID
                feats[SPECTRAL_CENTROID] = spectral_cetroid;
            }

            if(is_required(features_selector, SPECTRAL_SPREAD, SPECTRAL_SKEW))  {
                // compute SPEC_SPREAD
                float spectral_spread = compute_spread(magnitudes, frequencies, (len/2)+1, sum_mags, spectral_cetroid);

                if(features_selector[SPECTRAL_SPREAD])  {
                    // append SPEC_SPREAD
                    feats[SPECTRAL_SPREAD] = spectral_spread;
                }

                if(features_selector[SPECTRAL_KURTOSIS])  {
                    size = (len/2)+1;

                    multicore_args[0] = (int32_t*) magnitudes;
                    multicore_args[1] = (int32_t*) frequencies;
                    multicore_args[2] = (int32_t*) &size;
                    multicore_args[3] = (int32_t*) &sum_mags;
                    multicore_args[4] = (int32_t*) &spectral_cetroid;
                    multicore_args[5] = (int32_t*) &spectral_spread;

                    // compute SPEC_KURT - both cores working
                    multicore_launch_core1(helper_kurt_multicore);
                    compute_kurt_multicore(multicore_args, 0);
                    multicore_reset_core1();
                    // return value
                    feats[SPECTRAL_KURTOSIS] = *multicore_args[9];
                }

                if(features_selector[SPECTRAL_SKEW]){
                    // Prepare arguments for multicore
                    float skew;
                    size = (len/2)+1;
                    multicore_args[0] = (int32_t*) magnitudes;
                    multicore_args[1] = (int32_t*) frequencies;
                    multicore_args[2] = (int32_t*) &size;
                    multicore_args[3] = (int32_t*) &sum_mags;
                    multicore_args[4] = (int32_t*) &spectral_cetroid;
                    multicore_args[5] = (int32_t*) &spectral_spread;
                    multicore_args[9] = (int32_t*) &skew;

                    // compute SPEC_KURT - both cores working
                    multicore_launch_core1(helper_skew_multicore);
                    compute_skew_multicore(multicore_args, 0);
                    multicore_reset_core1();

                    feats[SPECTRAL_SKEW] = *multicore_args[9];
                }
            }
        }

        free(magnitudes);
        free(frequencies);
    }
}



/*
    Computes the required periodogram-based features of the audio signal
*/
void periodogram_based_features(int8_t *features_selector, float *sig, int16_t len, int16_t fs, float *feats)  {

    // Periodogram dependent features' indexes
    // 7  : 9 for the singular ones
    // 10 : 12 for the PSD ones
    if(is_required(features_selector, SPECTRAL_FLATNESS, POWER_SPECTRAL_DENSITY + N_PSD - 1))  {
        // compute Periodogram
        int16_t psd_size = (NPERSEG / 2) + 1;
        float *psd = (float*)malloc(psd_size * sizeof(float));
        float *freqs = (float*)malloc(psd_size * sizeof(float));
        compute_periodogram(sig, len, fs, psd, freqs);

        if(features_selector[SPECTRAL_FLATNESS])  {
           // compute SPEC_FLAT
           float spectral_flatness = compute_flatness(psd, psd_size);
           feats[SPECTRAL_FLATNESS] = spectral_flatness;
        }

        if(features_selector[SPECTRAL_STD])  {
            // compute SPEC_STD
            float spectral_std = compute_std(psd, psd_size);
            feats[SPECTRAL_STD] = spectral_std;
        }

        if(features_selector[DOMINANT_FREQUENCY])  {
            // compute DOM_FREQ
            float dominant_freq = get_domiant_freq(psd, freqs, psd_size);
            feats[DOMINANT_FREQUENCY] = dominant_freq;
        }

        if(is_required(features_selector, POWER_SPECTRAL_DENSITY, POWER_SPECTRAL_DENSITY + N_PSD - 1))  {
            // compute PSD
            float *band_powers = (float*)malloc(N_PSD * sizeof(float));
            normalized_bandpowers(psd, freqs, psd_size, &features_selector[POWER_SPECTRAL_DENSITY], band_powers);
            for(int8_t i=0; i<N_PSD; i++)  {
                feats[POWER_SPECTRAL_DENSITY + i] = band_powers[i];
            }

            free(band_powers);
        }

        free(psd);
        free(freqs);
    }
}



/*
    Computes the required MFCC features of the audio signal
*/
void mfcc_features(int8_t *features_selector, float *sig, int16_t len, float *feats)  {

    // 13 : 38 for the MFCCs features
    if(is_required(features_selector, MEL_FREQUENCY_CEPSTRAL_COEFFICIENT, MEL_FREQUENCY_CEPSTRAL_COEFFICIENT + (N_MFCC * 2) - 1))  {
        // compute MFCCs

        float *mean_mfcc = (float*)malloc(N_MFCC * sizeof(float));
        float *std_mfcc = (float*)malloc(N_MFCC * sizeof(float));
        get_mfcc_features(sig, len, mean_mfcc, std_mfcc);

        // stores first the mean and then the std, one after the other
        for(int16_t i=0; i<N_MFCC; i++)  {
            feats[MEL_FREQUENCY_CEPSTRAL_COEFFICIENT + i] = mean_mfcc[i];
            feats[MEL_FREQUENCY_CEPSTRAL_COEFFICIENT + N_MFCC + i] = std_mfcc[i];
        }


        free(mean_mfcc);
        free(std_mfcc);
    }    
}



/*
    Computes the required mean-based features of the audio signal
*/
void mean_based_features(int8_t *features_selector, float *sig, int16_t len, float *feats)  {

    // 39 : 41 for the singular ones
    if(is_required(features_selector, ROOT_MEANS_SQUARED, CREST_FACTOR))  {
        // compute mean

        float *zero_mean = (float *) malloc(len * sizeof(float));  // to store the signal after subtracting the mean
        sub_mean(sig, zero_mean, len);


        if(features_selector[ZERO_CROSSING_RATE])  {
            // compute ZCR
            float zcr = compute_zrc(zero_mean, len);
            feats[ZERO_CROSSING_RATE] = zcr;
        }

        if(features_selector[ROOT_MEANS_SQUARED] || features_selector[CREST_FACTOR])  {
            // compute RMS
            float rms = get_rms(zero_mean, len);

            if(features_selector[ROOT_MEANS_SQUARED])  {
                // append RMS
                feats[ROOT_MEANS_SQUARED] = rms;
            }

            if(features_selector[CREST_FACTOR])  {
                // compute CREST
                float peak = get_max(zero_mean, len);
                float crest_factor = peak / rms;
                feats[CREST_FACTOR] = crest_factor;
            }
        }

        free(zero_mean);
    }
}



/*
    Computes the required EEPD features of the audio signal
*/
void eepd_features(int8_t *features_selector, float *sig, int16_t len, int16_t fs, float *feats)  {

    // 42 : 61 for the singular ones
    if(is_required(features_selector, ENERGY_ENVELOPE_PEAK_DETECT, Number_AUDIO_Features))  {

        int16_t *eepds = (int16_t*)malloc(N_EEPD * sizeof(int16_t));

        // compute EEPD
        eepd(sig, len, fs, eepds);

        for(int16_t i=0; i<N_EEPD; i++)  {
            feats[ENERGY_ENVELOPE_PEAK_DETECT + i] = eepds[i];
        }

        free(eepds);
    }
}



/*
    Computes features from the AUDIO signal passed as parameter.
    The features will be stored in the feats array.
    The features to be extracted are specified by the features_selector 
    array with a one-hot encoding:
    0: DO NOT extract the corresponding feature
    1: DO extract the corresponding feature
*/
void audio_features(int8_t *features_selector, float *sig, int16_t len, int16_t fs, float *feats)  {

    /* FFT based features */
    fft_based_features(features_selector, sig, len, fs, feats);

    /* Periodogram-based features */
    periodogram_based_features(features_selector, sig, len, fs, feats);

    /* MFCCs features */
    mfcc_features(features_selector, sig, len, feats);

    /* Mean-based features */
    mean_based_features(features_selector, sig, len, feats);

    /* EEPD features */
    eepd_features(features_selector, sig, len, fs, feats);

}


/*
    Process one IMU signal, checks which features are required and computes them
*/
void imu_signal_features(int8_t *features_selector, float *sig, int16_t len, float *feats)  {

    if(features_selector[LINE_LENGTH])  {
        // LINE_LENGTH is required
        float line_l = get_line_length(sig, len);
        feats[LINE_LENGTH] = line_l;
    }
    if(features_selector[ZERO_CROSSING_RATE_IMU])  {
        // ZERO_CROSSING_RATE_IMU is required
        float zcr_imu = compute_zrc(sig, len);
        feats[ZERO_CROSSING_RATE_IMU] = zcr_imu;
    }
    if(features_selector[KURTOSIS])  {
        // KURTOSIS kurt is required
        float kurt_imu = get_kurtosis(sig, len);
        feats[KURTOSIS] = kurt_imu;
    }
    if(features_selector[ROOT_MEANS_SQUARED_IMU] || features_selector[CREST_FACTOR_IMU])  {
        // ROOT MEANS SQUARED is required
        float rms_imu = get_rms(sig, len);
        if(features_selector[ROOT_MEANS_SQUARED_IMU])  {
            feats[ROOT_MEANS_SQUARED_IMU] = rms_imu;
        }
        if(features_selector[CREST_FACTOR_IMU])  {
            // CREST_FACTOR_IMU is required
            float cf_imu = get_max(sig, len);
            cf_imu = cf_imu / rms_imu;
            feats[CREST_FACTOR_IMU] = cf_imu;

        }
    }
}


/*
    This function triggers the feature extraction process for a specific IMU feature family.
    First it checks the the features has to be computed, by means of the features_selector array.
    Then it retrieves the proper data and it calls the feature extraction function.

    The discrimination between different IMU signal here is done through the use of the two
    input parameters "signal_idx" and "sig_feat_idx".

    signal_idx   : index of the IMU signal 
    sig_feat_idx : starting index of the features for the IMU signal inside the features_selector vector
*/
void compute_imu_family(int8_t *features_selector, float signal[][Num_IMU_signals], int16_t len, int8_t signal_idx, int8_t sig_feat_idx, float *feats)  {
    
    // This array is filled with the proper signal samples 
    float *signal_samples = (float*)malloc(len * sizeof(float));


    if(is_required(features_selector, sig_feat_idx, sig_feat_idx+Num_imu_feat_families-1))  {
        // the specific signal is required

        // take only samples for the required signal
        for(int16_t i=0; i<len; i++)  {
            signal_samples[i] = signal[i][signal_idx];
        }

        imu_signal_features(&features_selector[sig_feat_idx], signal_samples, len, &feats[sig_feat_idx]);
    }

    free(signal_samples);
}


/*
    Computes features from the IMU signal passed as parameter.
    The features will be stored in the feats array.
    The features to be extracted are specified by the features_selector 
    array with a one-hot encoding:
    0: DO NOT extract the corresponding feature
    1: DO extract the corresponding feature
*/
void imu_features(int8_t *features_selector, float sig[][Num_IMU_signals], int16_t len, float *feats)  {

    // Here len is the IMU_DIM_1 macro in the hardcoded samples

    // ACCEL_X  
    compute_imu_family(features_selector, sig, len, ACCELEROMETER_X, ACCEL_X_FEAT, feats);

    // ACCEL_Y  
    compute_imu_family(features_selector, sig, len, ACCELEROMETER_Y, ACCEL_Y_FEAT, feats);

    // ACCEL_Z  
    compute_imu_family(features_selector, sig, len, ACCELEROMETER_Z, ACCEL_Z_FEAT, feats);



    // GYRO_Y  
    compute_imu_family(features_selector, sig, len, GYROSCOPE_Y, GYRO_Y_FEAT, feats);

    // GYRO_P  
    compute_imu_family(features_selector, sig, len, GYROSCOPE_P, GYRO_P_FEAT, feats);

    // GYRO_R  
    compute_imu_family(features_selector, sig, len, GYROSCOPE_R, GYRO_R_FEAT, feats);

    // Array to store the combination of signals (ACCEL and then GYRO)
    float *combo_signal = (float*)malloc(len * sizeof(float));

    // combine the signals by taking the L2 norm of them.
    // For every istant, the L2 norm is computed on the samples of the signals
    for(int16_t i=0; i<len; i++)  {
        combo_signal[i] = L2_norm(&sig[i], 3);
    }
    imu_signal_features(&features_selector[ACCEL_COMBO], combo_signal, len, &feats[ACCEL_COMBO]);

    for(int16_t i=0; i<len; i++)  {
        combo_signal[i] = L2_norm(&sig[i][3], 3);
    }
    imu_signal_features(&features_selector[GYRO_COMBO], combo_signal, len, &feats[GYRO_COMBO]);

    free(combo_signal);

}
