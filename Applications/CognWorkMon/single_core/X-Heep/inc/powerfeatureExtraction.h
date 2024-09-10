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



#ifndef POWER_FEAT_EXTRACTION_H
#define POWER_FEAT_EXTRACTION_H

#include <featureExtraction.h>

int8_t PowerFeatureExtractionInit(uint16_t fftSize, unsigned short samp_freq, uint8_t nBatches, uint8_t nchannels);
int8_t PowerFeatures_BatchCalc(my_int *sig, uint8_t channel);
void PowerFeatures_GetFeat(my_int *features, uint8_t channel);
void Psd(my_int *periodogram, my_int *power_density, int sampling_frequency, int numel_power);
void Periodogram(my_int *sig, my_int *periodogram, int numel_sig);
my_int Bpower(my_int *power_density, my_int f1, my_int f2, my_int frequency_resolution);

void endPowerModule();

#endif
