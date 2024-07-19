// Porting to X-Heep : Francesco Poluzzi
/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////
// Author:  Dimitrios Samakovlis        //
// Date:    September 2023              //
//////////////////////////////////////////




#ifndef RP_CLASSIFIER_H_
#define RP_CLASSIFIER_H_

#include <stdint.h>

/*
	classify
This function receives as input a buffer (of length multiple of 2 and at least INPUT_FREQUENCY)
that stores the samples of a heart beat to classify. It also receives the length of the buffer
and the index of the sample corresponding to the R peak of the heart beat. The input is not
modified and the output of the function is the number of the classification decission (from 
0 to NUM_CLASS-1) if it recognizes a class or NUM_CLASSES in case of an unknown beat.
0 is assumed to be the class Normal
*/
uint16_t classify(int16_t buffer[], uint16_t buffer_length, uint16_t peak);
void report_rpeak(int32_t *arg[]);

#endif  //RP_CLASSIFIER_H_
