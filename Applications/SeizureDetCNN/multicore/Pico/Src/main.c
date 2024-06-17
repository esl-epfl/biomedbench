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


//////////////////////////////////////////////////////
// Main Author:     Alireza Amirshahi               //
// Optimizations:   Dimitrios Samakovlis            //
/////////////////////////////////////////////////////



#include "main.h"

#include "pico/multicore.h"
#include "Pico_management/gpio_plus_sleep.h"
#include "Pico_management/multicore.h"


#include <stdio.h>
#include <stdint.h>

// Some stables regarding the convolutional layer sizes and filter weights
#define FILTER_LENGTH 3
#define FILTER_NUMBER 128
#define INPUT_DEPTH_A 23
#define INPUT_DEPTH_B 128
#define INPUT_DEPTH_C 128
#define INPUT_LENGTH_A 1024
#define INPUT_LENGTH_B 256
#define INPUT_LENGTH_C 64
#define INPUT_LENGTH_D 16

// Number of phases required per conv layer
// NUmbers calculated on paper to avoid L1 memory overflow (Power of 2 for perfect divisions)
#define CONV0_PHASES 2
#define CONV1_PHASES 32
#define CONV2_PHASES 4


int16_t intermediate_map[INPUT_LENGTH_B*INPUT_DEPTH_B];
int32_t done = 0;
int32_t conv_layer_dimensions[3][3] = { {INPUT_LENGTH_A, INPUT_DEPTH_A, INPUT_LENGTH_B}, 
                                        {INPUT_LENGTH_B, INPUT_DEPTH_B, INPUT_LENGTH_C},
                                        {INPUT_LENGTH_C, INPUT_DEPTH_C, INPUT_LENGTH_D} };
int32_t maximum[FILTER_NUMBER];
int32_t *arguments_Conv[7];
    
// ===========================> Functions Prototype <===============================
void conv1d(const int16_t *data, const signed char *filter, int16_t *map_out, const signed char *bias, int32_t filter_size,
            int32_t input_len, int32_t input_depth, int32_t output_len, int32_t n_filter, int32_t strides, int32_t relu);

void batch_normalization(const int16_t *data, const signed char *gamma, const signed char *beta, const signed char *mean, const signed char *var,
                         int16_t *map_out, int32_t input_len);

int16_t forward_propagation(const int16_t *data, int16_t *intermediate);

// =================================================================================


// Convolution / maximum layer in the cluster
void conv_max(int32_t *arg[], int id) {

    // fetch the parameters from arg
    const int16_t *data = (int16_t *)arg[0];
    const signed char *filter = (signed char *)arg[1];
    const signed char *bias = (signed char *)arg[2];
    int16_t *map_out = (int16_t *)arg[3];
    int32_t input_length = (int32_t) arg[4];
    int32_t input_depth = (int32_t) arg[5];
    int32_t output_len = (int32_t) arg[6];
     
    register int32_t sum, mult, chunk = FILTER_NUMBER / NUMBER_OF_CORES;
    const signed char *filter_address;
    
    // Initial set
    for (int32_t w_n = id * chunk; w_n < (id + 1) * chunk; w_n++) {
        maximum[w_n] = (int32_t)NEG_INF;
    }
    pico_barrier(id);

    // Left most position - one less inner iteration because of padding
    filter_address = filter + id * chunk * input_depth * 3;
    for (int32_t w_n = id * chunk; w_n < (id+1) * chunk; w_n++) {
        sum = 0;
        for (int32_t w_j = 0; w_j < input_depth; w_j++) {
            const int16_t *data_address = &data[input_length * w_j];		// reset data address every time we go to a deeper layer

            filter_address++; //skip padding
            for (register int32_t w_i = 1; w_i < 3; w_i++) {
                mult = MUL_CONV(*(filter_address++), *(data_address++), NUM_FRACTION_CNV_FC);
                sum += mult;
            }
        }
            
        sum += bias[w_n];
            
        if (sum > (1 << 15) ){
            sum = (1<<15) -1;
        }
        else if (sum < -(1 << 15)){
            sum = -(1<<15) +1;
        }
        if (sum > maximum[w_n]) {
            maximum[w_n] = sum;
        }      
    }

    for (int32_t start_index = 1; start_index < input_length - 1; start_index++) {
        filter_address = filter + id * chunk * input_depth * 3;					// reset filter address every time we change position
        register int32_t offset = start_index - 1;						        // precalculate this addition
        
        for (int32_t w_n = id * chunk; w_n < (id+1) * chunk; w_n++) {
            sum = 0;
            for (int32_t w_j = 0; w_j < input_depth; w_j++) {
                const int16_t *data_address = &data[input_length * w_j + offset];		// reset data address every time we go to a deeper layer
                for (register int32_t w_i = 0; w_i < 3; w_i++) {
                    mult = MUL_CONV(*(filter_address++), *(data_address++), NUM_FRACTION_CNV_FC);
                    sum += mult;
                }
            }
            
            sum += bias[w_n];
            
            if (sum > (1 << 15) ){
                sum = (1<<15) -1;
            }
            else if (sum < -(1 << 15)){
                sum = -(1<<15) +1;
            }
            if (sum > maximum[w_n]) {
                maximum[w_n] = sum;
            }
            
            if ( start_index % 4 == 3) {
                mem2d(map_out, output_len, w_n, start_index / 4) = (int16_t) maximum[w_n];
                maximum[w_n] = NEG_INF;
            }
        }
    }

    filter_address = filter + id * chunk * input_depth * 3;
    register int32_t offset = input_length - 2;

    for (int32_t w_n = id * chunk; w_n < (id+1) * chunk; w_n++) {
        sum = 0;
        for (int32_t w_j = 0; w_j < input_depth; w_j++) {
            const int16_t *data_address = &data[input_length * w_j + offset];		// reset data address every time we go to a deeper layer

            for (register int32_t w_i = 0; w_i < 2; w_i++) {
                mult = MUL_CONV(*(filter_address++), *(data_address++), NUM_FRACTION_CNV_FC);
                sum += mult;
            }
            filter_address++;           //skip padding
        }
            
        sum += bias[w_n];
            
        if (sum > (1 << 15) ){
            sum = (1<<15) -1;
        }
        else if (sum < -(1 << 15)){
            sum = -(1<<15) +1;
        }
        if (sum > maximum[w_n]) {
            maximum[w_n] = sum;
        }

        if ((input_length - 1) % 4 == 3) {
            mem2d(map_out, output_len, w_n, (input_length - 1) / 4) = (int16_t) maximum[w_n];
            maximum[w_n] = NEG_INF;
        }
    }
    pico_barrier(id);                   // important to put this - else core 0 will kill core 1 if it finishes earlier
}


// Intermediate functions of all modules for calling the cluster cores
static void multicore_conv_layer_1(void)	{
    conv_max(arguments_Conv, 1);
}


void conv1d(const int16_t *data, const signed char *filter, int16_t *map_out, const signed char *bias, const int32_t filter_size,
            const int32_t input_len, const int32_t input_depth, const int32_t output_len, const int32_t n_filter, const int32_t strides,
            const int32_t relu) {
    register int32_t sum;
    int32_t mult;
    for (int32_t start_index = 0; start_index < input_len; start_index += strides) {
	const signed char *filter_address = filter;		                                // Ali's optimization for faster addressing of filter elements

        for (int32_t w_n = 0; w_n < n_filter; w_n++) {
            sum = 0;
            for (int32_t w_j = 0; w_j < input_depth; w_j++) {
            const int16_t *data_address = &data[input_len * w_j + start_index];		// similar to Ali's optimization for faster addressing of data elements
                for (register int32_t w_i = 0; w_i < filter_size; w_i++) {
                    mult = MUL_CONV(*(filter_address), *(data_address), NUM_FRACTION_CNV_FC);
                    sum += mult;
                    filter_address++;
                    data_address++;
                }
            }
            sum += bias[w_n];
            if (relu && sum < 0)
                sum = 0; // Relu
            if (sum > (1 << 15) ){
                    sum = (1<<15) -1;
            }else if (sum < -(1 << 15)){
                    sum = -(1<<15) +1;
            }
            mem2d(map_out, output_len, w_n, start_index / strides) = (int16_t) sum;
        }
    }
}


void batch_normalization(const int16_t *data, const signed char *gamma, const signed char *beta, const signed char *mean, const signed char *var,
                         int16_t *map_out, const int32_t input_len) {

    const int16_t *data_address = data;
    int16_t *map_out_address = map_out;
    for (int32_t w_j = 0; w_j < 128; w_j++) {
	    for (int32_t start_index = 0; start_index < input_len; start_index++) {
            int16_t normalized = *(data_address++) - ((int16_t) mean[w_j] << (NUM_FRACTION_DATA - NUM_FRACTION_BN));
            int16_t standardized = MUL(normalized, var[w_j], NUM_FRACTION_BN);
            int16_t new_standardized = MUL(standardized, gamma[w_j], NUM_FRACTION_BN);
            *(map_out_address++) = (int16_t) (new_standardized + ((int16_t) beta[w_j] << (NUM_FRACTION_DATA - NUM_FRACTION_BN)));
        }
    }

}


void relu(int16_t * const data, int32_t input_len) {
    for (int32_t i = 0; i < input_len; i++)
        data[i] = (data[i] < 0) ? 0 : data[i];
}


void post_convolution(const int32_t block, int16_t * conv1d_out){
    int32_t depth_size[6] = {23, 128, 128, 128, 100, 2};
    int32_t map_size[6] = {1024, 256, 64, 16, 1, 1};
    const signed char* filter = conv1d_w[block];
    const signed char* bias = conv1d_b[block];
    
    batch_normalization(conv1d_out, bn[block * 4], bn[block * 4 + 1], bn[block * 4 + 2], bn[block * 4 + 3], conv1d_out, map_size[block+1]);

    relu(conv1d_out, map_size[block+1] * depth_size[block + 1]);
}


int16_t forward_propagation(const int16_t *data, int16_t *intermediate) {
    int32_t fc_depth_size[3] = {128, 100, 2};
    int32_t fc_map_size[3] = {16, 1, 1};
    int16_t *intermediate_map0 = intermediate;
    int16_t *intermediate_map1 = data;
    
    // COMPUTATIONAL BLOCK 1: CONVOLUTIONAL LAYERS
    int32_t no_convolution_layer;
    
    // *** CONV LAYER 0 ***
    no_convolution_layer = 0;
    arguments_Conv[0] = (int32_t *)intermediate_map1;
    arguments_Conv[1] = (int32_t *)conv1d_w[no_convolution_layer];
    arguments_Conv[2] = (int32_t *)conv1d_b[no_convolution_layer];
    arguments_Conv[3] = (int32_t *)intermediate_map0;
    arguments_Conv[4] = (int32_t *)conv_layer_dimensions[no_convolution_layer][0];
    arguments_Conv[5] = (int32_t *)conv_layer_dimensions[no_convolution_layer][1];
    arguments_Conv[6] = (int32_t *)conv_layer_dimensions[no_convolution_layer][2];

    // Call core 1
    multicore_launch_core1(multicore_conv_layer_1);
    conv_max(arguments_Conv, 0);
    multicore_reset_core1();
    
    post_convolution(no_convolution_layer, intermediate_map0);
    
    // *** CONV LAYER 1 ***
    no_convolution_layer = 1;
    arguments_Conv[0] = (int32_t *)intermediate_map0;
    arguments_Conv[1] = (int32_t *)conv1d_w[no_convolution_layer];
    arguments_Conv[2] = (int32_t *)conv1d_b[no_convolution_layer];
    arguments_Conv[3] = (int32_t *)intermediate_map1;
    arguments_Conv[4] = (int32_t *)conv_layer_dimensions[no_convolution_layer][0];
    arguments_Conv[5] = (int32_t *)conv_layer_dimensions[no_convolution_layer][1];
    arguments_Conv[6] = (int32_t *)conv_layer_dimensions[no_convolution_layer][2];
    
    // Call core 1
    multicore_launch_core1(multicore_conv_layer_1);
    conv_max(arguments_Conv, 0);
    multicore_reset_core1();
    
    post_convolution(no_convolution_layer, intermediate_map1);
    
    // *** CONV LAYER 2 ***
    no_convolution_layer = 2;
    arguments_Conv[0] = (int32_t *)intermediate_map1;
    arguments_Conv[1] = (int32_t *)conv1d_w[no_convolution_layer];
    arguments_Conv[2] = (int32_t *)conv1d_b[no_convolution_layer];
    arguments_Conv[3] = (int32_t *)intermediate_map0;
    arguments_Conv[4] = (int32_t *)conv_layer_dimensions[no_convolution_layer][0];
    arguments_Conv[5] = (int32_t *)conv_layer_dimensions[no_convolution_layer][1];
    arguments_Conv[6] = (int32_t *)conv_layer_dimensions[no_convolution_layer][2];
    
    // Call core 1
    multicore_launch_core1(multicore_conv_layer_1);
    conv_max(arguments_Conv, 0);
    multicore_reset_core1();

    post_convolution(no_convolution_layer, intermediate_map0);
    

    // COMPUTATIONAL BLOCK 2: FULLY CONNECTED LAYERS
    int16_t *layer_out;// = intermediate_map1;
    int16_t *layer_in;// = intermediate_map0;
    
    //  ************  FC 0  ************ //
    layer_out = intermediate_map1;
    layer_in = intermediate_map0;
    conv1d(layer_in, dense_w[0], layer_out, dense_b[0], fc_map_size[0],fc_map_size[0],
           fc_depth_size[0], fc_map_size[1], fc_depth_size[1], fc_map_size[0], 1);
    
    #ifdef PRINT_FC1_OUT
    printf("\nFC 0 out:\n");
    for(int i=0; i< fc_depth_size[1]*fc_map_size[0]; i++)
	    printf("%d ", layer_out[i]);
    printf("\n");
    #endif
    
    //  ************  FC 1  ************ //
    layer_out = intermediate_map0;
    layer_in = intermediate_map1;
    conv1d(layer_in, dense_w[1], layer_out, dense_b[1], fc_map_size[1],fc_map_size[1],
           fc_depth_size[1], fc_map_size[2], fc_depth_size[2], fc_map_size[1], 0);
    
    #ifdef PRINT_FC2_OUT
    printf("\nFC 1 out:\n");
    for(int i=0; i< fc_depth_size[2]*fc_map_size[1]; i++)
	    printf("%d ", layer_out[i]);
    printf("\n");
    #endif

    if (layer_out[0] > layer_out[1])
        return 0;
    else
        return 1;
}


int main()
{
    gpio_ini();
    gpio_enable(); // set frequency to max (250MHz for GAPuino) and enable GPIO signal for accurate measurements
    
    int16_t predict = forward_propagation(input_array, intermediate_map);
    
#ifdef PRINT_PREDICTION
    printf("Prediction : %d\n", predict);
#endif
    
    gpio_disable(); // disable GPIO to end measurements

    return 0;
}
