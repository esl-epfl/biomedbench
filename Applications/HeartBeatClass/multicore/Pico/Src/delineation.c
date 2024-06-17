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



#include <stdio.h>
#include <stdlib.h>

#include "delineation.h"
#include "Pico_management/multicore.h"

//Fiducial point code
uint16_t fiducial_point_code[FPSIZE] = {6,7,8,9,1,2,3,4,5};
uint16_t type_wave[FPSIZE]={0,2,2,2,1,1,1,0,0};
uint16_t type_point[FPSIZE] = {2,0,1,2,0,1,2,0,1,};
int16_t flagOnsetOffset[9] = {1,-1,0,-1,-1,-1,1,-1,0};

//Delineated beat
uint32_t delineatedRR[FPSIZE*H_B];
sync_del = 0;		//used for dynamic/guided sync of cores (chunk = 1)

//  +---------------------------------------------------+
//  |               General helper functions            |
//  +---------------------------------------------------+

void initDelineationArray(){
    for(int i=0; i<FPSIZE;i++)
        delineatedRR[i] = 0;
}

//max peak index (Attention: end is included)
cntType mmaxpeak(int16_t* vec, cntType start, cntType end){
    if(end<start || start<0)
        return -1;
    int i;
    cntType maxpeakI=start;
    int16_t maxpeakV = 0;

    for(i=start+1;i<end;i++){
        if(vec[i]>maxpeakV && vec[i]>=vec[i+1] && vec[i]>=vec[i-1]){
            maxpeakI=i;
            maxpeakV = vec[i];
        }
    }

    return maxpeakI;
}

//mmin index (Attention: end is included)
cntType mmin(int16_t* vec, cntType start, cntType end){
    if(end<start || start<0)
        return -1;

    int i;
    cntType minI=start;
    for(i=start+1;i<=end;i++){
        if(vec[i]<vec[minI]){
            minI=i;
        }
    }
    return minI;
}

//mean (Attention: end is included)
dType mean(int16_t *vec, cntType start, cntType end){
    if(end<start)
        return 0;

    dType sum=0;
    cntType i;
    for(i=start;i<=end;i++){
#ifdef MUL
        sum+= (dType) vec[i]*SCALE;
#else 
        sum+= (dType) (vec[i]<<SCALE);
#endif
    }
    return sum/(end-start+1);
}


//  +---------------------------------------------------+
//  |               DELINEATION FUNCTIONS               |
//  +---------------------------------------------------+

//isoelectric line
dType isoelectric_line(int16_t* final_sig,dType fs, cntType final_sigLength, cntType tpeak, cntType ppeak){
    dType isoline;

    if(ppeak==tpeak || ppeak-tpeak<SAMPLES_AFTER_T+SAMPLES_BEFORE_P){
        //twave is in pwave
        if(tpeak<= MAX_T_IN_P)
            isoline=0;
        else
           isoline=mean(final_sig, tpeak - MAX_T_IN_P, tpeak - MIN_DIST_ISO_IN_P);
            // isoline=mean(final_sig, tpeak-28, tpeak-23);
    }
    else{
        if(tpeak+SAMPLES_AFTER_T>ppeak-SAMPLES_BEFORE_P)
            isoline=mean(final_sig, tpeak,ppeak);
        else
            isoline=mean(final_sig, tpeak+SAMPLES_AFTER_T, ppeak-SAMPLES_BEFORE_P);
    }
    return isoline;
}

cntType min_eucldist_isotriangle(dType* halfwave, cntType sizehalfwave,cntType peakRR,short flagOnOff){

    dType peak = halfwave[0];
    dType eucldistNosqrt[HALFWAVESIZE+1];
    for(int i=0;i<HALFWAVESIZE+1;i++)
        eucldistNosqrt[i] = 0;

    cntType indminED = 1;

    for(int j=1;j<=sizehalfwave;j++){
        dType sumSquares = 0;
        for(int k=0;k<sizehalfwave;k++){
            if(k<j)
                sumSquares+=(halfwave[k] - (-(peak/j)*k+peak) )*(halfwave[k]- (-(peak/j)*k+peak ));
            else
                sumSquares+=(halfwave[k])*(halfwave[k]);
        }
        eucldistNosqrt[j-1] = sumSquares;
        if(j>1){ 
            if(eucldistNosqrt[indminED]<eucldistNosqrt[j-2])
                indminED = j;
            else
                break;
        }
    }

    if(flagOnOff==0)
        return peakRR+indminED;
    else
        return peakRR-indminED;

}

//Delineate rstpq based on feature requested
int16_t delineate_rstpq(int16_t* sig,cntType sigLength,dType fs,uint16_t fp,uint16_t timeamp){

    // Range for fiducial points search
    cntType two_third= 2*sigLength/3;


    cntType point=1,R=1;
    cntType Smax=sigLength;
    if(Smax>R+MAX_HALF_QRS)
        Smax=R+MAX_HALF_QRS;

    cntType Qmin=1;
    if(Qmin<sigLength-MAX_HALF_QRS)
        Qmin=sigLength-MAX_HALF_QRS;


    if(fp==2)
        point=mmaxpeak(sig,two_third+1,Qmin-2); //P peak
    else if(fp==4)
        point=mmin(sig,Qmin,sigLength-1); //Q
    else if(fp==6)
        point=mmin(sig,2,Smax); //S
    else if(fp==8)
        point=mmaxpeak(sig,Smax,two_third); //T peak


    if(timeamp == 1)
        return point; //feature time
    else
        return sig[point]; //feature amplitude

}
    

int16_t delineate_onset_offset(int16_t* sig,cntType sigLength,dType fs,uint16_t fp,uint16_t timeamp,cntType* ptpeaks,dType isoline){
    
    cntType j;
   
    // cntType qrshalf_dur=10;//(cntType)round(fs*0.08/2);
    // cntType pwavehalf_dur=13;//(cntType)round(fs*0.10/2);
    // cntType twavehalf_dur=24;//(cntType)round(fs*0.19/2);

    cntType peak=-1,ppeak=-1,tpeak=-1;
    cntType wavehalf_dur = 0;
    cntType point=0;

    ppeak = ptpeaks[0];
    tpeak = ptpeaks[1];


    if(fp<5){

        peak = ppeak;
        wavehalf_dur = PWAVE_HALF_DUR;
        if(flagOnsetOffset[fp-1]==0){
            //Offset
            if(peak + wavehalf_dur>sigLength-QRS_HALF_DUR)
                wavehalf_dur = sigLength-QRS_HALF_DUR-1-peak;
        }else{
            //Onset

            if(ppeak-tpeak<PWAVE_HALF_DUR*2)
                wavehalf_dur=(cntType) (PWAVE_HALF_DUR/2)-2;
            else{
                if(peak - wavehalf_dur<tpeak+SAMPLES_AFTER_T)
                    wavehalf_dur = peak - tpeak+wavehalf_dur;
            }
        }


    }else{

        peak = tpeak;
        wavehalf_dur = TWAVE_HALF_DUR;

        if(flagOnsetOffset[fp-1]==0){
            //Offset

            if(ppeak-tpeak<PWAVE_HALF_DUR*2)
                wavehalf_dur=(cntType) (PWAVE_HALF_DUR/2)-2;
            else{
                if(peak + wavehalf_dur>ppeak-SAMPLES_BEFORE_P)
                    wavehalf_dur = ppeak-SAMPLES_BEFORE_P-peak;
            }
        }else{
            //Onset
            if(peak - wavehalf_dur<QRS_HALF_DUR)
                wavehalf_dur = peak - wavehalf_dur;
        }
    }
    // This may happen when t and p are too close to each other because of noisy signal
    if(wavehalf_dur<=0)
        wavehalf_dur=2;


    dType halfwave[HALFWAVESIZE+1];
    for(int i=0;i<HALFWAVESIZE+1;i++)
        halfwave[i] = 0;
    cntType stopHalfwave = 0;


    if(flagOnsetOffset[fp-1]==0){
        //Offset

        if(HALFWAVESIZE<(sigLength-peak-10)){
            if(wavehalf_dur<HALFWAVESIZE)
                stopHalfwave = peak+wavehalf_dur+1;
            else
                stopHalfwave = peak+HALFWAVESIZE+1;
        }else{
            stopHalfwave = sigLength-10;
        }
        //find pbon-off with min distance
        cntType maxintHalfwave = 0;
        for(j=peak+1;j<=stopHalfwave;j++){

            if(j==stopHalfwave)
                maxintHalfwave = j-peak;
#ifdef MUL
            halfwave[j-peak-1] = (dType) sig[j-1]*SCALE-isoline;   
#else
            halfwave[j-peak-1] = (dType) ((sig[j-1]<<SCALE)-isoline); 
#endif
                //printf("%d:%d\n", (sig[j+1]<<5)-isoline, sig[j+1]*32-isoline);
        }

        // find onset and offset points
        point=min_eucldist_isotriangle(halfwave, maxintHalfwave+1,peak,flagOnsetOffset[fp-1]);

        // error checking
        if(point<=0)
            point=peak+wavehalf_dur;


    }else{
        //Onset
        if(HALFWAVESIZE<peak){
            if(wavehalf_dur<HALFWAVESIZE)
                stopHalfwave = peak-wavehalf_dur-1;
            else
                stopHalfwave = peak-HALFWAVESIZE-1;
        }else{
            stopHalfwave = 1;
        }

        cntType maxintHalfwave = 0;
        for(j=peak-1;j>=stopHalfwave;j--){
            if(j==stopHalfwave)
                maxintHalfwave = -j+peak;
#ifdef MUL
            halfwave[-j+peak-1] = (dType) sig[j+1]*SCALE-isoline;
#else
            halfwave[-j+peak-1] = (dType) ((sig[j-1]<<SCALE)-isoline);  
#endif
        }

        point=min_eucldist_isotriangle(halfwave, maxintHalfwave+1,peak,flagOnsetOffset[fp-1]);

        if(point<=0)
            point=ppeak-PWAVE_HALF_DUR;

    }

    if(timeamp == 1)
        return point; //feature time
    else
        return sig[point]; //feature amplitude
}





void optimized_feature_extraction(int16_t* ecgRR,int32_t sigLength, int fs, int16_t* indexesCodeCurrentPeak, int16_t startindexRR, int number_of_peaks){

     int16_t Ptime=0,Ttime=0;
    dType isoline=0;
    cntType ptpeaks[2];
	
	// ************** Dimitrios Samakovlis: precompute once for all cases ****************
	Ptime = delineate_rstpq(ecgRR,sigLength,fs,2,1);
	Ttime = delineate_rstpq(ecgRR,sigLength,fs,8,1);
	ptpeaks[0]= (cntType) Ptime;
	ptpeaks[1]=(cntType) Ttime;
	isoline = isoelectric_line(ecgRR,fs,sigLength,Ttime,Ptime);
	
	
    for(int32_t i=indexesCodeCurrentPeak[0]; i<=indexesCodeCurrentPeak[1];i++){

        uint16_t fp = fiducial_point_code[i];
        uint16_t timeamp = 1;
        int16_t feature = 0;
        

        switch(fp){
            case 1:
            case 3:
            case 7:
            case 9:
                //Compute onset/offset based on flagOnsetOffset[fp]. Use P peak as sizeof(ecgRR)-Ptime (to have the index from the start, but save it as related to the current peak)
                feature = delineate_onset_offset(ecgRR,sigLength,fs,fp,timeamp,ptpeaks,isoline);
                break;
            case 2:
            	// **************** Dimitrios Samakovlis: removed ifs , just assign ****************
                feature = Ptime;
                break;
            case 4:
            case 6:
                feature = delineate_rstpq(ecgRR,sigLength,fs,fp,timeamp);
                break;
            case 5:
                //Take RR
                feature = (int16_t) sigLength-1;
                break; 
            case 8:
            	// **************** Dimitrios Samakovlis: removed ifs , just assign ****************
                feature = Ttime;
                break;
            default:
//              printf("Error in reading fiducial point\n");
                break;
        }
        delineatedRR[number_of_peaks*FPSIZE+i] = (uint32_t)(feature+startindexRR);
    }
}

void delineateECG_w(int32_t *arg[], uint32_t id){

    int16_t* ecg_buffRR_w = (int16_t*) arg[0];
    int32_t* indicesRpeaks_w = arg[2];
    uint32_t startindexRR = 0;
    uint32_t stopRR = 0;
    int32_t *offset_del = arg[3];
    int32_t *rpeaks_counter = arg[4];
    uint32_t *complete_del = arg[5];
    uint32_t out = 0;

     /* ----- Dimitrios Samakovlis for dynamic - guided scheduling ----- */
    sync_del = NUMBER_OF_CORES;
    //rt_team_barrier();	//no sync (barrier) needed assuming each thread will statically work on first part - SHOULD BE ON for dynamic scheduling with chunk 1
    
    if(id < *rpeaks_counter-1) {
	register int rp = id;
	while (1)	{
	    out = indicesRpeaks_w[rp+1] - indicesRpeaks_w[rp];

	    startindexRR = indicesRpeaks_w[rp];
	    stopRR = out+1;

	    int16_t featureIndexCodePointer[2] = {0,FPSIZE-1};

	    optimized_feature_extraction(&ecg_buffRR_w[startindexRR],stopRR,ECG_SAMPLING_FREQUENCY, &featureIndexCodePointer, startindexRR, rp);
	    
	    for(int ix = rp * FPSIZE; ix < (rp+1) * FPSIZE; ix++)	{
            complete_del[ix] = delineatedRR[ix]+*offset_del; 
	    }
	    
	    // critical region for sync
	    pico_critical_enter();
	    rp = sync_del;	// index of next beat
	    sync_del += 1;	// increment counter
	    pico_critical_exit();
	    
	    if (rp >= *rpeaks_counter-1)
		goto end;
	    
	}
    }
    
    end:
    pico_barrier(id);
    return;
}

