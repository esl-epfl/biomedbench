/*
Copyright 2022 Mattia Orlandi

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "shared_buf.h"

#include "defines.h"

PI_L1 float buffer[NUM_CORES];
PI_L1 float g_acc;
PI_L1 float tmp1_data[N_CH_EXT];
PI_L1 float tmp2_data[N_CH_EXT * N_CH_EXT];
PI_L1 float tmp3_data[N_MU * N_CH_EXT];
PI_L1 float tmp4_data[N_MU];
PI_L1 float tmp5_data[N_CH_EXT * Q];
PI_L1 float tmp6_data[N_CH_EXT * EXT_WIN];
PI_L1 uint8_t tmp7_data[N_MU * N_SAMPLES];
