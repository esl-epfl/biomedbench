
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



#ifndef MORPH_FILT_H_
#define MORPH_FILT_H_

#include "stdint.h"

#define TYPE int16_t
#define OPENING_WIN 50
#define CLOSING_WIN 75
#define BL_LATENCY (OPENING_WIN-1 + CLOSING_WIN-1)
// Needs to be a power of two bigger than the window
#define Q0_SIZE 64
// Needs to be a power of two bigger than the sum of both windows minus 1
#define Q1_SIZE 128
// Needs to be a power of two bigger than the window
#define Q2_SIZE 128

#define Q0_MASK (Q0_SIZE-1)
#define Q1_MASK (Q1_SIZE-1)
#define Q2_MASK (Q2_SIZE-1)

// Baseliner ──────────────────────────────────────────────────────────────────

/**
 * Data structure that uses an opening followed by a closing
 * to delineate the baseline of a sequence.
 */
typedef struct {
    int16_t pos;

    uint16_t q0_front;
    uint16_t q0_back;
    uint16_t q0_poss[Q0_SIZE];
    TYPE q0_vals[Q0_SIZE];

    uint16_t q1_front;
    uint16_t q1_back;
    uint16_t q1_poss[Q1_SIZE];
    TYPE q1_vals[Q1_SIZE];

    uint16_t q2_front;
    uint16_t q2_back;
    uint16_t q2_poss[Q2_SIZE];
    TYPE q2_vals[Q2_SIZE];
} Baseliner;

void InitBaseliner(Baseliner* b);
TYPE BaselinerProcess(Baseliner* b, TYPE input);

// BaselineFilt ───────────────────────────────────────────────────────────────

/**
 * A filter that eliminates the baseline from an input sequence.
 */
typedef struct {
    Baseliner baseliner;
    TYPE buffer[BL_LATENCY];
    int pos;
} BaselineFilt;

inline void InitBaselineFilt(BaselineFilt* f) {
    InitBaseliner(&f->baseliner);
    f->pos = 0;
}

inline TYPE BaselineFiltProcess(BaselineFilt* f, TYPE input) {
    TYPE old_input = f->buffer[f->pos];
    f->buffer[f->pos] = input;
    ++f->pos;
    if (f->pos == BL_LATENCY) {
        f->pos = 0;
    }
    return old_input - BaselinerProcess(&f->baseliner, input);
}

// HighFreqFilt ───────────────────────────────────────────────────────────────

#define HF_FILT_250HZ_WIN 5
#define HF_FILT_250HZ_LATENCY 2*(HF_FILT_250HZ_WIN/2)

/**
 * A filter that eliminates the high frequeny noise.
 * Targetted for ECG signals taken at 250Hz.
 */
typedef struct {
    int pos;
    TYPE original[HF_FILT_250HZ_WIN];
    TYPE dilation[HF_FILT_250HZ_WIN];
    TYPE erosion[HF_FILT_250HZ_WIN];
} HighFreqFilt250Hz;

void InitHighFreqFilt250Hz(HighFreqFilt250Hz* f);
TYPE HighFreqFilt250HzProcess(HighFreqFilt250Hz* f, TYPE input);

void filterWindows(int32_t *arg[]);

#endif  // MORPH_FILT_H_
