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



#ifndef FIXPNT_LIN_PRED_EST_HPP_
#define FIXPNT_LIN_PRED_EST_HPP_

#include <stdint.h>

namespace fixpnt_32b {

/**
 * Computes the linear prediction coefficients of a sequence
 * obtained via the root mean square criterion or correlation method,
 * which minimizes the expected value of the square error.
 *
 * The linear prediction coefficients
 * of two sequences that differ in a constant factor
 * are the same.
 *
 * This version uses 64 bit variables for accumulation.
 *
 * @param seq input sequence, computed at any scale.
 * @param seqSize size of `seq`.
 * @param order number of coefficients calculated.
 * @param coeffs array of size `order` to store the coefficients.
 * @param frac number of fractional bits used during computation.
 */
void ComputeLinPredCoeffs(const int32_t** arg, int id);

/**
 * Computes the linear prediction coefficients
 * obtained via the correlation method
 * using the normalized correlations of the sequence.
 *
 * Normalized correlations of a sequence seq
 * are given by dividing by corr[0],
 * which is the norm of the sequence,
 *   norm_corr[i] = corr[i+1]/corr[0],
 *   corr[i] = sum_j seq[j]*seq[j+i],
 * and are always between -1 and 1.
 *
 * The normalized correlations
 * must be stored with `frac` fractional bits.
 *
 * @param norm_corr array of size `order` of normalized correlations.
 * @param order number of coefficients calculated.
 * @param coeffs array of size `order` to store the coefficients.
 * @param frac number of fractional bits used during computation.
 */
void ComputeLinPredCoeffs_helper(int32_t* coeffs);

/*namespace acc_32b {

 *
 * Computes the linear prediction coefficients of a sequence
 * obtained via the root mean square criterion or correlation method,
 * which minimizes the expected value of the square error.
 *
 * The linear prediction coefficients
 * of two sequences that differ in a constant factor
 * are the same.
 *
 * This version uses 32 bit variables for accumulation.
 *
 * @param seq input sequence, computed at any scale.
 * @param seqSize size of `seq`.
 * @param order number of coefficients calculated.
 * @param coeffs array of size `order` to store the coefficients.
 * @param frac number of fractional bits used during computation.
 * 
void ComputeLinPredCoeffs(
    const int32_t* seq,
    int seqSize,
    int order,
    int32_t* coeffs,
    int frac
);

} */  // namespace acc_32b

}  // namespace fixpnt_32b

#endif  // FIXPNT_LIN_PRED_EST_HPP_

