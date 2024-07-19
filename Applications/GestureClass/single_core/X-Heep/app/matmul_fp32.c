// Porting to X-Heep : Francesco Poluzzi
/*
 * Copyright (C) 2021-2022 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Authors: Davide Nadalini, Leonardo Ravaglia
*/

///////////////////////////////////////////////////////////////////
// Additional contributions by:                                  //
// - Dimitrios Samakovlis (Embedded System Laboratory - EPFL)    //
///////////////////////////////////////////////////////////////////     

#include "matmul_fp32.h"

#ifdef DEBUG
#include <stdio.h>
#endif

/**
 * NAIVE VERSIONS
 */

void mm(void * void_args) {

  struct matMul_args* args = (struct matMul_args *)void_args;
  float * __restrict__ A = args->A;
  float * __restrict__ B = args->B;
  float * __restrict__ C = args->C;

  const int N = args->N;
  const int M = args->M;
  const int K = args->K;

  int transp = args->trans_B;

  const int start = 0;
  const int stop = N;

  // =====> B NOT TRANSPOSED <=====
  if (transp==0)
  {
    if (K == 1) 
    {
      for (int i=start; i < stop; i++) 
      {
        for (int j = 0; j < M; j++) 
        {
          C[i*M+j] = A[i*K] * B[j];
          #ifdef DEBUG
          printf("C[%i] += A[%i] * B[%i] -> %f = %f * %f", i*M+j, i*K, j, C[i*M+j], A[i], B[j]);
          #endif
        }
      }
    }
    else if (K > 0)
    {
      for (int i=start; i < stop; i++) 
      {
        for (int j = 0; j < M; j++) 
        {
          float temp = 0;
          for (int k = 0; k < K; k++) 
          {
                temp += A[i*K+k] * B[j+k*M];
                #ifdef DEBUG
                printf("C[%i] += A[%i] * B[%i] -> %f = %f * %f", i*M+j, i*K+k, j+k*M, C[i*M+j], A[i*K+k], B[j+k*M]);
                #endif
          } 
          C[i*M+j] = temp;
        } 
      } 
    }
  }

  // =====> B IS TRANSPOSED <=====  
  else 
  {
    if (K == 1) 
    {
      for (int i=start; i < stop; i++) 
      {
        for (int j = 0; j < M; j++) 
        {
          C[i*M+j] = A[i*K] * B[j*K];
          #ifdef DEBUG
          printf("C[%i] += A[%i] * B[%i] -> %f = %f * %f
", i*M+j, i, j*K, C[i*M+j], A[i*K], B[j*K]);
          #endif
        } 
      } 
    }
    else if (K > 0)
    {
      for (int i=start; i < stop; i++) 
      {
        for (int j = 0; j < M; j++) 
        {
          float temp = 0;
          for (int k = 0; k < K; k++) 
          {
              temp += A[i*K+k] * B[k+j*K];
              #ifdef DEBUG
              printf("C[%i] += A[%i] * B[%i] -> %f = %f * %f
", i*M+j, i*K+k, k+j*K, C[i*M+j], A[i*K+k], B[k+j*K]);
              #endif
          } 
          C[i*M+j] = temp;
        } 
      } 
    }
  }
}


// Naive matmul with parallelism on M
void mm_M(void * void_args) {

  struct matMul_args* args = (struct matMul_args *)void_args;
  float * __restrict__ A = args->A;
  float * __restrict__ B = args->B;
  float * __restrict__ C = args->C;

  const int N = args->N;
  const int M = args->M;
  const int K = args->K;

  int transp = args->trans_B;

  const int start = 0;
  const int stop = M;

  // =====> B NOT TRANSPOSED <=====
  if (transp==0)
  {
    for (int i = 0; i < N; i++) 
    {
      for (int j=start; j < stop; j++) 
      {
        float temp = 0;
        for (int k = 0; k < K; k++) 
        {
              temp += A[i*K+k] * B[j+k*M];
              #ifdef DEBUG
              printf("C[%i] += A[%i] * B[%i] -> %f = %f * %f", i*M+j, i*K+k, j+k*M, C[i*M+j], A[i*K+k], B[j+k*M]);
              #endif
        } 
        C[i*M+j] = temp;
      } 
    } 
  }

  // =====> B IS TRANSPOSED <=====
  else 
  {
    for (int i = 0; i < N; i++) 
    {
      for (int j=start; j < stop; j++) 
      {
        float temp = 0;
        for (int k = 0; k < K; k++) 
        {
              temp += A[i*K+k] * B[j*K+k];
              #ifdef DEBUG              
              printf("C[%i] += A[%i] * B[%i] -> %f = %f * %f
", i*M+j, i*K+k, k+j*K, C[i*M+j], A[i*K+k], B[k+j*K]);
              #endif
        } 
        C[i*M+j] = temp;
      } 
    } 
  }
}


void mm_unroll_1x2 (void * void_args) 
{
  struct matMul_args* args = (struct matMul_args *)void_args;
  float * __restrict__ A = args->A;
  float * __restrict__ B = args->B;
  float * __restrict__ C = args->C;

  int N = args->N;
  int M = args->M;
  int K = args->K;
  int transp = args->trans_B;

  int start = 0;
  int stop = N;

  // Check if sizes are smaller than the unrolling, and take countermeasures
  if (M < 2) { mm(args); }
  else 
  {
    // =====> B NOT TRANSPOSED <=====
    if (transp == 0) 
    {
      // Unrolled core
      for (int i=start; i<stop; i++) 
      {
        for (int j=0; j<(M & 0xfffffffe); j=j+2)
        {
          float temp0 = 0;
          float temp1 = 0;

          for (int k=0; k<K; k++) 
          {
            int idx   = k*M+j;
            float Ash = A[i*K+k];
            temp0     += Ash * B[idx];
            temp1     += Ash * B[idx+1];
          }
          C[i*M+j]    = temp0;
          C[i*M+j+1]  = temp1;
        }
      }
      // Leftover on M
      if (M & 0x00000001) 
      {
        for (int i=start; i<stop; i++) 
        {
          float temp = 0;
          for (int k=0; k<K; k++)
          {
            temp += A[i*K+k] * B[k*M+M-1];
          }
          C[i*M+M-1] = temp;
        }
      }
    }

    // =====> B IS TRANSPOSED <=====
    else 
    {
      // Unrolled core
      for (int i=start; i<stop; i++) 
      {
        for (int j=0; j<(M & 0xfffffffe); j=j+2)
        {
          float temp0 = 0;
          float temp1 = 0;

          for (int k=0; k<K; k++) 
          {
            int idx   = k+j*K;
            float Ash = A[i*K+k];
            temp0     += Ash * B[idx];
            temp1     += Ash * B[idx+K];
          }
          C[i*M+j]    = temp0;
          C[i*M+j+1]  = temp1;
        }
      }
      // Leftover on M
      if (M & 0x00000001) 
      {
        for (int i=start; i<stop; i++) 
        {
          float temp = 0;
          for (int k=0; k<K; k++)
          {
            temp += A[i*K+k] * B[k+(M-1)*K];
          }
          C[i*M+M-1] = temp;
        }
      }    
    }
  }
}



void mm_unroll_1x4 (void * void_args)
{
  struct matMul_args* args = (struct matMul_args *)void_args;
  float * __restrict__ A = args->A;
  float * __restrict__ B = args->B;
  float * __restrict__ C = args->C;

  int N = args->N;
  int M = args->M;
  int K = args->K;
  int transp = args->trans_B;

  int start = 0;
  int stop = N;

  // Check if sizes are smaller than the unrolling, and take countermeasures
  if (M < 4) { mm_unroll_1x2(args); }
  else 
  {
    // =====> B NOT TRANSPOSED <=====
    if (transp == 0)
    {
      // Unrolled core
      for (int i=start; i<stop; i++) 
      {
        for (int j=0; j<(M & 0xfffffffc); j=j+4)
        {
          float temp0 = 0;
          float temp1 = 0;
          float temp2 = 0;
          float temp3 = 0;

          for (int k=0; k<K; k++) 
          {
            int idx   = k*M+j;
            float Ash = A[i*K+k];
            temp0     += Ash * B[idx];
            temp1     += Ash * B[idx+1];
            temp2     += Ash * B[idx+2];
            temp3     += Ash * B[idx+3];
          }
          C[i*M+j]    = temp0;
          C[i*M+j+1]  = temp1;
          C[i*M+j+2]  = temp2;
          C[i*M+j+3]  = temp3;
        }
      }
      // Leftover on M
      if (M & 0x00000003) 
      {
        for (int i=start; i<stop; i++) 
        {
          for (int j=(M-(M & 0x00000003)); j<M; j++)
          {
            float temp = 0;
            for (int k=0; k<K; k++)
            {
              temp += A[i*K+k] * B[k*M+j];
            }
          C[i*M+j] = temp;
          }
        }
      }
    }

    // =====> B IS TRANSPOSED <=====
    else 
    {
      // Unrolled core
      for (int i=start; i<stop; i++) 
      {
        for (int j=0; j<(M & 0xfffffffc); j=j+4)
        {
          float temp0 = 0;
          float temp1 = 0;
          float temp2 = 0;
          float temp3 = 0;

          for (int k=0; k<K; k++) 
          {
            int idx   = k+j*K;
            float Ash = A[i*K+k];
            temp0     += Ash * B[idx];
            temp1     += Ash * B[idx+K];
            temp2     += Ash * B[idx+2*K];
            temp3     += Ash * B[idx+3*K];
          }
          C[i*M+j]    = temp0;
          C[i*M+j+1]  = temp1;
          C[i*M+j+2]  = temp2;
          C[i*M+j+3]  = temp3;
        }
      }
      // Leftover on M
      if (M & 0x00000003) 
      {
        for (int i=start; i<stop; i++) 
        {
          for (int j=(M-(M & 0x00000003)); j<M; j++)
          {
            float temp = 0;
            for (int k=0; k<K; k++)
            {
              temp += A[i*K+k] * B[k+j*K];
            }
          C[i*M+j] = temp;
          }
        }
      }  
    }
  }
}



void mm_unroll_1x8 (void * void_args)
{
  struct matMul_args* args = (struct matMul_args *)void_args;
  float * __restrict__ A = args->A;
  float * __restrict__ B = args->B;
  float * __restrict__ C = args->C;

  int N = args->N;
  int M = args->M;
  int K = args->K;
  int transp = args->trans_B;

  int start = 0;
  int stop = N;

  // Check if sizes are smaller than the unrolling, and take countermeasures
  if (M < 8) { mm_unroll_1x4(args); }
  else 
  {
    // =====> B NOT TRANSPOSED <=====
    if (transp == 0)
    {
      // Unrolled core
      for (int i=start; i<stop; i++) 
      {
        for (int j=0; j<(M & 0xfffffff8); j=j+8)
        {
          float temp0 = 0;
          float temp1 = 0;
          float temp2 = 0;
          float temp3 = 0;
          float temp4 = 0;
          float temp5 = 0;
          float temp6 = 0;
          float temp7 = 0;

          for (int k=0; k<K; k++) 
          {
            int idx   = k*M+j;
            float Ash = A[i*K+k];
            temp0     += Ash * B[idx];
            temp1     += Ash * B[idx+1];
            temp2     += Ash * B[idx+2];
            temp3     += Ash * B[idx+3];
            temp4     += Ash * B[idx+4];
            temp5     += Ash * B[idx+5];
            temp6     += Ash * B[idx+6];
            temp7     += Ash * B[idx+7];
          }
          C[i*M+j]    = temp0;
          C[i*M+j+1]  = temp1;
          C[i*M+j+2]  = temp2;
          C[i*M+j+3]  = temp3;
          C[i*M+j+4]  = temp4;
          C[i*M+j+5]  = temp5;
          C[i*M+j+6]  = temp6;
          C[i*M+j+7]  = temp7;
        }
      }
      // Leftover on M
      if (M & 0x00000007) 
      {
        for (int i=start; i<stop; i++) 
        {
          for (int j=(M-(M & 0x00000007)); j<M; j++)
          {
            float temp = 0;
            for (int k=0; k<K; k++)
            {
              temp += A[i*K+k] * B[k*M+j];
            }
          C[i*M+j] = temp;
          }
        }
      }
    }

    // =====> B IS TRANSPOSED <=====
    else 
    {
      // Unrolled core
      for (int i=start; i<stop; i++) 
      {
        for (int j=0; j<(M & 0xfffffff8); j=j+8)
        {
          float temp0 = 0;
          float temp1 = 0;
          float temp2 = 0;
          float temp3 = 0;
          float temp4 = 0;
          float temp5 = 0;
          float temp6 = 0;
          float temp7 = 0;

          for (int k=0; k<K; k++) 
          {
            int idx   = k+j*K;
            float Ash = A[i*K+k];
            temp0     += Ash * B[idx];
            temp1     += Ash * B[idx+K];
            temp2     += Ash * B[idx+2*K];
            temp3     += Ash * B[idx+3*K];
            temp4     += Ash * B[idx+4*K];
            temp5     += Ash * B[idx+5*K];
            temp6     += Ash * B[idx+6*K];
            temp7     += Ash * B[idx+7*K];
          }
          C[i*M+j]    = temp0;
          C[i*M+j+1]  = temp1;
          C[i*M+j+2]  = temp2;
          C[i*M+j+3]  = temp3;
          C[i*M+j+4]  = temp4;
          C[i*M+j+5]  = temp5;
          C[i*M+j+6]  = temp6;
          C[i*M+j+7]  = temp7;
        }
      }
      // Leftover on M
      if (M & 0x00000007) 
      {
        for (int i=start; i<stop; i++) 
        {
          for (int j=(M-(M & 0x00000007)); j<M; j++)
          {
            float temp = 0;
            for (int k=0; k<K; k++)
            {
              temp += A[i*K+k] * B[k+j*K];
            }
          C[i*M+j] = temp;
          }
        }
      }    
    }
  }
}


void mm_M_unroll_2x1 (void * void_args) 
{
  struct matMul_args* args = (struct matMul_args *)void_args;
  float * __restrict__ A = args->A;
  float * __restrict__ B = args->B;
  float * __restrict__ C = args->C;

  int N = args->N;
  int M = args->M;
  int K = args->K;
  int transp = args->trans_B;

  int N_par = N & 0xfffffffe;
  int N_left = N - N_par;

  int start = 0;
  int stop = M;

   // Check if sizes are smaller than the unrolling, and take countermeasures
  if  (N < 2) { mm_M(args); }
  else
  { 
    // =====> B NOT TRANSPOSED <=====
    if (transp == 0) 
    {
      for (int j=start; j<stop; j++)
      {
        for (int i=0; i<(N & 0xfffffffe); i=i+2)
        {
          float temp0 = 0;
          float temp1 = 0;
          
          for (int k=0; k<K; k++)
          {
            float Bsh = B[j+k*M];
            int idx0 = i*K+k;
            int idx1 = (i+1)*K+k;
            temp0 += A[idx0] * Bsh;
            temp1 += A[idx1] * Bsh;
          }
          C[i*M+j]      = temp0;
          C[(i+1)*M+j]  = temp1;
        }
      }
      // Leftover on N
      if (N_left > 0)
      {
        for (int j=start; j<stop; j++) 
        {
          float temp = 0;
          for (int k=0; k<K; k++)
          { 
            temp += A[(N-1)*K+k] * B[j+k*M];
          }
          C[(N-1)*M+j] = temp;
        }
      }
    }

    // =====> B IS TRANSPOSED <=====
    else 
    {
      for (int j=start; j<stop; j++)
      {
        for (int i=0; i<(N & 0xfffffffe); i=i+2)
        {
          float temp0 = 0;
          float temp1 = 0;
          
          for (int k=0; k<K; k++)
          {
            float Bsh = B[j*K+k];
            int idx0 = i*K+k;
            int idx1 = (i+1)*K+k;
            temp0 += A[idx0] * Bsh;
            temp1 += A[idx1] * Bsh;
          }
          C[i*M+j]      = temp0;
          C[(i+1)*M+j]  = temp1;
        }
      }
      // Leftover on N
      if (N_left > 0)
      {
        for (int j=start; j<stop; j++) 
        {
          float temp = 0;
          for (int k=0; k<K; k++)
          { 
            temp += A[(N-1)*K+k] * B[j*K+k];
          }
          C[(N-1)*M+j] = temp;
        }
      }
    }
  }
}
