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



#ifndef _IMU_INPUT_55502_W2_H_
#define _IMU_INPUT_55502_W2_H_

#include "pmsis.h"

/* Sampling frequency of the IMU signal */
#define IMU_FS 100

/* 
	Number of samples for the IMU signals 
	Note that each sample contains the meaasurement of each of
	the three axis of the accelerometer and gyroscope.
*/
#define IMU_LEN 30



/*
	Each sub-array contains one sample per each IMU signal
	in the following order:
	{
		Accelerometer_x,
		Accelerometer_y,
		Accelerometer_z,
		Gyroscope_z,
		Gyroscope_p,
		Gyroscope_r
	}
*/

PI_L2 float imu_in[IMU_LEN][6] = {
    {
        -0.71,
        1.5,
        104.86,
        -9.74,
        0.2,
        -2.73
    },
    {
        -0.73,
        1.49,
        104.89,
        -9.71,
        0.2,
        -2.69
    },
    {
        -0.76,
        1.49,
        104.91,
        -9.67,
        0.2,
        -2.69
    },
    {
        -0.78,
        1.49,
        104.91,
        -9.63,
        0.2,
        -2.81
    },
    {
        -0.81,
        1.49,
        104.89,
        -9.67,
        0.24,
        -2.89
    },
    {
        -0.84,
        1.49,
        104.83,
        -9.74,
        0.2,
        -2.85
    },
    {
        -0.88,
        1.5,
        104.74,
        -9.82,
        0.24,
        -2.73
    },
    {
        -0.94,
        1.5,
        104.64,
        -9.74,
        0.17,
        -2.65
    },
    {
        -1.02,
        1.52,
        104.51,
        -9.74,
        0.01,
        -2.89
    },
    {
        -1.1,
        1.56,
        104.35,
        -9.78,
        0.13,
        -2.85
    },
    {
        -1.18,
        1.61,
        104.17,
        -9.71,
        0.32,
        -2.65
    },
    {
        -1.25,
        1.66,
        103.96,
        -9.63,
        0.52,
        -2.22
    },
    {
        -1.28,
        1.7,
        103.74,
        -9.63,
        0.56,
        -2.18
    },
    {
        -1.29,
        1.74,
        103.5,
        -9.67,
        0.6,
        -2.07
    },
    {
        -1.28,
        1.77,
        103.26,
        -9.71,
        0.56,
        -2.03
    },
    {
        -1.27,
        1.79,
        102.99,
        -9.71,
        0.28,
        -1.91
    },
    {
        -1.28,
        1.83,
        102.7,
        -9.74,
        0.24,
        -1.56
    },
    {
        -1.33,
        1.87,
        102.41,
        -9.78,
        0.24,
        -1.52
    },
    {
        -1.4,
        1.92,
        102.14,
        -9.82,
        0.2,
        -1.56
    },
    {
        -1.47,
        1.95,
        101.9,
        -9.9,
        0.17,
        -1.4
    },
    {
        -1.54,
        1.96,
        101.72,
        -10.17,
        0.32,
        -1.48
    },
    {
        -1.58,
        1.95,
        101.63,
        -10.33,
        0.48,
        -1.6
    },
    {
        -1.6,
        1.92,
        101.59,
        -10.33,
        0.36,
        -1.99
    },
    {
        -1.6,
        1.89,
        101.56,
        -10.29,
        0.4,
        -2.26
    },
    {
        -1.61,
        1.87,
        101.45,
        -9.98,
        0.4,
        -3.08
    },
    {
        -1.62,
        1.89,
        101.21,
        -9.82,
        0.4,
        -3.2
    },
    {
        -1.63,
        1.95,
        100.83,
        -9.74,
        0.44,
        -3.24
    },
    {
        -1.59,
        2.03,
        100.34,
        -9.82,
        0.63,
        -2.89
    },
    {
        -1.46,
        2.12,
        99.86,
        -9.55,
        0.71,
        -1.36
    },
    {
        -1.25,
        2.16,
        99.44,
        -9.12,
        0.83,
        -1.95
    }
};

#endif