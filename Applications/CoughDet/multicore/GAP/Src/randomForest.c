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

#include <randomForest.h>


void predict_c(float *feat, float *probs)  {
    
    // probs[0] = 15.0;
    // probs[0]++;

    int16_t current_n = 0;  // current node

    for(int16_t i=0; i<N_TREES; i++)  {
        
        current_n = 0;

        for(int16_t j=0; j<MAX_NODES; j++)  {
            if(feat[node_features[i][current_n]] <= weights[i][current_n])  {
                current_n = children[i][current_n][0];
            } else {
                current_n = children[i][current_n][1];
            }

            if(current_n == CLASS_0)  {
                probs[0]++;
                break;
            }
            if(current_n == CLASS_1)  {
                probs[1]++;
                break;
            }
        }
    }
}