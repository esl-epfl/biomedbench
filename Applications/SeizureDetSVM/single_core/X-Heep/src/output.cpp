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
// Porting to X-Heep: Francesco Poluzzi         //
/////////////////////////////////////////////////



#include "output.hpp"
extern "C" {
#include <stdio.h>
}
#include "global_config.hpp"
#include "print_float.hpp"

void OutputVector(const int32_t* v, int size, const char* name) {
    printf("%s[%d] = [\n", name, size);
    for (int i = 0; i < size; ++i) {
        printf("%d ", v[i]);
    }
    printf("\n]\n");
}

void OutputVector(const int16_t* v, int size, const char* name) {
    printf("%s[%d] = [\n", name, size);
    for (int i = 0; i < size; ++i) {
        printf("%d ", v[i]);
    }
    printf("\n]\n");
}

void OutputFixVector(const int32_t* v, int size, const char* name, int FRAC) {
    if (!FLOAT_OUTPUT) {
        OutputVector(v, size, name); // Assuming OutputVector is defined elsewhere
        return;
    }
    printf("%s[%d] = [\n", name, size);
    for (int i = 0; i < size; ++i) {
        print_float_cpp(fx_xtof(v[i], FRAC), 6);
        printf(" ");
    }
    printf("\n]\n");
}

void Output(const RriStatisticalFeats& f) {
    printf(
        "RriStatisticalFeats[mean, stdDev, sqrt(order 2), "
        "%% irregularities] = [\n"
    );
    if (FLOAT_OUTPUT) {
        print_float_cpp(fx_xtof(f.mean, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.stdDev, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.sqrtOrder2Derivative, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.percIrregularities, RRI_FRAC), 6);
        printf("\n");
    } else {
        printf("%d\n", f.mean);
        printf("%d\n", f.stdDev);
        printf("%d\n", f.sqrtOrder2Derivative);
        printf("%d\n", f.percIrregularities);
    }
    printf("]\n");
}

void Output(const RriLorenzFeats& f) {
    printf("RriLorenzFeats[std1, std2, T, L, csi, modCsi, cvi] = [\n");
    if (FLOAT_OUTPUT) {
        print_float_cpp(fx_xtof(f.std1, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.std2, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.T, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.L, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.csi, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.modCsi, RRI_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.cvi, RRI_FRAC), 6);
        printf("\n");
    } else {
        printf("%d\n", f.std1);
        printf("%d\n", f.std2);
        printf("%d\n", f.T);
        printf("%d\n", f.L);
        printf("%d\n", f.csi);
        printf("%d\n", f.modCsi);
        printf("%d\n", f.cvi);
    }
    printf("]\n");
}

void Output(const RriFeats& f) {
    Output(f.stats);
    Output(f.lorenz);
}

void Output(const RriFreqFeats& f) {
    printf("RriFreqFeats[totPow, lf%%, hf%%, lf/hf] = [\n");
    if (FLOAT_OUTPUT) {
        print_float_cpp(fx_xtof(f.totPow, FLOMB_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.lowFreqRatio, FLOMB_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.highFreqRatio, FLOMB_FRAC), 6);
        printf("\n");
        print_float_cpp(fx_xtof(f.lowToHighFreqRatio, FLOMB_FRAC), 6);
        printf("\n");
    } else {
        printf("%d\n", f.totPow);
        printf("%d\n", f.lowFreqRatio);
        printf("%d\n", f.highFreqRatio);
        printf("%d\n", f.lowToHighFreqRatio);
    }
    printf("]\n");
}


