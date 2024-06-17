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



#include "output.hpp"
extern "C" {
#include <am_util_stdio.h>
}
#include "global_config.hpp"

void OutputVector(const int32_t* v, int size, const char* name) {
#ifdef PRINTING
    am_util_stdio_printf("%s[%d] = [\n", name, size);
    for (int i = 0; i < size; ++i) {
        am_util_stdio_printf("%d ", v[i]);
    }
    am_util_stdio_printf("\n]\n");
#endif
}

void OutputVector(const int16_t* v, int size, const char* name) {
#ifdef PRINTING
    am_util_stdio_printf("%s[%d] = [\n", name, size);
    for (int i = 0; i < size; ++i) {
        am_util_stdio_printf("%d ", v[i]);
    }
    am_util_stdio_printf("\n]\n");
#endif
}

void OutputFixVector(const int32_t* v, int size, const char* name, int FRAC) {
#ifdef PRINTING
    if (!FLOAT_OUTPUT) {
        OutputVector(v, size, name);
        return;
    }
    am_util_stdio_printf("%s[%d] = [\n", name, size);
    for (int i = 0; i < size; ++i) {
        am_util_stdio_printf("%f ", fx_xtof(v[i], FRAC));
    }
    am_util_stdio_printf("\n]\n");
#endif
}

void Output(const RriStatisticalFeats& f) {
#ifdef PRINTING
    am_util_stdio_printf(
        "RriStatisticalFeats[mean, stdDev, sqrt(order 2), "
        "%% irregularities] = [\n"
    );
    if (FLOAT_OUTPUT) {
        am_util_stdio_printf("%f\n", fx_xtof(f.mean, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.stdDev, RRI_FRAC));
        // am_util_stdio_printf("%f\n", fx_xtof(f.variance, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.sqrtOrder2Derivative, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.percIrregularities, RRI_FRAC));
    } else {
        am_util_stdio_printf("%d\n", f.mean);
        am_util_stdio_printf("%d\n", f.stdDev);
        // am_util_stdio_printf("%d\n", f.variance);
        am_util_stdio_printf("%d\n", f.sqrtOrder2Derivative);
        am_util_stdio_printf("%d\n", f.percIrregularities);
    }
    am_util_stdio_printf("]\n");
#endif
}

void Output(const RriLorenzFeats& f) {
#ifdef PRINTING
    am_util_stdio_printf("RriLorenzFeats[std1, std2, T, L, csi, modCsi, cvi] = [\n");
    if (FLOAT_OUTPUT) {
        am_util_stdio_printf("%f\n", fx_xtof(f.std1, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.std2, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.T, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.L, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.csi, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.modCsi, RRI_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.cvi, RRI_FRAC));
    } else {
        am_util_stdio_printf("%d\n", f.std1);
        am_util_stdio_printf("%d\n", f.std2);
        am_util_stdio_printf("%d\n", f.T);
        am_util_stdio_printf("%d\n", f.L);
        am_util_stdio_printf("%d\n", f.csi);
        am_util_stdio_printf("%d\n", f.modCsi);
        am_util_stdio_printf("%d\n", f.cvi);
    }
    am_util_stdio_printf("]\n");
#endif
}

void Output(const RriFeats& f) {
    Output(f.stats);
    Output(f.lorenz);
}

void Output(const RriFreqFeats& f) {
#ifdef PRINTING
    am_util_stdio_printf("RriFreqFeats[totPow, lf%%, hf%%, lf/hf] = [\n");
    if (FLOAT_OUTPUT) {
        am_util_stdio_printf("%f\n", fx_xtof(f.totPow, FLOMB_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.lowFreqRatio, FLOMB_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.highFreqRatio, FLOMB_FRAC));
        am_util_stdio_printf("%f\n", fx_xtof(f.lowToHighFreqRatio, FLOMB_FRAC));
    } else {
        am_util_stdio_printf("%d\n", f.totPow);
        am_util_stdio_printf("%d\n", f.lowFreqRatio);
        am_util_stdio_printf("%d\n", f.highFreqRatio);
        am_util_stdio_printf("%d\n", f.lowToHighFreqRatio);
    }
    am_util_stdio_printf("]\n");
#endif
}

void PrintModuleTime(unsigned cycles, const char* name) {
#ifdef PRINTING
    if (PRINT_MODULES_TIME) am_util_stdio_printf("%s time (cycles): %u\n", name, cycles);
#endif
}

