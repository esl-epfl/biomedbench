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

#include "launcher.h"
#include "pmsis.h"
#include "profile.h"

#define PROFILE // for activate the clock cycles profiling

#ifdef GAP9
#include "GAP9/peripherals_sleep.h"
#endif

#ifdef GAPUINO
#include "GAPUINO/gpio_mng.h"
#endif

// variables to power on the cluster
PI_L2 struct pi_device cluster_dev;
PI_L2 struct pi_cluster_conf cl_conf;

void cluster_launch() {

#ifdef PROFILE
  profile_start();
#endif

  launch();
  
#ifdef PROFILE
  profile_stop();
#endif
}

int main()
{
  struct pi_cluster_task cl_task;

  // *** SETUP THE CLUSTER CONFIG AND ENABLE IT ***
  pi_cluster_conf_init(&cl_conf);
  cl_conf.id = 0;                /* Set cluster ID. */
                       // Enable the special icache for the master core
  #ifdef GAP9
  setup_frequency_voltage(240000000, 650);
  cl_conf.icache_conf = PI_CLUSTER_MASTER_CORE_ICACHE_ENABLE |   
                       // Enable the prefetch for all the cores, it's a 9bits mask (from bit 2 to bit 10), each bit correspond to 1 core
                       PI_CLUSTER_ICACHE_PREFETCH_ENABLE |      
                       // Enable the icache for all the cores
                       PI_CLUSTER_ICACHE_ENABLE;
  #endif

  #if defined(GAP9) || defined(GAPUINO)
  gpio_ini();
  gpio_enable();
  #endif

  /* Configure & open cluster. */
  pi_open_from_conf(&cluster_dev, &cl_conf);
  if (pi_cluster_open(&cluster_dev))
  {
    printf("Cluster open failed !\n");
    pmsis_exit(-1);
  }

  setup_frequency_voltage(240000000, 650);    // For the FC
  pi_freq_set(PI_FREQ_DOMAIN_CL, 240000000);  // For the cluster


  pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, cluster_launch, NULL));

  // Power down the cluster
  pi_cluster_close(&cluster_dev);

  #if defined(GAP9) || defined(GAPUINO)
  gpio_disable();
  #endif

  return 0;
}