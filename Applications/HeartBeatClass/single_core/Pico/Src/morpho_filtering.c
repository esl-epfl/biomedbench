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



#include "morpho_filtering.h"
#include "defines.h"

// for runtime - one for each core/lead
BaselineFilt bl_filt[NLEADS];
HighFreqFilt250Hz hf_filt[NLEADS];

void InitBaseliner(Baseliner* b) {
    b->pos = 0;
    b->q0_front = 0;
    b->q0_back = 0;
    b->q1_front = 0;
    b->q1_back = 0;
    b->q2_front = 0;
    b->q2_back = 0;
}

/**
 * Processes the next input value an returns the baseline value.
 * This function has a latency of LATENCY positions.
 */
TYPE BaselinerProcess(Baseliner* b, TYPE input) {
    // We are calculating the closing(opening(sequence)), where
    // opening(x) = dilation(erosion(x)) and
    // closing(x) = erosion(dilation(x)).
    // The two consecutives dilations are merged into a single dilation.
    ++b->pos;

    // First erosion is of size OPENING_WIN
    // We maintain a queue where only the last OPENING_WIN elements can be,
    // in order to find the minimum over those elements fast.

    // If the new element is smaller than another that it is in the queue,
    // we can remove the older one because the new one
    // would stay more time in the queue than the older and it is
    // always an smaller element.
    while (b->q0_front != b->q0_back && input < b->q0_vals[b->q0_back]) {
        // Pop back:
        b->q0_back = (b->q0_back-1)&Q0_MASK;
    }
    // We add the new element to the queue.
    // By the previous while, the queue is in increasing (front to back) order.
    b->q0_back = (b->q0_back+1)&Q0_MASK;
    b->q0_poss[b->q0_back] = b->pos;
    b->q0_vals[b->q0_back] = input;
    // Remove the element (if it has not been already removed)
    // which is now OPENING_WIN positions away from the current position,
    if (OPENING_WIN <=
        ((b->pos - b->q0_poss[(b->q0_front+1)&Q0_MASK])&Q0_MASK)) {
        // Pop front:
        b->q0_front = (b->q0_front+1)&Q0_MASK;
    }
    // The smallest element in this window is in the front of the queue.
    input = b->q0_vals[(b->q0_front+1)&Q0_MASK];

    // Dilation of size OPENING_WIN+CLOSING_WIN-1,
    // equivalent to a dilation of OPENING_WIN and one of CLOSING_WIN.
    // The code is similar to the erosion.

    while (b->q1_front != b->q1_back && input > b->q1_vals[b->q1_back]) {
        b->q1_back = (b->q1_back-1)&Q1_MASK;
    }
    b->q1_back = (b->q1_back+1)&Q1_MASK;
    b->q1_poss[b->q1_back] = b->pos;
    b->q1_vals[b->q1_back] = input;
    if (OPENING_WIN+CLOSING_WIN-1 <=
        ((b->pos - b->q1_poss[(b->q1_front+1)&Q1_MASK])&Q1_MASK)) {
        b->q1_front = (b->q1_front+1)&Q1_MASK;
    }
    input = b->q1_vals[(b->q1_front+1)&Q1_MASK];

    // Erosion of size CLOSING_WIN.

    while (b->q2_front != b->q2_back && input < b->q2_vals[b->q2_back]) {
        b->q2_back = (b->q2_back-1)&Q2_MASK;
    }
    b->q2_back = (b->q2_back+1)&Q2_MASK;
    b->q2_poss[b->q2_back] = b->pos;
    b->q2_vals[b->q2_back] = input;
    if (CLOSING_WIN <=
        ((b->pos  - b->q2_poss[(b->q2_front+1)&Q2_MASK])&Q2_MASK)) {
        b->q2_front = (b->q2_front+1)&Q2_MASK;
    }
    return b->q2_vals[(b->q2_front+1)&Q2_MASK];
}


void InitHighFreqFilt250Hz(HighFreqFilt250Hz* f) {
    f->pos = 0;
}

TYPE HighFreqFilt250HzProcess(HighFreqFilt250Hz* f, TYPE input) {
    static TYPE B[HF_FILT_250HZ_WIN*HF_FILT_250HZ_WIN] = {
        0, 0, 1, 5, 1,
        1, 0, 0, 1, 5,
        5, 1, 0, 0, 1,
        1, 5, 1, 0, 0,
        0, 1, 5, 1, 0
    };
    f->original[f->pos] = input;
    TYPE* Bi = B + HF_FILT_250HZ_WIN*f->pos;
    f->dilation[f->pos] = f->original[0] + Bi[0];
    f->erosion[f->pos] = f->original[0] - Bi[0];
    // Given the small size of this loops,
    // it is faster to run them than to try to
    // use a better algorithm.
    for (int i = 1; i < HF_FILT_250HZ_WIN; ++i) {
        if (f->original[i] + Bi[i] > f->dilation[f->pos]) {
            f->dilation[f->pos] = f->original[i] + Bi[i];
        }
        if (f->original[i] - Bi[i] < f->erosion[f->pos]) {
            f->erosion[f->pos] = f->original[i] - Bi[i];
        }
    }
    TYPE closing = f->dilation[0];
    TYPE opening = f->erosion[0];
    for (int i = 1; i < HF_FILT_250HZ_WIN; ++i) {
        if (f->dilation[i] < closing) {
            closing = f->dilation[i];
        }
        if (f->erosion[i] > opening) {
            opening = f->erosion[i];
        }
    }
    ++f->pos;
    if (f->pos == HF_FILT_250HZ_WIN) f->pos = 0;
    return (opening + closing)/2;
}

void filterWindows(int32_t *arg[])
{
	int16_t *ecg_buffer = (int16_t*) arg[0];
	int32_t *flag = arg[8];	// this was used in original ESWEEK to separate first run from subsequent
	int16_t *i_lead = (int16_t*)arg[9], temp, offset, threshold;
	int32_t *bufferSize = arg[10];
	
	if (*flag == 0)	{
	    InitBaselineFilt(&bl_filt[(*i_lead)]);
	    InitHighFreqFilt250Hz(&hf_filt[(*i_lead)]);
	}
    
	// Code added to match special first case of ESWEEK - need to compensate the offset
	/*if (*flag == 0)	{
	     offset = 26;
	    threshold = 158;
	}
	else 
	    offset = 0;
	*/
	for (int indsample = 0; indsample<*bufferSize; indsample++) {
		 ecg_buffer[indsample + dim * (*i_lead)] = HighFreqFilt250HzProcess(&hf_filt[(*i_lead)], BaselineFiltProcess(&bl_filt[(*i_lead)], ecg_buffer[indsample + dim * (*i_lead)]));
		 /*
		if (indsample < threshold)	{
		    ecg_buffer[indsample + dim * i_lead] = 0;
		}
		else {
		    ecg_buffer[indsample + dim * i_lead - offset] = temp;
		}*/
	}
}



