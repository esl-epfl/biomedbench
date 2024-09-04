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
// Porting to X-Heep: Francesco Poluzzi             //
// Date:            September 2023                  //
//////////////////////////////////////////////////////



#include <stdio.h>

#include <randomForest.h>
#include "x-heep.h"
#include "launcher.h"
#include "w25q128jw.h"
#include "helpers.h"

void predict_c(float *feat, float *probs)	{

    int16_t current_n = 0;  // current node

    for(int16_t i=0; i<N_TREES; i++){
        
        current_n = 0;

        for(int16_t j=0; j<MAX_NODES; j++){
            uint32_t *source_flash = heep_get_flash_address_offset( (float *)weights + i*N_NODES + current_n );
            float read_weights;
            if(w25q128jw_read_standard(source_flash, &read_weights, sizeof(float))!=FLASH_OK)printf("Error reading from flash\n");

            #ifdef DEBUG_PRINTS 
                printf("Read weight [%d][%d]   ", i, current_n);
                print_float(read_weights,6);
                printf("\n");
                printf("Feature [%d]:", node_features[i][current_n]);
                print_float(feat[node_features[i][current_n]],6);
                printf("\n");
            #endif

            if(feat[node_features[i][current_n]] <= read_weights){
                current_n = children[i][current_n][0];
            } else {
                current_n = children[i][current_n][1];
            }

            if(current_n == CLASS_0){
                probs[0]++;
                break;
            }
            if(current_n == CLASS_1){
                probs[1]++;
                break;
            }
        }
    }
}
