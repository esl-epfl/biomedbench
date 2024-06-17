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



#include <stdint.h>

#define BI
void __attribute__ ((noinline)) shift16();
//#define GPIO
/*GPIO definitions*/
#define PAD_GPIO_NUMBER		9
#define GPIO_NUMBER			9

/*Other definitions*/
#define RESET_FLAG 			0
#define SET_FLAG			1


//FOR PARALLEL EXECUTION
#define STACK_SIZE      1024
#define MOUNT           1
#define UNMOUNT         0
#define CID             0


#if TARGET == 1
#define CL
#else
#define FC 
#endif
