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

extern const float imu_in[IMU_LEN][6];

#endif
