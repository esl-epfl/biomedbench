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



#ifndef _AUDIO_INPUT_55502_W2_H_
#define _AUDIO_INPUT_55502_W2_H_


/* Sampling frequency */
#define AUDIO_FS  16000

/* Number of samples per each audiosignal */
#define AUDIO_LEN   4800


/* 
	The samples are taken from two microphones, one facing the sking
	the other facing the air 
    In this case only the air is used, this is a simplified input
    data set
*/
typedef struct audio_input_55502
{
    float air[AUDIO_LEN];
} audio_input_t;

extern const audio_input_t audio_in;

#endif
