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



#include "peakDetection.h"
#include "relativeEnergy.h"
#include <stdint.h>
#include "defines.h"

#define ABS(N) ((N<0)?(-N):(N))

#include "pmsis.h"

//Variables used to detect peaks
PI_L1 uint32_t lastPeakIndex= 0;
PI_L1 int16_t lastPeakAmplitude = 0;
PI_L1 uint32_t numberOfAnalyzedWindows = 0;
PI_L1 uint8_t lastPeakWasIncomplete = 0;
PI_L1 uint16_t lastPeakWidth = INT16_MAX;

PI_L1 int16_t thr[N*2][N];

PI_L1 uint16_t startnext = 0;
PI_L1 int16_t width_compl[MAX_BEATS_PER_MIN*8];

void maxAndMinandMean(int16_t* ecg, int16_t* max, int16_t* min, int32_t* avg) {
	*max = ecg[0];
	*min = ecg[0];
	for (int16_t i = 1; i < BUFFER_SIZE; i++) {
		*avg += ecg[i];
		if (ecg[i] > *max) {
			*max = ecg[i];
			continue;
		}
		if (ecg[i] < *min) {
			*min = ecg[i];
		}
	}

	*avg = *avg / (BUFFER_SIZE);
	
}

void maxAndMin(int16_t* ecg, int16_t* max, int16_t* min) {
	*max = ecg[0];
	*min = ecg[0];
	for (int16_t i = 1; i < BUFFER_SIZE; i++) {
		if (ecg[i] > *max) {
			*max = ecg[i];
			continue;
		}
		if (ecg[i] < *min) {
			*min = ecg[i];
		}
	}

}

//Get the index of the maximum value within a certain range of ecg values in the window
uint32_t getIndexOfMaxInRange(int16_t* ecg, uint16_t startIndex, uint16_t stopIndex) {
	uint32_t result = 0;
	int16_t maxEcg = -9999;
	for (int16_t i = startIndex; i < stopIndex; i++) {
		if (ecg[i] > maxEcg) {
			maxEcg = ecg[i];
			result = i;
		}
	}
	return result;
}

uint32_t getIndexOfMinInRange(int16_t* ecg, uint16_t startIndex, uint16_t stopIndex) {
	uint32_t result = 0;
	int16_t minEcg = INT16_MAX;
	for (int16_t i = startIndex; i < stopIndex; i++) {
		if (ecg[i] < minEcg) {
			minEcg = ecg[i];
			result = i;
		}
	}
	return result;
}

void resetPeakDetection() {
	lastPeakIndex = 0;
	lastPeakAmplitude = 0;
	numberOfAnalyzedWindows = 0;
	lastPeakWasIncomplete = 0;
	lastPeakWidth = INT16_MAX;
}

void getPeakIndicesThroughHysteresisComparator(int16_t* ecgWindow, uint32_t* lastPeakIndex, int16_t* lastPeakAmplitude, uint32_t numberOfAnalyzedWindows, uint8_t numberOfFoundPeaks, uint32_t *output, int16_t* width_check) {
 
	numberOfFoundPeaks = 0;
	uint8_t thisWindowIsIncomplete = 0;
	uint16_t end = BUFFER_SIZE;

	uint32_t tempOutput[TEMPORARY_PEAK_BUFFER_SIZE]; //temporary indices of all found peaks before the false peaks are removed
	int16_t peakAmplitudes[TEMPORARY_PEAK_BUFFER_SIZE]; //amplitudes of found peaks, used to detect false positives
	int16_t peakWidths[TEMPORARY_PEAK_BUFFER_SIZE];

	//Index and amplitude of the last peak of the previous window
	uint32_t lastPeakIndexTemp = *lastPeakIndex;
	int16_t lastPeakAmplitudeTemp = *lastPeakAmplitude;

	//Generate the hysteresis thresholds
	int32_t avg = 0;
	int16_t max = 0;
	int16_t min = 0;
	maxAndMinandMean(ecgWindow, &max, &min, &avg);

	//Initializing output buffer
	for (int i = 0;i < MAX_BEATS_PER_MIN;i++)
		tempOutput[i] = 0;

	uint16_t numberOfAnalyzedPeaks = 0; //# of detected peaks by the algorithm

	if ( (ABS(max - avg)) < (TH_NEGATIVE_PEAK*ABS(min - avg))/100) { //Negative peak

		int16_t lowerThreshold = avg - (BIG_THRESHOLD * ABS(min - avg))/100;
		int16_t upperThreshold = avg - (SMALL_THRESHOLD * ABS(min - avg))/100;

#ifdef PRINT_THR 
		thr[pi_core_id()*2][pi_core_id()]=lowerThreshold;
		thr[pi_core_id()*2 + 1][pi_core_id()]=upperThreshold;
#endif	 
		
		uint8_t analyzingPeak = 0; //boolean indicating if a peak is being analyzed
		uint16_t startIndex = 0; //start index of analyzed peak
		uint16_t endIndex = 0; //end index of analyzed peak


		//Loop through Xecg
		for (uint16_t i = 0; i < end; i++) {
 
			if (ecgWindow[i] < lowerThreshold && analyzingPeak == 0) { //beginning of a peak
				startIndex = i;
				analyzingPeak = 1;
				end = BUFFER_SIZE;
			}
			else if (analyzingPeak == 1 && ecgWindow[i] > upperThreshold) { //end of a peak
				endIndex = i;
				analyzingPeak = 0;

				//There is a previous peak that needs to be finished
				if (startIndex < 0.03*ECG_SAMPLING_FREQUENCY && lastPeakWasIncomplete == 1 && numberOfAnalyzedPeaks == 0) {
					uint32_t minIndex = getIndexOfMinInRange(ecgWindow, startIndex, endIndex);
					uint16_t peakWidth = lastPeakWidth + endIndex - startIndex;

					if (ecgWindow[minIndex] < lastPeakAmplitudeTemp) { //this peak is "more negative" than the previous one
						tempOutput[numberOfAnalyzedPeaks] = minIndex + numberOfAnalyzedWindows * BUFFER_SIZE + LONG_WINDOW / 2 + 1;
						peakAmplitudes[numberOfAnalyzedPeaks] = ecgWindow[minIndex];
					}
					else { //last peak amplitude was "more negative"
						tempOutput[numberOfAnalyzedPeaks] = lastPeakIndexTemp;
						peakAmplitudes[numberOfAnalyzedPeaks] = lastPeakAmplitudeTemp;
					}
					peakWidths[numberOfAnalyzedPeaks] = peakWidth;
					numberOfAnalyzedPeaks++;

				}//The previous peak finished at the boundary and its value needs to be recorded
				else if (lastPeakWasIncomplete == 1 && numberOfAnalyzedPeaks == 0) {
					//Save previous peak
					tempOutput[numberOfAnalyzedPeaks] = lastPeakIndexTemp;
					peakAmplitudes[numberOfAnalyzedPeaks] = lastPeakAmplitudeTemp;
					peakWidths[numberOfAnalyzedPeaks] = lastPeakWidth;
					numberOfAnalyzedPeaks++;

					//Save new peak
					uint32_t minIndex = getIndexOfMinInRange(ecgWindow, startIndex, endIndex);
					uint16_t peakWidth = endIndex - startIndex;
					tempOutput[numberOfAnalyzedPeaks] = minIndex + numberOfAnalyzedWindows * BUFFER_SIZE + LONG_WINDOW / 2 + 1;
					peakAmplitudes[numberOfAnalyzedPeaks] = ecgWindow[minIndex];
					peakWidths[numberOfAnalyzedPeaks] = peakWidth;
					numberOfAnalyzedPeaks++;
				}
				else {//We do not need to care about previous peaks
					uint32_t minIndex = getIndexOfMinInRange(ecgWindow, startIndex, endIndex);
					uint16_t peakWidth = endIndex - startIndex;
					tempOutput[numberOfAnalyzedPeaks] = minIndex + numberOfAnalyzedWindows * BUFFER_SIZE + LONG_WINDOW / 2 + 1;
					peakAmplitudes[numberOfAnalyzedPeaks] = ecgWindow[minIndex];
					peakWidths[numberOfAnalyzedPeaks] = peakWidth;
					numberOfAnalyzedPeaks++;
				}
				end = BUFFER_SIZE;

			}
			else if (analyzingPeak == 1 && i == BUFFER_SIZE - 1) { //peak is being analyzed when window ends
				uint32_t minIndex = getIndexOfMinInRange(ecgWindow, startIndex, BUFFER_SIZE - 1);
				*lastPeakAmplitude = ecgWindow[minIndex];
				*lastPeakIndex = minIndex + numberOfAnalyzedWindows * BUFFER_SIZE + LONG_WINDOW / 2 + 1;
				lastPeakWidth = BUFFER_SIZE - 1 - startIndex;
				thisWindowIsIncomplete = 1;
				if(pi_core_id()<NUM_CORES-1){
					end = BUFFER_SIZE + 50;
					thisWindowIsIncomplete = 0;
				}	

			}
		}
	}
	else { //Positive peak
		int16_t lowerThreshold = avg + (SMALL_THRESHOLD * (max - avg))/100;
		int16_t upperThreshold = avg + (BIG_THRESHOLD * (max - avg))/100;
	 
#ifdef PRINT_THR 
		thr[pi_core_id()*2][pi_core_id()]=lowerThreshold;
		thr[pi_core_id()*2 + 1][pi_core_id()]=upperThreshold;
#endif	

		uint8_t analyzingPeak = 0; //boolean indicating if a peak is being analyzed
		uint16_t startIndex = 0; //start index of analyzed peak
		uint16_t endIndex = 0; //end index of analyzed peak
		//Loop through Xecg
		for (uint16_t i = 0; i < end; i++) {

			if (ecgWindow[i] >= upperThreshold && analyzingPeak == 0) { //beginning of a peak
				startIndex = i;
				analyzingPeak = 1;
				end = BUFFER_SIZE;

			}
			else if (analyzingPeak == 1 && ecgWindow[i] <= lowerThreshold) { //end of a peak
				endIndex = i;
				analyzingPeak = 0;

				//There is a previous peak that needs to be finished
				if (startIndex < 0.03*ECG_SAMPLING_FREQUENCY && lastPeakWasIncomplete == 1 && numberOfAnalyzedPeaks == 0) {
					uint32_t maxIndex = getIndexOfMaxInRange(ecgWindow, startIndex, endIndex);
					uint16_t peakWidth = lastPeakWidth + endIndex - startIndex;

					if (ecgWindow[maxIndex] > lastPeakAmplitudeTemp) { //this peak is greater than the previous one
						tempOutput[numberOfAnalyzedPeaks] = maxIndex + numberOfAnalyzedWindows * BUFFER_SIZE + LONG_WINDOW / 2 + 1;
						peakAmplitudes[numberOfAnalyzedPeaks] = ecgWindow[maxIndex];
					}
					else { //last peak amplitude was greater
						tempOutput[numberOfAnalyzedPeaks] = lastPeakIndexTemp;
						peakAmplitudes[numberOfAnalyzedPeaks] = lastPeakAmplitudeTemp;
					}
					peakWidths[numberOfAnalyzedPeaks] = peakWidth;
					numberOfAnalyzedPeaks++;
					lastPeakWidth = peakWidth;

				}//The previous peak finished at the boundary and its value needs to be recorded
				else if (lastPeakWasIncomplete == 1 && numberOfAnalyzedPeaks == 0) {
					//Save previous peak
					tempOutput[numberOfAnalyzedPeaks] = lastPeakIndexTemp;
					peakAmplitudes[numberOfAnalyzedPeaks] = lastPeakAmplitudeTemp;
					peakWidths[numberOfAnalyzedPeaks] = lastPeakWidth;
					numberOfAnalyzedPeaks++;

					//Save new peak
					uint32_t maxIndex = getIndexOfMaxInRange(ecgWindow, startIndex, endIndex);
					uint16_t peakWidth = endIndex - startIndex;
					tempOutput[numberOfAnalyzedPeaks] = maxIndex + numberOfAnalyzedWindows * BUFFER_SIZE + LONG_WINDOW / 2 + 1;
					peakAmplitudes[numberOfAnalyzedPeaks] = ecgWindow[maxIndex];
					peakWidths[numberOfAnalyzedPeaks] = peakWidth;
					numberOfAnalyzedPeaks++;
				}
				else {//We do not need to care about previous peaks
					uint32_t maxIndex = getIndexOfMaxInRange(ecgWindow, startIndex, endIndex);
					uint16_t peakWidth = endIndex - startIndex;
					tempOutput[numberOfAnalyzedPeaks] = maxIndex + numberOfAnalyzedWindows * BUFFER_SIZE + LONG_WINDOW / 2 + 1;
					peakAmplitudes[numberOfAnalyzedPeaks] = ecgWindow[maxIndex];
					peakWidths[numberOfAnalyzedPeaks] = peakWidth;
					numberOfAnalyzedPeaks++;
				}

				end = BUFFER_SIZE;

			}
			else if (analyzingPeak == 1 && i == BUFFER_SIZE - 1) { //peak is being analyzed when window ends
				uint32_t maxIndex = getIndexOfMaxInRange(ecgWindow, startIndex, BUFFER_SIZE - 1);
				*lastPeakAmplitude = ecgWindow[maxIndex];
				*lastPeakIndex = maxIndex + numberOfAnalyzedWindows * BUFFER_SIZE + LONG_WINDOW / 2 + 1;
				lastPeakWidth = BUFFER_SIZE - 1 - startIndex;
				thisWindowIsIncomplete = 1;
				if(pi_core_id()<NUM_CORES-1){
					end = BUFFER_SIZE + 50;
					thisWindowIsIncomplete = 0;
				}
			}
		}
	}

	//Loop through output to make sure peaks are far enough away from one another
	if (numberOfAnalyzedPeaks > 1) {
		for (int16_t j = (numberOfAnalyzedPeaks - 1); j > 0; j--) {

			int16_t diff_peaks = tempOutput[j] - tempOutput[j - 1];

			if (diff_peaks >= NORMAL_PEAKS_DISTANCE) { //peaks are more than 0.5 seconds apart
				//All is well
			}
			else {
				if (diff_peaks > MIN_PEAKS_DISTANCE && peakWidths[j] != INT16_MIN && peakAmplitudes[j] != 0) { //select the peak with the lowest width
					int16_t peakWidthRatio = (peakWidths[j - 1] * 100) / peakWidths[j];

					if (peakWidthRatio <= TH_LOW_WIDTH_RATIO || peakWidthRatio >= TH_HIGH_WIDTH_RATIO) { // peaks are not in the same range
						if (peakWidths[j] < peakWidths[j - 1]) {
							tempOutput[j - 1] = tempOutput[j];
							tempOutput[j] = 0;
							peakWidths[j - 1] = peakWidths[j];
							peakWidths[j] = 0;
						}
						else {
							tempOutput[j] = 0;
							peakWidths[j] = 0;
						}
					}
				}
				else {
					int16_t peakWidthRatio = (peakWidths[j - 1] * 100) / peakWidths[j];
					if(peakWidthRatio >= TH_HIGH_WIDTH_RATIO){
						tempOutput[j - 1] = tempOutput[j];
						tempOutput[j] = 0;
						peakWidths[j - 1] = peakWidths[j];
						peakWidths[j] = 0;
					}else{
						tempOutput[j] = 0;
						peakWidths[j] = 0;
					}
				}
			}
		}
	}
 
	//Load temporary result buffer into final output buffer
	int numberOfActualPeaks = 0;
	for (int m = 0; m <= numberOfAnalyzedPeaks; m++) {

		if (tempOutput[m] > 0) {
			numberOfFoundPeaks = numberOfFoundPeaks + 1; //get rid of redundant variable
			output[numberOfActualPeaks] = tempOutput[m];
			width_check[numberOfActualPeaks] = peakWidths[m];
			numberOfActualPeaks++;
		}
	}

	lastPeakWasIncomplete = thisWindowIsIncomplete;
}

void getPeaks_w(int32_t *arg[]){

	int16_t* xRE = (int16_t*) arg[1];
	uint32_t* indicesRpeaks = (uint32_t*) arg[2];
	uint32_t outputSingleBuff[N/NUM_CORES][MAX_BEATS_PER_MIN];
	int16_t width_check[N/NUM_CORES][MAX_BEATS_PER_MIN];

	// This should be set to 0 every time we delineate 8 beats
	startnext = 0;

	int id = pi_core_id();

	int idx;
	int chunk = N/NUM_CORES;
	int offset = id * chunk;

	for (idx = 0; idx < chunk; idx++)	{
		uint8_t numberOfDetectedPeaks = 0;

		for(int i=0; i<MAX_BEATS_PER_MIN; i++){ 
			outputSingleBuff[idx][i] = 0;
		}

		getPeakIndicesThroughHysteresisComparator(&xRE[LONG_WINDOW + (offset + idx)*(BUFFER_SIZE)], &lastPeakIndex, &lastPeakAmplitude, (offset + idx), &numberOfDetectedPeaks,
												 outputSingleBuff[idx], width_check[idx]);
	}

	for(int i = 0; i < N; i++){
		if (i >= offset && i < offset+chunk)	{
			for(int m=0; m<MAX_BEATS_PER_MIN; m++){
				if(i == 0 && outputSingleBuff[i-offset][m] > 0) {

					indicesRpeaks[startnext] = outputSingleBuff[i-offset][m];
					width_compl[startnext] = width_check[i-offset][m];
					startnext++; 

				} else if(i > 0 && m > 0 && outputSingleBuff[i-offset][m] > 0) {

					indicesRpeaks[startnext] = outputSingleBuff[i-offset][m];
					width_compl[startnext] = width_check[i-offset][m]; 
					startnext++; 
				 
				} else if(i > 0 && m == 0 && outputSingleBuff[i-offset][m] > 0) {

					indicesRpeaks[startnext] = outputSingleBuff[i-offset][m];
					width_compl[startnext] = width_check[i-offset][m]; 
					if (indicesRpeaks[startnext] - indicesRpeaks[startnext-1] >= NORMAL_PEAKS_DISTANCE) { //peaks are more than 0.5 seconds apart
						startnext++; 
		 			} else {
						if (indicesRpeaks[startnext] - indicesRpeaks[startnext - 1] > MIN_PEAKS_DISTANCE && width_compl[startnext] != INT16_MIN) { //select the peak with the lowest width
							int16_t peakWidthRatio = (width_compl[startnext-1] * 100) / width_compl[startnext];
							if (peakWidthRatio < TH_LOW_WIDTH_RATIO || peakWidthRatio > TH_HIGH_WIDTH_RATIO) { // peaks are not in the same range
								if (width_compl[startnext] < width_compl[startnext-1]) { 
									indicesRpeaks[startnext-1] = indicesRpeaks[startnext];
									indicesRpeaks[startnext] = 0;
									width_compl[startnext-1] = width_compl[startnext];
									width_compl[startnext] = 0;
								} else {
									indicesRpeaks[startnext] = 0;
									width_compl[startnext] = 0;
								}
							} else {
								startnext++; 
							}
						} else {
							indicesRpeaks[startnext] = 0;
							width_compl[startnext] = 0;
						}
					}
				}
			}
		}
		pi_cl_team_barrier();
	}

#ifdef PRINT_RELEN_INSIDE
	if(id==0){
		for(int ix=0; ix < BUFFER_SIZE * 1 + LONG_WINDOW; ix++)
			printf("%d\n", xRE[ix]);
	}	
#endif

	pi_cl_team_barrier();
}
