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



#include <inttypes.h>
#include <GAP9/dma_transfer.h>

#include <pmsis.h>

// DMA transfer of signal from L2 to L1
void DmaTransfer_L2_to_L1(int32_t **arg)	{
    pi_cl_dma_cmd_t dmaCp;
    
    pi_cl_dma_cmd((unsigned int)arg[0], (unsigned int)arg[1], (int32_t)*arg[2], PI_CL_DMA_DIR_EXT2LOC, &dmaCp);

    // Wait for dma to finish
    pi_cl_dma_cmd_wait(&dmaCp);
}

// DMA transfer of signal from L1 (local) to L2 (external)
void DmaTransfer_L1_to_L2(int32_t **arg) {
    pi_cl_dma_cmd_t dmaCp;

    pi_cl_dma_cmd((unsigned int)arg[0], (unsigned int)arg[1], (int32_t)*arg[2], PI_CL_DMA_DIR_LOC2EXT, &dmaCp);

    // Wait for dma to finish
    pi_cl_dma_cmd_wait(&dmaCp);
}


PI_L2 struct pi_device cluster_dev;