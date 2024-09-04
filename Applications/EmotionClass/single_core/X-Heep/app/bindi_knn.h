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


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Original app:    Bindi from Jose A. Miranda Calero,  Universidad Carlos III de Madrid                //
// Modified by:     Dimitrios Samakovlis, Embedded Systems Laboratory (ESL), EPFL                       //
                                                                //
// Date:            February 2024                                                                       //  
//////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _BINDI_KNN_H
#define _BINDI_KNN_H

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/
#include <stdint.h>

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

/*...*/
#define TRAINING_DATASET_SIZE          685
/*...*/
#define COST_MISSCLASIFICATION_FEAR    1.2
/*...*/
#define COST_MISSCLASIFICATION_NOFEAR  1

/* Maximum and Minimum BVP, GSR, TEMP */
//Max BVP: 163721  GSR: 13402  TEMP: 31.98
#define MAX_BVP                        163721
#define MAX_GSR                        13402
#define MAX_TEMP                       31.98
//Min BVP: 60075 GSR: 12872  TEMP: 30.6
#define MIN_BVP                        60075
#define MIN_GSR                        12872
#define MIN_TEMP                       30.6

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/* ... */

/* KNN Labels --> No Fear / Fear */
typedef enum knnState_t
{
  NO_FEAR,
  FEAR,
}knnState_t;

/* Datatype definition for a sample */
typedef struct
{
  float field_1;
  float field_2;
  float field_3;
  knnState_t      label;
} Knn_sample_definitionT;

typedef struct{
  /* Max value: 255 */
  uint8_t neighbour_size;
  /* Max value: 65535 */
  uint16_t training_size;
  uint32_t testing_size;
  /* Max value: 255 */
  uint8_t testing_mem_size;
  uint8_t coss_miss_fear;
  uint8_t coss_miss_nofear;
  /* NO_FEAR - '1' / FEAR - '0' */
  knnState_t knnState : 1;
  
}__attribute__((__packed__)) KnnT;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/
#ifndef _BINDI_KNN_C_SRC

extern Knn_sample_definitionT   knn_testing_dataset;

#endif  /* _BINDI_KNN_C_SRC */

/**
 * @brief runKNN. ...
 *
 * @param none.
 * 
 * @return none.
 */
knnState_t runKNN(void);

#endif /* _BINDI_KNN_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
