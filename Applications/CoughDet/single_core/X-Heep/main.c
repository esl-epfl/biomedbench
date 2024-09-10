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


#include <stdlib.h>

#include "launcher.h"
#include "x-heep.h"
#include "w25q128jw.h"
#include "timer_sdk.h"

int main()
{
  #ifdef PRINT_CYCLES
    timer_init();
    timer_start();
  #endif

  if (w25q128jw_init(spi_flash) != FLASH_OK){
    printf("Error initializing the flash memory\n");
    return EXIT_FAILURE;
  } 
  launch();

  #ifdef PRINT_CYCLES
    uint32_t cycles = timer_stop(); 
    printf("Cycles: %d\n", cycles);
  #endif

  return 0;
}
