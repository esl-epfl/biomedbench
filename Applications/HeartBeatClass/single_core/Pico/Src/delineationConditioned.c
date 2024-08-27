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
#include "defines.h"
#include "morpho_filtering.h"
#include "rms.h"
#include "peakDetection.h"
#include "relativeEnergy.h"
#include "rp_classifier.h"


#ifdef PICO
#include "PICO/gpio_plus_sleep.h"
#endif


#include "data/signal_250_3leads.h"

#include <stdio.h>


#define N_WINDOWS (ECG_VECTOR_SIZE/dim)

int16_t ecg_buff[dim*(NLEADS+1)];
int32_t indicesRpeaks[H_B+1];
int32_t indicesBeatClasses[H_B+1];
uint32_t complete_del[H_B * FPSIZE];

int32_t *arg[12];

int32_t count_window;
int32_t overlap;
int32_t buffSize_MF_RMS;
int32_t flag_prevWindAB;
int32_t rWindow;


void clearRelEn() {
    clearAndResetRelEn();
    resetPeakDetection();
}

void delineateECG(){


#ifdef MODULE_MF_3L

        if(flag_prevWindAB==1) {
            arg[0] = (int32_t*) ecg_buff;
            buffSize_MF_RMS = dim;
        } else {
            arg[0] = (int32_t*) &ecg_buff[overlap];
            buffSize_MF_RMS = dim-overlap;
        }

        for(int32_t lead_abnbeat = 1; lead_abnbeat < NLEADS; lead_abnbeat++){
            arg[9] = &lead_abnbeat;
            filterWindows(arg);
        }

    #ifdef PRINT_SIG_MF_3L
        if(count_window==0) {
            int32_t lead_print = 1;
            for(int32_t sample = dim*lead_print; sample<dim*(lead_print+1); sample++) {
                printf("%d ", ecg_buff[sample]);
            }
            printf("\n");
        }
    #endif

#endif

#ifdef MODULE_RMS_3L

            arg[0] = (int32_t*) ecg_buff;
            arg[1] = (int32_t*) &ecg_buff[dim*NLEADS]; //Using last dim samples of the buffer to keep the first for the MF lead0
            buffSize_MF_RMS = dim;

            combine_leads(arg);

    #ifdef PRINT_SIG_RMS_3L
            if(count_window==0) {
                for(int32_t sample = dim*NLEADS; sample<dim*(NLEADS+1); sample++) {
                    printf("%d ", ecg_buff[sample]);
                }
                printf("\n");
            }
    #endif

#endif



#ifdef MODULE_DEL_3L

            arg[0] = (int32_t*) &ecg_buff[dim*NLEADS];

            delineateECG_w(arg);

#endif
}

void classifyBeatECG()  {

    int32_t count_sample = 0;
    int32_t firstDel = 0;
    int32_t offset_del = 0;
    int32_t rpeaks_counter = 0;
    int32_t tot_overlap = 0;
    int32_t offset_MF = 0;
    int32_t totP = 0;
    int32_t flagMF = 0;
    int32_t i_lead = 0;
    int32_t flag_abnBeat = 0;


    offset_MF = 150;    // For 250Hz sampling frequency

    buffSize_MF_RMS = dim;
    count_window = 0;
    overlap = 0;
    flag_prevWindAB = 0;

    arg[0] = (int32_t*) ecg_buff;
    arg[1] = (int32_t*) &ecg_buff[dim];
    arg[2] = indicesRpeaks;
    arg[3] = &offset_del;
    arg[4] = &rpeaks_counter;
    arg[5] = (int32_t*) complete_del;
    arg[6] = &totP;
    // arg[7] = &rWindow;
    arg[8] = &flagMF;
    arg[9] = &i_lead;
    arg[10] = &buffSize_MF_RMS;
    arg[11] = indicesBeatClasses;

    clearRelEn();

    for(rWindow=0; rWindow<N_WINDOWS; rWindow++)
    {
        if (firstDel == 1) {
            for(int32_t lead=0; lead<NLEADS; lead++) {
                for(int32_t i=0; i<overlap; i++) {
                    ecg_buff[i + dim*lead] = ecg_buff[(dim - overlap + i) + dim*lead]; //copy the overlap of the 3 leads
                }
            }
        }

        for(int32_t lead=0; lead<NLEADS; lead++) {
            for(int32_t i=overlap; i<dim; i++) {
                ecg_buff[i + dim*lead] = ecg_3l[rWindow*dim + i - tot_overlap][lead];
            }
        }

        if (firstDel == 0) {
            // Needed to initialize the MF filter properly
            for(int32_t lead=0; lead<NLEADS; lead++) {
                for(int32_t i=0; i<=offset_MF; i++) {
                    ecg_buff[i + dim*lead] = 0;
                }
            }
        }

#ifdef MODULE_MF
        arg[0] = (int32_t*) &ecg_buff[overlap];
        buffSize_MF_RMS = dim-overlap;

        arg[9]= &i_lead;
        filterWindows(arg);

    #ifdef PRINT_SIG_MF
        if(count_window==0){
            for(int32_t sample = 0; sample<dim; sample++) {
                printf("%d ", ecg_buff[sample]);
            }
            printf("\n");
        }
    #endif

#endif // MODULE_MF


#ifdef MODULE_RELEN
        clearAndResetRelEn();

        arg[0] = (int32_t*) ecg_buff; //keep the first dim samples for lead0 MF
        arg[1] = (int32_t*) &ecg_buff[dim*NLEADS];

        relEn_w(arg);

    #ifdef PRINT_RELEN
        if(count_window==0) {
            for(int32_t sample = dim*NLEADS; sample<dim*(NLEADS+1); sample++) {
                printf("%d ", ecg_buff[sample]);
            }
            printf("\n");
        }
    #endif

#endif // MODULE_RELEN

#ifdef MODULE_RPEAK

        getPeaks_w(arg);

        rpeaks_counter = 0;

        while(indicesRpeaks[rpeaks_counter]!=0) {
            rpeaks_counter++;
        }

    #ifdef PRINT_RPEAKS
        for(int32_t indR=0; indR<rpeaks_counter; indR++) {
            printf("%d ", (indicesRpeaks[indR] + offset_del));
        }
        printf("\n");
    #endif

#endif // MODULE_RPEAK


#ifdef MODULE_BEATCLASS

        report_rpeak(arg);
        for(int32_t indR=0; indR<rpeaks_counter; indR++) {
            if(indicesBeatClasses[indR]>0) {
                flag_abnBeat=1;
                break;
            }
        }

    #ifdef PRINT_BEATCLASS
        for(int32_t indR=0; indR<rpeaks_counter; indR++) {
          printf("%d %d\n", indicesRpeaks[indR], indicesBeatClasses[indR]);
        }
    #endif
    
    #ifdef PRINT_RESULT
        if(flag_abnBeat)
	    printf("Window %d: ABNORMAL_BEAT!\n", rWindow);
	else
	    printf("Window %d: BIEN!\n", rWindow);
    #endif

#endif // MODULE_BEATCLASS


#ifdef MODULE_3L

        if(flag_abnBeat==1) {
            delineateECG();
            flag_abnBeat = 0;
            flag_prevWindAB = 0;
        } else {
            flag_prevWindAB = 1;
        }

#endif

    #ifdef PRINT_DEL
            for(int32_t ix = 0; ix<(rpeaks_counter-1)*FPSIZE; ix++)
                printf("%d ", complete_del[ix]);
            printf("\n");
    #endif

#ifdef ONLY_FIRST_WINDOW
        return;
#endif

#ifdef ONLY_TWO_WINDOW
        if (rWindow == 1)
            return;
#endif

        overlap = dim - (indicesRpeaks[rpeaks_counter - 2] - LONG_WINDOW);


#ifdef DEBUG_FIRST_MODULES
        overlap = LONG_WINDOW+LONG_WINDOW;
#endif

        tot_overlap += overlap;
        offset_del = (count_window+1) * dim - tot_overlap;

        totP += rpeaks_counter-1;

        firstDel = 1;
        flagMF=1;
        count_window++;
        rpeaks_counter = 0;

        for(int32_t ix_rp = 0; ix_rp < H_B+1 ; ix_rp++) {
           indicesRpeaks[ix_rp] = 0;
        }

    }
}
