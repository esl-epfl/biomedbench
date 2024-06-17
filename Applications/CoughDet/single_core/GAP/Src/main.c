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




#include <stdlib.h>
// #include <errno.h>

#include "launcher.h"
#include "pmsis.h"

#ifdef GAP9
#include "GAP9/peripherals_sleep.h"
#endif

#ifdef GAPUINO
#include "GAPUINO/gpio_mng.h"
#endif

int main()
{

  #ifdef GAP9
  setup_frequency_voltage(240000000, 650);
  printf("Geia\n");       // Need this as a delay to get correct measurements (correct voltage/frequency setting)
  #endif
  #if defined(GAP9) || defined(GAPUINO)
  gpio_ini();
  gpio_enable();
  #endif

  #ifdef PROFILING_ON
  profile_start();
  #endif

  launch();
  
  #ifdef PROFILING_ON
  profile_stop();
  #endif

  #if defined(GAP9) || defined(GAPUINO)
  gpio_disable();
  #endif

  return 0;
}