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

#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

#include "defines.h"

#include "pmsis.h"

// 2 healthy + 2 unhealthy inputs
PI_L2 extern const my_type x_p1[2048];

PI_L2 extern const my_type x_p2[2048];

PI_L2 extern const my_type x_p3[2048];

PI_L2 extern const my_type x_p4[2048];

extern const my_type *x_set[4];


// *** Filter + Bias parameters ***

extern my_type filters_[6144]; 

extern my_type bias_[64];

PI_L2 extern const my_type gamma_[64];

PI_L2 extern const my_type beta_[64];

PI_L2 extern const my_type mean_[64];

PI_L2 extern const my_type var_[64];

#endif
