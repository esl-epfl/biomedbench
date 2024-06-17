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



#include "rp_classifier.h"
#include "params.h"
#include "defines.h"

#include "pmsis.h"

// For dynamic loop scheduling
PI_L1 rpeak_sync;

/*
    polygmf:
This function estimatest the value of a gaussian distribution
by using 2 linear segments. Values that are more than 4*segment_size
(4.70 sigma) further from the center are approximated to 0. Values
that are between 4*segment_size and 2*segment_size from the center
are approximated to 1 (minimum value). The other 2 segments are 
approximated by two linear functions with slope and offset
*/

uint16_t polygmf(uint16_t x, uint16_t class, uint16_t coeff)
{    
    //Center of the gaussian
    uint16_t mu = center[class][coeff];
    //Segment size of the gaussian
    uint16_t segment_sz = segment_size[class][coeff];

    uint16_t temp, slope, offset;

    //Remap value to the left half of the gaussian
    if(x >= mu) {
        temp = x - mu + 1;
        if(temp > mu)
            return 0; //The value falls out of the x axis (return 0)
        else
            x = mu - temp;
    }

    //If x is further than 4*segment_sz then it is out of the gaussian
    temp = mu - (segment_sz<<2);
    if(x < temp)
        return 0;

    //If x is between 4*segment_sz and 2* segment_sz distance from the 
    //center then we return the minimum value (1)
    temp = mu - (segment_sz<<1);
    if(x < temp)
        return 1;

    //Choose between the 2 linear aproximations and return the 
    //corresponding value
    temp = mu - segment_sz;
    if(x < temp)
    {
        temp = temp - segment_sz;
        slope = slope_bottom[class][coeff];
        offset = OFFSET_BOTTOM;
    }
    else
    {
        slope = slope_top[class][coeff];
        offset = OFFSET_MIDDLE;
    }

    x = x - temp;
    return offset + (x*slope);
}
/*
    applyRnadomProjection
This funciton calculates the random projection coefficients of a given heart beat
or signal by doing a normal matrix multiplication of the signal by the random
projection matrix PI (filled with 0, 1 and -1). As the possible values of PI are
only 3, the N values of a given row (equal to the number of coefficients) are 
compressed in N/8 bits, in other words, 2 bits per value (00 represents 0, 01
represents 1 and 10 represents -1).
*/
void applyRandomProjection(uint16_t input[INPUT_FREQUENCY], uint16_t output[NUM_COEFFICIENTS])
{
    uint16_t c, i, s, temp;
    for(c=0; c<NUM_COEFFICIENTS; c++)
        output[c] = (1<<15) - 1; //Initialize to half of the range

    for(i=0; i<OUTPUT_FREQUENCY; i++)
    {    
        for(c=0; c<(NUM_COEFFICIENTS/COMPRESSED_ELEMENTS); c++)
        {
            temp = rp_mat[c][i];
            for(s=0;s<COMPRESSED_ELEMENTS;s++)
            {
                if(temp & 0x0001) //Represents +1
                    output[(c<<3) + s] += input[i];
                if(temp & 0x0002) //Represents -1
                    output[(c<<3) + s] -= input[i];
                temp = temp >> 2;
            }        
        }        
    }
}

/*
    rescaledProductory
This function uses the coeeficients coming frmo the random projection to
calculate the estimated value of the gaussian for every coefficient and for
every class. Later it calculates the productory of all the coefficients for each
class giving as an output a single value per class. The productory is carefully
performed in order to maintain the highest possible precission in every multipli-
cation since at the end we are only interested in the ratio between the values
of the different classes
*/
void rescaledProductory(uint16_t coeffs[NUM_COEFFICIENTS], uint16_t output[NUM_CLASSES])
{
    uint32_t gaussians[NUM_CLASSES], max;
    uint16_t c, i;
    
    //Initial value (polygonal estimation for the first coefficient)
    for(c=0;c<NUM_CLASSES;c++)
        output[c] = polygmf(coeffs[0], c, 0);

    
    for(i=1;i<NUM_COEFFICIENTS;i++)
    {
        //Calculate the polygonal estimation for the given coefficient and multiply it
        max = 0;
        for(c=0; c<NUM_CLASSES; c++)
        {
            gaussians[c] = output[c] * polygmf(coeffs[i], c, i);
            if(gaussians[c] > max)
                max = gaussians[c];    
        }

        //If the maximum is 0 we cannot rescale (all vaules are 0)
        if(max == 0)
            continue;

        //Shift numbers as much as possible to the right in 32 bit precission
        while(max < (1<<31))
        {
            max = 0;
            for(c=0; c<NUM_CLASSES; c++)
            {
                gaussians[c] = gaussians[c] << 1;
                if(gaussians[c] > max)
                    max = gaussians[c];    
            }
        }

        //Shift the 32 bit precission number 16 bits to the left to obtain the partial
        //result of the productory with highest precission possible
        for(c=0; c<NUM_CLASSES; c++)
        {
            output[c] = gaussians[c] >> 16;
        }
    }
}

/*
    classifyBeat\
This function receives as parameter the array of values calculated by the prodcutory of
the gaussians for every class. According to a predefined threshold it takes a decission of
whether a heart beat belongs to a class or to an unknown class. It returns NUMBER_CLASSES
in the case the beat is not clear to belong to a given class
*/

uint16_t classifyBeat(uint16_t values[NUM_CLASSES])
{
    uint16_t max = 0, max2 = 1, i, thr = 0;
    uint32_t temp;

    //Linear search for the 2 highest values
    for(i=1; i<NUM_CLASSES;i++)
    {
        if(values[i] >= values[max]) {
            max2 = max;
            max = i;
        }
        else {
            if(values[i] >= values[max2])
                max2 = i;
        }
    }
    //Calculate the threshold
    for(i=0;i<NUM_CLASSES;i++)
    {
        temp = (int32_t)values[i] * (int32_t)THR_FACTOR;
        thr += (temp >> 16);
    }

    if(values[max] - values[max2] >= thr)
        return max;
    else
        return NUM_CLASSES;
}    

/*
    normalizeAndResampleInput
This function resamples the input signal and normalize it into the range that the RP
expects. In order to do so the function assumes that the signal is centered in 0, in
other words, the basline and low frequency components have been removed from the signal
*/
void normalizeAndResampleInput(int16_t input[], uint16_t start, uint16_t mask, uint16_t output[OUTPUT_FREQUENCY])
{
    int16_t max = input[0], min = input[0];
    uint16_t i, signal_range, lower_mult, upper_mult, factor = 0, temp;
    
       
    //Linear search of the maximum and the minimum
    for(i=1; i<INPUT_FREQUENCY; i++)
    {
        if(input[(start + i) & mask] > max) max=input[(start + i) & mask];
        if(input[(start + i) & mask] < min) min=input[(start + i) & mask];    
    }

    //Compute signal range
    if(-(min) > max)
        max = -(min);
    signal_range = (uint16_t) max << 1;

    //Binary search of the best factor to multiply
    lower_mult = 1;
    upper_mult = (1<<16) - 1;
    while(lower_mult + 1 < upper_mult)
    {
        factor = lower_mult + ((upper_mult - lower_mult) >> 1);
        temp = (((int32_t)((int32_t)factor*(int32_t)signal_range)) >> 16);
        if(temp == MAX_INPUT) break; //Found factor
        if(temp > MAX_INPUT)
            upper_mult = factor;
        else
            lower_mult = factor;        
    }
    temp = (((int32_t)((int32_t)(factor+1)*(int32_t)signal_range)) >> 16);
    if(temp <= MAX_INPUT)
        factor = factor + 1;

    //Normalize the resampled signal
    temp = 0;
    for(i=0;i<INPUT_FREQUENCY; i+=RESAMPLE_STEP)
        output[temp++] = ((int32_t)((int32_t)(input[(start + i) & mask] + max) * (int32_t)factor)) >> 16;
}


/*
    classify
This function receives as input a buffer (of length multiple of 2 and at least INPUT_FREQUENCY)
that stores the samples of a heart beat to classify. It also receives the length of the buffer
and the index of the sample corresponding to the R peak of the heart beat. The input is not
modified and the output of the function is the number of the classification decission (from 
0 to NUM_CLASS-1) if it recognizes a class or NUM_CLASSES in case of an unknown beat.
0 is assumed to be the class Normal
*/
uint16_t classify(int16_t buffer[], uint16_t buffer_length, uint16_t peak)
{

    uint16_t resampled_buffer[OUTPUT_FREQUENCY];
    uint16_t coefficients[NUM_COEFFICIENTS];
    uint16_t productory[NUM_CLASSES];
    uint16_t mask = buffer_length - 1;
    uint16_t start = (peak + buffer_length - (INPUT_FREQUENCY >> 1)) & mask;
        
    normalizeAndResampleInput(buffer, start, mask, resampled_buffer);
    applyRandomProjection(resampled_buffer,coefficients);
    rescaledProductory(coefficients, productory);

    return classifyBeat(productory);
}


void report_rpeak(int32_t *arg[])
{
    int16_t *buffer = (int16_t*) arg[0];
    int32_t *offset = arg[2];
    int16_t bufferSize = 512;
    int32_t *rpeaks_counter = arg[4];
    int16_t class = 0;
    int32_t *indicesBeatClasses = arg[10];

    int id = pi_core_id();
    
    if (*rpeaks_counter%NUM_CORES == 0)	{
	    int start, end;
	    int chunk, even_parts;

	    even_parts = *rpeaks_counter/NUM_CORES;
	    if (even_parts < 1){ 
	        chunk = 1;
	        start = chunk*id;
	    }else{
	        int r = *rpeaks_counter%NUM_CORES;
	
	        if(id < (NUM_CORES-1)){
		        chunk = even_parts;
		        start = id*chunk;
	        }else{
		        chunk = even_parts;
		        start = id*even_parts;
	        }
	    } 
	    end = start + chunk;
	
    	for(int ix = start; ix < end; ix++){
            if(offset[ix]-bufferSize/2<0){
            	class = classify(&buffer[0], bufferSize, offset[ix]);
            }else if(offset[ix]+bufferSize/2>dim){
                class = classify(&buffer[dim-bufferSize], bufferSize, offset[ix]-(dim-bufferSize));
            }else{
                class = classify(&buffer[offset[ix]-bufferSize/2], bufferSize, offset[ix]-bufferSize/2);
            }

            indicesBeatClasses[ix] = class;
        }
    }
    else	{
	// code for dynamic sync of cores
	rpeak_sync = NUM_CORES;
	register int ix = id;
	 while (1){
	    if (ix >= *rpeaks_counter - 1)
		break;
	    if(offset[ix]-bufferSize/2<0){
		class = classify(&buffer[0], bufferSize, offset[ix]);
	    }else if(offset[ix]+bufferSize/2>dim){
		class = classify(&buffer[dim-bufferSize], bufferSize, offset[ix]-(dim-bufferSize));
	    }else{
		class = classify(&buffer[offset[ix]-bufferSize/2], bufferSize, offset[ix]-bufferSize/2);
	    }

	    indicesBeatClasses[ix] = class;
	    pi_cl_team_critical_enter();
	    ix = rpeak_sync;
	    rpeak_sync++;
	    pi_cl_team_critical_exit();
	}
    }
    
}
