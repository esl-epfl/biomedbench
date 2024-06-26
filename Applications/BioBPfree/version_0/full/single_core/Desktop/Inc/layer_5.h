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


//////////////////////////////////////////
// Author:  Dimitrios Samakovlis        //
// Date:    February 2024               //
//////////////////////////////////////////

#ifndef _LAYER_5_H_
#define _LAYER_5_H_

#include "defines.h"

my_type weights_l5[] = {0.000000000017333, 0.000000000000798, 0.000000000000002, -0.000000000001298, 0.000000000001029, -0.000000000000001, 0.476857453584671, 0.000000081942432, 0.000000000000002, 0.000000000000013, -0.000000000000018, -0.000000000000001, -0.000000000000684, -0.000000000000017, 0.000000000000341, 0.000000000000003, -0.000000000000797, -0.000000000106054, 0.000000000000006, 0.448909610509872, 0.000000000000001, -0.000000000000004, -0.000000000000101, -0.000000000000000, -0.434266209602356, -0.000000000000001, -0.000000000000002, -0.000000000000002, 0.000000000000034, 0.000000000000003, -0.000000000000002, 0.000000000000001, 0.000000000361227, 0.000000000000023, -0.000000000000031, -0.000000000005701, -0.000000000000002, 0.000000000000000, 0.000000067085573, -0.000000000000000, 0.000000000000000, 0.000000000000001, 0.000000000000007, -0.000000000000000, 0.000000000000001, -0.000000000000152, -0.000000000000010, -0.000000000035329, -0.000000000001491, 0.000000000000012, -0.000000000000046, 0.000000000118995, -0.000000000000001, -0.000000000000356, -0.000000000000004, 0.578589558601379, 0.000000000036737, -0.640663683414459, 0.000000000042076, 0.439241439104080, 0.000000000000003, 0.322883874177933, 0.000000000000004, 0.000000000000067, -0.000000000000485, 0.472492754459381, 0.000000092281397, -0.000000000000018, -0.000000000000000, -0.000000000000020, 0.000000000000027, -0.000000000000002, -0.450112164020538, 0.000000000000005, 0.000000000000004, 0.000000000000005, 0.000000000000015, -0.000000000000011, -0.000000006314931, 0.000000000001321, -0.000000000000279, 0.000000000000002, 0.000000000001186, -0.000000000000971, 0.000000000000005, 0.000000000000007, -0.683443844318390, 0.000000000000215, 0.000000000000001, -0.000000000000073, -0.000000000000002, -0.041103973984718, 0.000000000000017, -0.000000000000239, -0.000000000000001, -0.166283994913101, -0.000000000000134, -0.448334991931915, -0.409240841865540, -0.000000000000025, -0.000000000017335, 0.000000000000799, -0.000000000000002, 0.000000000001297, -0.000000000001029, 0.000000000000001, -0.476851552724838, -0.000000081957239, -0.000000000000002, -0.000000000000013, 0.000000000000017, 0.000000000000001, 0.000000000000684, -0.000000000000018, -0.000000000000341, -0.000000000000003, 0.000000000000797, 0.000000000106073, -0.000000000000006, -0.448904544115067, -0.000000000000001, 0.000000000000004, -0.000000000000101, 0.000000000000000, 0.434261411428452, 0.000000000000001, 0.000000000000002, 0.000000000000002, -0.000000000000034, -0.000000000000003, 0.000000000000002, -0.000000000000001, -0.000000000361287, 0.000000000000023, 0.000000000000031, 0.000000000005702, 0.000000000000002, -0.000000000000000, -0.000000067097673, 0.000000000000000, -0.000000000000000, -0.000000000000001, 0.000000000000009, 0.000000000000000, -0.000000000000001, 0.000000000000152, 0.000000000000010, 0.000000000035335, 0.000000000001490, -0.000000000000012, 0.000000000000046, -0.000000000119015, 0.000000000000001, -0.000000000000356, 0.000000000000004, -0.578583180904388, -0.000000000036743, 0.640656411647797, -0.000000000042083, -0.439236670732498, -0.000000000000004, -0.322880685329437, -0.000000000000004, -0.000000000000067, 0.000000000000485, -0.472486972808838, -0.000000092297604, 0.000000000000017, 0.000000000000000, 0.000000000000020, -0.000000000000027, 0.000000000000002, 0.450107246637344, -0.000000000000005, -0.000000000000004, -0.000000000000005, -0.000000000000015, -0.000000000000011, 0.000000006316041, -0.000000000001321, 0.000000000000279, -0.000000000000002, -0.000000000001185, 0.000000000000971, -0.000000000000005, -0.000000000000008, 0.683435797691345, 0.000000000000215, -0.000000000000001, 0.000000000000073, 0.000000000000002, 0.041103620082140, -0.000000000000017, 0.000000000000239, 0.000000000000001, 0.166282609105110, 0.000000000000134, 0.448330342769623, 0.409235894680023, 0.000000000000025};
my_type bias_l5[] = {0.012418611906469, -0.016451222822070};

#endif