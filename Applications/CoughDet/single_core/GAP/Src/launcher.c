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
#include <string.h>
#include <inttypes.h>

#include <launcher.h>

#include <feature_extraction.h>
#include <audio_features.h>
#include <imu_features.h>

#include <randomForest.h>

/* This is for testing more different widnows of data */
// #include <input_data/audio_input_55502.h>
// #include <input_data/imu_input_55502.h>

/* This is for testing just one window of data */
#include <audio_input_55502_w2.h>
#include <imu_input_55502_w2.h>
#include <bio_input_55502.h>

#include <pmsis.h>


// NOTE that if the types of features are changed then the classfication will fail since it
// was only trained for this set of features

/*
    Features selector vectors.
    Each array is a one-hot econding of the features to be extracted.
    1 --> extract the corresponding feature
    0 --> non extract the corresponding feature
*/
PI_L2 static const int8_t audio_features_selector[Number_AUDIO_Features] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
PI_L2 static const int8_t imu_features_selector[Number_IMU_Features] = {1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1};



void print_features(float *feats, int16_t len, int8_t *select);


void launch(){

    // These two arrays contain the indexes of the features that are going to be extracted
    int8_t *indexes_audio_f = (int8_t*)pvPortMalloc(N_AUDIO_FEAT_RF * sizeof(int8_t));
    int8_t *indexes_imu_f = (int8_t*)pvPortMalloc(N_IMU_FEAT_RF * sizeof(int8_t));
    int8_t idx = 0;
    for(int8_t i=0; i<Number_AUDIO_Features; i++){
        if(audio_features_selector[i] == 1){
            indexes_audio_f[idx] = i;
            idx++;
        }
    }
    idx = 0;
    for(int8_t i=0; i<Number_IMU_Features; i++){
        if(imu_features_selector[i] == 1){
            indexes_imu_f[idx] = i;
            idx++;
        }
    }

    ////    AUDIO FEATURES    ////
    // Array for containing the audio features values. The order is the same as of the
    // features families enum
    float  *audio_feature_array = (float*)pvPortMalloc(Number_AUDIO_Features * sizeof(float));
    memset(audio_feature_array, 0.0, Number_AUDIO_Features);


    ////    IMU FEATURES    ////
    float *imu_feature_array = (float*)pvPortMalloc(Number_IMU_Features * sizeof(float)); // To store all the possible IMU features
    memset(imu_feature_array, 0.0, Number_IMU_Features);


    int16_t n_runs = 0;
    int16_t audio_runs = AUDIO_LEN / AUDIO_STEP;
    int16_t imu_runs = IMU_LEN / IMU_STEP;
    if(audio_runs <= imu_runs)
        n_runs = audio_runs - N_OVERLAPS;
    else
        n_runs = imu_runs - N_OVERLAPS;

    /* Array to contain all the features that are going to be used by the RF */
    float *ensamble_feats = (float*)pvPortMalloc(TOT_FEATURES_RF * sizeof(float));
    float *probs = (float*)pvPortMalloc(2 * sizeof(float));
    float *model_out = (float*)pvPortMalloc(n_runs * sizeof(float));

    for(int16_t i=0; i<n_runs; i++){
        audio_features(audio_features_selector, &audio_in.air[i*AUDIO_STEP], WINDOW_SAMP_AUDIO, AUDIO_FS, audio_feature_array);
        imu_features(imu_features_selector, &imu_in[i*IMU_STEP], WINDOW_SAMP_IMU, imu_feature_array);

        // Save all the features in the same array
        for(int8_t j=0; j<N_AUDIO_FEAT_RF; j++){
            ensamble_feats[j] = audio_feature_array[indexes_audio_f[j]];
        }
        for(int8_t j=0; j<N_IMU_FEAT_RF; j++){
            ensamble_feats[j+N_AUDIO_FEAT_RF] = imu_feature_array[indexes_imu_f[j]];
        }
        ensamble_feats[38] = gender;
        ensamble_feats[39] = bmi;

        for(int8_t j=0; j<TOT_FEATURES_RF; j++){
            ensamble_feats[j] = (ensamble_feats[j] - feat_means[j]) / feat_stds[j];
        }

        // resset the score
        probs[0] = 0.0;
        probs[1] = 0.0;

        predict_c(ensamble_feats, probs);

        #ifdef PRINTING_ON
        printf("[%d]\t[%f\t%f]\n", i, probs[0], probs[1]);
        #endif

        model_out[i] = probs[1];

    }

    vPortFree(indexes_audio_f);
    vPortFree(indexes_imu_f);

    vPortFree(audio_feature_array);
    vPortFree(imu_feature_array);

    vPortFree(ensamble_feats);
    vPortFree(probs);
    vPortFree(model_out);

}
