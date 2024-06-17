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



#include "delineationConditioned.h"
#include "profiling/profile.h"
#include "data/signal_250_3leads.h"
#include "defines.h"
#include "morpho_filtering.h"
#include "rms.h"
#include "peakDetection.h"
#include "relativeEnergy.h"
#include "rp_classifier.h"

#include "pmsis.h"

#define N_WINDOWS (ECG_VECTOR_SIZE/dim)


#define PI_CL_SLAVE_STACK_SIZE 2048

PI_L2 struct pi_device cluster_dev;
PI_L2 struct pi_cluster_conf cl_conf;
PI_L2 int32_t done = 0;
PI_L2 int32_t rOverlap;
PI_L2 int32_t rWindow = 0;
PI_L2 int16_t aL2EcgBuffer[ECG_VECTOR_SIZE*3];
PI_L2 int32_t *arg[12];
PI_L2 int32_t count_window;
PI_L2 int32_t flag_prevWindAB;
PI_L2 int32_t buffSize_MF_RMS;
PI_L2 int32_t flag_3l;

PI_L1 int32_t indicesRpeaks[H_B+1];
PI_L1 int32_t indicesBeatClasses[H_B+1];
PI_L1 uint32_t complete_del[H_B * FPSIZE];
PI_L1 int32_t rL2BufferIndex;
PI_L1 int16_t aL1EcgBuffer[dim*(NLEADS+1)];
PI_L1 int32_t rOffset_MF;
PI_L1 uint32_t heap_CL[16384];


static void prof_cluster_start(void *arg)
{
  pi_cl_team_fork(NUM_CORES, profile_start, arg);
}

static void prof_cluster_stop(void *arg)
{
  pi_cl_team_fork(NUM_CORES, profile_stop, arg);
}

static void cluster_relEn(int32_t *arg[])
{
  pi_cl_team_fork(NUM_CORES, relEn_w, arg);
}

static void cluster_getPeaks(int32_t *arg[])
{
  pi_cl_team_fork(NUM_CORES, getPeaks_w, arg);
}

static void cluster_report_rpeak(int32_t *arg[])
{
  pi_cl_team_fork(NUM_CORES, report_rpeak, arg);
}

static void cluster_delineateECG(int32_t *arg[])
{
  pi_cl_team_fork(NUM_CORES, delineateECG_w, arg);
}

static void cluster_combine_leads(int32_t *arg)
{
  pi_cl_team_fork(NUM_CORES, combine_leads, arg);
}

static void cluster_morph_nleads(int32_t *arg[])
{
  if (NUM_CORES != 1 )
    pi_cl_team_fork(NLEADS, filterWindows, arg);
  else
    pi_cl_team_fork(NUM_CORES, filterWindows, arg);
}

void clearRelEn() {
  clearAndResetRelEn();
  resetPeakDetection();
}

static void fCore0_DmaTransfer_Windows(void *arg)
{
  pi_cl_dma_cmd_t dmaCp;

  if(rWindow == 0)
  {
  for(int32_t rLead=0; rLead<NLEADS; rLead++) {
    for(int32_t rIndex=0; rIndex<=rOffset_MF; rIndex++) {

    // Set to 0 first part of each window in L1 memory
    aL1EcgBuffer[(dim*rLead) + rIndex] = 0;
    }
  }

  for(int32_t rLead=0; rLead<NLEADS; rLead++) {

    // Copy data block from L2 to L1 memory using the cluster DMA
    pi_cl_dma_cmd((unsigned int)&aL2EcgBuffer[(ECG_VECTOR_SIZE*rLead) + rL2BufferIndex + rOffset_MF + 1], (unsigned int)&aL1EcgBuffer[(dim*rLead) + rOffset_MF + 1], (dim - (rOffset_MF + 1))*2, PI_CL_DMA_DIR_EXT2LOC, &dmaCp);

    // Wait for dma to finish
    pi_cl_dma_cmd_wait(&dmaCp);
  }
  }
  else
  {
  for(int32_t i = 0; i < rOverlap; i++) {
    for(int32_t rLead=0; rLead<NLEADS; rLead++) {

    // Copy overlapping block from end to start of windows
    aL1EcgBuffer[(dim*rLead) + i] = aL1EcgBuffer[(dim - rOverlap + i) + dim*rLead];
    }
  }

  for(int32_t rLead=0; rLead<NLEADS; rLead++) {

    // Copy non-overlapping block from L2 to L1 memory using the cluster DMA
    pi_cl_dma_cmd((unsigned int)&aL2EcgBuffer[(ECG_VECTOR_SIZE*rLead) + rL2BufferIndex], (unsigned int)&aL1EcgBuffer[dim*rLead + rOverlap], (dim - rOverlap)*2, PI_CL_DMA_DIR_EXT2LOC, &dmaCp);

    // Wait for dma to finish
    pi_cl_dma_cmd_wait(&dmaCp);
  }
  }

  rL2BufferIndex = (rL2BufferIndex - rOverlap) + dim;
}


void delineateECG() {

#ifdef MODULE_RMS_3L
    struct pi_cluster_task cl_task;
    
  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
  #endif

    arg[0] = (int32_t*) aL1EcgBuffer;
    arg[1] = (int32_t*) &aL1EcgBuffer[dim*NLEADS];
    buffSize_MF_RMS = dim;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, cluster_combine_leads, arg));

  #ifdef HWPERF_MODULES
     pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
  #endif

  #ifdef PRINT_SIG_RMS_3L
      if(count_window==0) {
        for(int32_t sample = dim*NLEADS; sample<dim*(NLEADS+1); sample++) {
          printf("%d ", aL1EcgBuffer[sample]);
        }
        printf("\n");
      }
  #endif

#endif


#ifdef MODULE_DEL_3L

    arg[0] = (int32_t*) &aL1EcgBuffer[dim*NLEADS];

  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
  #endif
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, cluster_delineateECG, arg));

  #ifdef HWPERF_MODULES
  pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
  #endif

#endif
}

void classifyBeatECG() {
    
    struct pi_cluster_task cl_task;
    
    // *** SETUP THE CLUSTER CONFIG AND ENABLE IT ***
    pi_cluster_conf_init(&cl_conf);
    cl_conf.id = 0;                /* Set cluster ID. */
    #ifdef GAP9
                       // Enable the special icache for the master core
    cl_conf.icache_conf = PI_CLUSTER_MASTER_CORE_ICACHE_ENABLE |   
                       // Enable the prefetch for all the cores, it's a 9bits mask (from bit 2 to bit 10), each bit correspond to 1 core
                       PI_CLUSTER_ICACHE_PREFETCH_ENABLE |      
                       // Enable the icache for all the cores
                       PI_CLUSTER_ICACHE_ENABLE;
    #endif
    
    /* Configure & open cluster. */
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        printf("Cluster open failed !\n");
        pmsis_exit(-1);
    }

  #ifdef GAP9
  pi_freq_set(PI_FREQ_DOMAIN_CL, 240000000);
  #elif defined(GAPUINO)
  pi_freq_set(PI_FREQ_DOMAIN_CL, 87000000);
  #endif

  for(int i=0; i<ECG_VECTOR_SIZE; i++)
  {
    aL2EcgBuffer[i] = ecg_3l[i][0];
    aL2EcgBuffer[i+ECG_VECTOR_SIZE] = ecg_3l[i][1];
    aL2EcgBuffer[i+(2*ECG_VECTOR_SIZE)] = ecg_3l[i][2];
  }

  int32_t firstDel = 0;
  int32_t offset_del = 0;
  int32_t rpeaks_counter = 0;
  int32_t tot_overlap = 0;
  int32_t totP = 0;
  int32_t flagMF = 0;
  int32_t flag_abnBeat = 0;

  if(ECG_SAMPLING_FREQUENCY == 250) {
    rOffset_MF = 150;
  } else {
    rOffset_MF = 300;
  }

  buffSize_MF_RMS = dim;
  count_window = 0;
  rOverlap = 0;
  flag_3l = 0;
  flag_prevWindAB = 0;

  arg[0] = (int32_t*) aL1EcgBuffer;
  arg[1] = (int32_t*) &aL1EcgBuffer[dim];
  arg[2] = indicesRpeaks;
  arg[3] = &offset_del;
  arg[4] = &rpeaks_counter;
  arg[5] = (int32_t*) complete_del;
  arg[6] = &totP;
  arg[7] = NULL;
  arg[8] = &flagMF;
  arg[9] = &buffSize_MF_RMS;
  arg[10] = indicesBeatClasses;
  arg[11] = &flag_3l;

  clearRelEn();

  rL2BufferIndex = 0;
  rOverlap = 0;

  for(rWindow=0; rWindow < N_WINDOWS; rWindow++)
  {

#ifdef HWPERF_FULL
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif

    // ----------------------------Copy windows from L2 to L1 memory ------------------------------------------------- //
    // Run function on Core 0 of the cluster
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, fCore0_DmaTransfer_Windows, NULL));
    // ----------------------------------------------------------------------------------------------------------------//

#ifdef HWPERF_STEP1
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
#endif


#ifdef MODULE_MF_3L

  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
  #endif
    arg[0] = &aL1EcgBuffer[rOverlap];
    buffSize_MF_RMS = dim-rOverlap;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, cluster_morph_nleads, arg));
    
  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
  #endif

  #ifdef PRINT_SIG_MF_3L
    printf("\n!MORPHOLOGICAL FILTERING!\n");
    if(count_window==0) {
      for(int32_t lead_print = 0; lead_print < NLEADS; lead_print++) {
        for(int32_t sample = dim*lead_print; sample<dim*(lead_print+1); sample++) {
          printf("%d ", aL1EcgBuffer[sample]);
        }
        printf("\n");
      }
    }
  #endif

#endif

#ifdef MODULE_RELEN

  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
  #endif
    clearAndResetRelEn();

    arg[0] = (int32_t*) &aL1EcgBuffer;
    arg[1] = (int32_t*) &aL1EcgBuffer[dim*NLEADS];

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, cluster_relEn, arg));


  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
  #endif

  #ifdef PRINT_RELEN
    printf("\n!RELEN!\n");
  if(count_window==0) {
    for(int32_t sample = dim*NLEADS; sample<dim*(NLEADS+1); sample++) {
      printf("%d ", aL1EcgBuffer[sample]);
    }
    printf("\n");
  }
  #endif

#endif

#ifdef MODULE_RPEAK

  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
  #endif
    
    pi_cluster_task(&cl_task, cluster_getPeaks, arg);
    cl_task.slave_stack_size = 2048;
    pi_cluster_send_task_to_cl(&cluster_dev, &cl_task);

    rpeaks_counter = 0;

    while(indicesRpeaks[rpeaks_counter]!=0) {
       rpeaks_counter++;
    }

  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
  #endif

  #ifdef PRINT_RPEAKS
    printf("\n!R_PEAKS!\n");
    for(int32_t indR=0; indR<rpeaks_counter; indR++) {
      printf("%d ", (indicesRpeaks[indR] + offset_del));
    }
    printf("\n");
  #endif

#endif

#ifdef MODULE_BEATCLASS

  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
  #endif

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, cluster_report_rpeak, arg));

    for(int32_t indR=0; indR<rpeaks_counter; indR++) {
      if(indicesBeatClasses[indR]>0) {
        flag_abnBeat=1;
        break;
      }
    }

  #ifdef HWPERF_MODULES
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
  #endif

  #ifdef PRINT_BEATCLASS
    printf("\n!BEATCLASS!\n");
    for(int32_t indR=0; indR<rpeaks_counter; indR++) {
      printf("%d %d\n", indicesRpeaks[indR], indicesBeatClasses[indR]);
    }
  #endif

#endif


#ifdef HWPERF_STEP1
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif


#ifdef MODULE_STEP2

  #ifdef HWPERF_STEP2
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_start, NULL));
  #endif
    if(flag_abnBeat==1) {
      delineateECG();
      flag_abnBeat = 0;
    }
#endif


#ifdef HWPERF_STEP2
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif


#ifdef HWPERF_FULL
	pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, prof_cluster_stop, NULL));
#endif


#ifdef PRINT_DEL
    printf("\n!DELINEATION!\n");
    for(int32_t ix = 0; ix<(rpeaks_counter-1)*FPSIZE; ix++) printf("%d ", complete_del[ix]);
    printf("\n");
 #endif


#ifdef ONLY_FIRST_WINDOW 
    pi_cluster_close(&cluster_dev);
    return; 
#endif

#ifdef ONLY_TWO_WINDOW
    if (rWindow == 1)
        return;
#endif

    rOverlap = dim - (indicesRpeaks[rpeaks_counter - 2] - LONG_WINDOW);


#ifdef DEBUG_FIRST_MODULES
    rOverlap = LONG_WINDOW+LONG_WINDOW;
#endif


    tot_overlap += rOverlap;
    offset_del = (count_window+1) * dim - tot_overlap;

    totP += rpeaks_counter-1;

    if(firstDel==0) {
      firstDel = 1;
    }

    count_window++;
    rpeaks_counter = 0;

    for(int32_t ix_rp = 0; ix_rp < H_B+1 ; ix_rp++) {
      indicesRpeaks[ix_rp] = 0;
    }

    flagMF=1;
  }
  pi_cluster_close(&cluster_dev);
}
