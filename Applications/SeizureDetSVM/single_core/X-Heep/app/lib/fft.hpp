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



#ifndef FFT_HPP_
#define FFT_HPP_

#include "../global_config.hpp"
extern "C" {
    #include "math.h"
    #include "fixmath.h"
}

#ifdef RISCV_ASS
    const int FFT_FRAC = 30;
#else
    const int FFT_FRAC = 15;
#endif

struct complex {
    fixed_t real;
    fixed_t imag;

    complex& operator+=(const complex& rhs) {
        real += rhs.real;
        imag += rhs.imag;
        return *this;
    }

    complex& operator-=(const complex& rhs) {
        real -= rhs.real;
        imag -= rhs.imag;
        return *this;
    }

    complex& operator*=(const complex& rhs) {
        fixed_t auxr = real;
        real = fx_mulx(real, rhs.real, FFT_FRAC)
             - fx_mulx(imag, rhs.imag, FFT_FRAC);
        imag = fx_mulx(auxr, rhs.imag, FFT_FRAC)
             + fx_mulx(imag, rhs.real, FFT_FRAC);
        return *this;
    }

    // complex& operator/=(fixed_t real) {
    //     real /= real;
    //     imag /= real;
    //     return *this;
    // }
};

inline void swap(complex& lhs, complex& rhs) {
    fixed_t aux = lhs.real;
    lhs.real = rhs.real;
    rhs.real = aux;
    aux = lhs.imag;
    lhs.imag = rhs.imag;
    rhs.imag = aux;
    // swap(lhs.real, rhs.real);
    // swap(lhs.imag, rhs.imag);
}

inline complex operator+(complex lhs, const complex& rhs) {
    return lhs += rhs;
}

inline complex operator-(complex lhs, const complex& rhs) {
    return lhs -= rhs;
}

inline complex operator*(complex lhs, const complex& rhs) {
    return lhs *= rhs;
}

// template<typename T>
// complex<T> operator/(complex<T> lhs, const T& rhs) {
//     return lhs /= rhs;
// }

using cn = complex;

#ifdef RISCV_ASS

inline int32_t mul_32b_16b(int32_t lhs, int32_t rhs) {
    int32_t res;
  __asm__( 
"  mulh t0, %1, %2\n"
"  mul t1, %1, %2\n"
"  slli t0, t0, 2\n"
"  srli t1, t1, 30\n"
"  add %0,t0,t1\n"
:"=r" (res) /* %0: Output variable list */
:"r" (lhs), "r" (rhs) /* %1: Input variable list */
:"t0", "t1" /* Overwritten registers ("Clobber list") */
  );
  return res;
}

#else

inline int32_t mul_32b_16b(int32_t lhs, int32_t rhs) {
    // (((int64_t)(x1)*(int64_t)(x2) >> ((frac) - 1)) + 1) >> 1 :

    // return ((((int64_t)lhs*(int64_t)rhs) >> (frac-1)) + 1) >> 1;

    int32_t lhsl = lhs & ((1 << 16) - 1);
    int32_t lhsu = lhs >> 16;
    int32_t mul = ((lhsu*rhs) << (16-FFT_FRAC)) +
                  ((lhsl*rhs) >> FFT_FRAC);
    return mul;
}

#endif

inline complex fastmul(complex lhs, const complex& rhs) {
    fixed_t auxr = lhs.real;
    lhs.real = mul_32b_16b(lhs.real, rhs.real)
             - mul_32b_16b(lhs.imag, rhs.imag);
    lhs.imag = mul_32b_16b(auxr, rhs.imag)
             + mul_32b_16b(lhs.imag, rhs.real);
    return lhs;
}

/* Modified Fft by Dimitrios Samakovlis
   Removed Struct and coded C style with global variables and functions
   Implementation in.cpp file
*/

/**
 * FLOMB_MAX_SIZE maximum size of a polynomial. Power of 2.
 */
void Fft (cn pol[], const int n);

#endif // FFT_HPP_

