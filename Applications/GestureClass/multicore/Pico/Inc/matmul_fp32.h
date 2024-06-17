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



/**
 * @brief Arguments for standard matrix multiplication C=A*B (A=N*K, B=K*M, result is C=N*M)
 * @param A  pointer to input matrix A
 * @param B  pointer to input matrix B
 * @param C  pointer to output matrix C
 * @param N  rows of A
 * @param M  columns of B
 * @param K  columns of A / rows of B
 * @param trans_B  if set to 1, compute C=A*Bt
 * @param H for Conv2D in grad: input width
 * @param W for Conv2D in grad: input height
 * @param pW for Conv2D in grad: kernel width
 * @param pH for Conv2D in grad: kernel height
 * @param pCin for Conv2D in grad: kernel in channels
 * @param pCout for Conv2D in grad: kernel out channels (number of blocks of filters with pCin channels each)
 */
struct matMul_args {
  float * __restrict__ A;
  float * __restrict__ B;
  float * __restrict__ C;
  int N;
  int M;
  int K;
  int trans_B;
  // For Conv2D in grad
  int H;
  int W;
  int pW;
  int pH;
  int pCin;
  int pCout;
};



/**
 * Collection of different matrix multiply functions for different purposes
 * Use pi_cl_team_fork(NUMBER_OF_CORES, MM_NAME, &args) to parallelize.
 */

/**
 * @brief Naive matrix multiply algorithm, performing C=A*B (C is N*M, A is N*K, B is K*M). Parallelizes on N.
 * @param void_args pointer to a matMul_args structure (please refer to this to setup the args)
 */
void mm(
    void * void_args, int id
);

/**
 * @brief Naive matrix multiply algorithm, performing C=A*B (C is N*M, A is N*K, B is K*M). Parallelizes on M.
 * @param void_args pointer to a matMul_args structure (please refer to this to setup the args)
 */
void mm_M(
	void * void_args, int id
);


/**
 * @brief Standard matmul with unrolling, parallelizes on N. Unrolls 1 row of A, 2 columns of B.
 * @param void_args pointer to a matMul_args structure (please refer to this to setup the args)
 */
void mm_unroll_1x2(
    void * void_args, int id
);

/**
 * @brief Standard matmul with unrolling, parallelizes on N. Unrolls 1 row of A, 4 columns of B.
 * @param void_args pointer to a matMul_args structure (please refer to this to setup the args)
 */
void mm_unroll_1x4(
    void * void_args, int id
);

/**
 * @brief Standard matmul with unrolling, parallelizes on N. Unrolls 1 row of A, 8 columns of B.
 * @param void_args pointer to a matMul_args structure (please refer to this to setup the args)
 */
void mm_unroll_1x8(
    void * void_args, int id
);


/**
 * @brief Standard matmul with unrolling, parallelizes on M. Unrolls 2 rows of A, 1 column of B.
 * @param void_args pointer to a matMul_args structure (please refer to this to setup the args)
 */
void mm_M_unroll_2x1(
    void * void_args, int id
);
