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


#ifndef _IMU_FEATURES_H_
#define _IMU_FEATURES_H_

#include <inttypes.h>

/* Defines all the parameters of IMU features extraction */

/*
    Different IMU signals that are used for the feature extraction
*/
enum imu_signals {
    ACCELEROMETER_X,
    ACCELEROMETER_Y,
    ACCELEROMETER_Z,
    GYROSCOPE_Y,
    GYROSCOPE_P,
    GYROSCOPE_R,
    Num_IMU_signals
};


/*
    Featues types that can be extracted from each IMU signal
*/
enum imu_features_families {
    LINE_LENGTH,
    ZERO_CROSSING_RATE_IMU,
    KURTOSIS,
    ROOT_MEANS_SQUARED_IMU,
    CREST_FACTOR_IMU,
    Num_imu_feat_families   // total number of different feature types we have for IMU
};


/*
    Indexes all the possibile IMU features that we can have.
    Basically, for every IMU signal we can have each one of
    the features families.
    This enum is primarly used to have base indexes for the features_selector array
*/
enum imu_signal_features {
    ACCEL_X_FEAT,
    ACCEL_Y_FEAT = ACCEL_X_FEAT + Num_imu_feat_families,
    ACCEL_Z_FEAT = ACCEL_Y_FEAT + Num_imu_feat_families,
    
    GYRO_Y_FEAT = ACCEL_Z_FEAT + Num_imu_feat_families,
    GYRO_P_FEAT = GYRO_Y_FEAT + Num_imu_feat_families,
    GYRO_R_FEAT = GYRO_P_FEAT + Num_imu_feat_families,

    ACCEL_COMBO = GYRO_R_FEAT + Num_imu_feat_families,
    GYRO_COMBO = ACCEL_COMBO + Num_imu_feat_families,

    Number_IMU_Features = GYRO_COMBO + Num_imu_feat_families
};


#endif