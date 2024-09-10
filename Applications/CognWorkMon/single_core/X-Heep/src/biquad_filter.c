
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

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <fixmath.h>
#include <biquad_filter.h>

//Initializes the filter instance with the coefficients (pCoeffs)
void init_filter(filter_instance *S, uint8_t numStages, fixed_t *pCoeffs, my_int *pState)
{
    S->numStages = numStages;
    S->pCoeffs = pCoeffs;
    memset(pState, 0, (4u * (uint32_t)numStages) * sizeof(my_int));
    S->pState = pState;
}

void biquad_filter(filter_instance *S, my_int *pSrc, my_int *pDst, my_int blockSize)
{
    my_int *pIn = pSrc;                    /*  source pointer            */
    my_int *pOut = pDst;                   /*  destination pointer       */
    my_int *pState = S->pState;            /*  pState pointer            */
    my_int *pCoeffs = S->pCoeffs;          /*  coefficient pointer       */
    my_int acc;                            /*  Simulates the accumulator */
    my_int b0, b1, b2, a1, a2;             /*  Filter coefficients       */
    my_int Xn1, Xn2, Yn1, Yn2;             /*  Filter pState variables   */
    my_int Xn;                             /*  temporary input           */
    uint32_t sample, stage = S->numStages; /*  loop counters             */

    do
    {
        //Reading the cefficients
        b0 = *pCoeffs++;
        b1 = *pCoeffs++;
        b2 = *pCoeffs++;
        a1 = *pCoeffs++;
        a2 = *pCoeffs++;

        /* Reading the pState values */
        Xn1 = pState[0];
        Xn2 = pState[1];
        Yn1 = pState[2];
        Yn2 = pState[3];

        //sample = blockSize >> 2u;
        sample = blockSize;

        while (sample > 0u)
        {
            //---------------------------------------
            //This is the one without loop unrolling!
            //---------------------------------------
            //
            /* Read the input */
            Xn = *pIn++;

            /* acc =  b0 * x[n] + b1 * x[n-1] + b2 * x[n-2] + a1 * y[n-1] + a2 * y[n-2] */

            acc = fx_mulx(b0, Xn, N_DEC_BIQ) + fx_mulx(b1, Xn1, N_DEC_BIQ) + fx_mulx(b2, Xn2, N_DEC_BIQ) +
                  fx_mulx(a1, Yn1, N_DEC_BIQ) + fx_mulx(a2, Yn2, N_DEC_BIQ);

            /* Store the result in the accumulator in the destination buffer. */
            *pOut++ = acc;

            /* Every time after the output is computed state should be updated. */
            Xn2 = Xn1;
            Xn1 = Xn;
            Yn2 = Yn1;
            Yn1 = acc;

            /* decrement the loop counter */
            sample--;
        }

        /*  Store the updated state variables back into the pState array */
        *pState++ = Xn1;
        *pState++ = Xn2;
        *pState++ = Yn1;
        *pState++ = Yn2;

        /*  The first stage goes from the input buffer to the output buffer. */
        /*  Subsequent numStages  occur in-place in the output buffer */
        pIn = pDst;

        /* Reset the output pointer */
        pOut = pDst;

        /* decrement the loop counter */
        stage--;

    } while (stage > 0u);
}
