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



#ifndef INC_LAUNCHER_H_
#define INC_LAUNCHER_H_

#include <inttypes.h>

#include <audio_features.h>
#include <imu_features.h>

// Enable printing of results
//#define PRINTING_ON

#define OVERLAP             80    // Percentage of overlap in one window

#define WINDOW_LEN          0.3  // seconds of a window

#define WINDOW_SAMP_AUDIO   (int16_t)(WINDOW_LEN * AUDIO_FS) // audio samples in a window
#define OVERLAP_SAMP        (int16_t)(WINDOW_SAMP_AUDIO * OVERLAP / 100.0)
#define AUDIO_STEP          (int16_t)(WINDOW_SAMP_AUDIO * (1.0 - (OVERLAP / 100.0)) + 1)
#define N_OVERLAPS          (int16_t)(WINDOW_SAMP_AUDIO / AUDIO_STEP - 1)

#define WINDOW_SAMP_IMU     (int16_t)(WINDOW_LEN * IMU_FS)
#define IMU_STEP            (int16_t)(WINDOW_SAMP_IMU * (1.0 - (OVERLAP / 100.0)) + 1)




// NOTE that if the types of features are changed then the classfication will fail since it
// was only trained for this set of features

/*
    Features selector vectors.
    Each array is a one-hot econding of the features to be extracted.
    1 --> extract the corresponding feature
    0 --> non extract the corresponding feature
*/
static int8_t audio_features_selector[Number_AUDIO_Features] = {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int8_t imu_features_selector[Number_IMU_Features] = {1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1};

void launch();

#endif /* INC_LAUNCHER_H_ */
