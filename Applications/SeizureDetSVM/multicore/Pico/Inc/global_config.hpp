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


//////////////////////////////////////////////////
// Author:          ESL team                    //  
// Optimizations:   Dimitrios Samakovlis        //
/////////////////////////////////////////////////



#ifndef RPEAK_DEL_CONFIG_HPP_
#define RPEAK_DEL_CONFIG_HPP_

#define SVM_SIZE 74

#define MAX_RPEAKS_PER_CORE (120/NUMBER_OF_CORES)          

const int RPEAK_CAPACITY = 150;

constexpr bool FLOAT_OUTPUT            = false;
constexpr bool PRINT_DELINEATION       = false;
constexpr bool PRINT_EDR               = false;
constexpr bool PRINT_RRI_FAST_FEATURES = false;
constexpr bool PRINT_RRI_FREQ_FEATURES = false;
constexpr bool PRINT_EDR_LPC           = false;
constexpr bool PRINT_FREQ_EDR          = false;
constexpr bool PRINT_FEATURES          = false;
constexpr bool PRINT_SVM               = false;
constexpr bool PRINT_PREDICTION        = false;

constexpr int ECG_FREQ = 64;
constexpr int WIN_SIZE_S = 60;
constexpr int NON_OVERLAP_SIZE_S = 10;
constexpr int OVERLAP_SIZE_S = WIN_SIZE_S - NON_OVERLAP_SIZE_S;
constexpr float WATCHDOG_EXTRA_TIME = 1;
constexpr int WIN_SIZE = WIN_SIZE_S * ECG_FREQ;
constexpr int NON_OVERLAP_SIZE = NON_OVERLAP_SIZE_S * ECG_FREQ;
constexpr int OVERLAP_SIZE = OVERLAP_SIZE_S * ECG_FREQ;

constexpr int ECG_FRAC = 12;
constexpr int RRI_FRAC = 20;
constexpr int FLOMB_FRAC = 18;
constexpr int LPC_FRAC = 24;
constexpr int SVM_FRAC = 23;
constexpr int FFT_SCALE = 10;

constexpr int EDR_LPC_ORDER = 9;
constexpr int EDR_NUM_FREQ_FEATS = 29;
constexpr int NUM_FEATURES = 15 + EDR_NUM_FREQ_FEATS + EDR_LPC_ORDER;
constexpr int FLOMB_MAX_SIZE = 1<<11;

#endif  // RPEAK_DEL_CONFIG_HPP_

