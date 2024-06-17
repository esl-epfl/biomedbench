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


PI_L2 int16_t intermediate_map[INPUT_LENGTH_B*INPUT_DEPTH_B];
PI_L2 int32_t done = 0;
PI_L2 int32_t conv_layer_dimensions[3][4] = { {INPUT_LENGTH_A, INPUT_DEPTH_A, CONV0_PHASES, INPUT_LENGTH_B}, {INPUT_LENGTH_B, INPUT_DEPTH_B, CONV1_PHASES, INPUT_LENGTH_C}, {INPUT_LENGTH_C, INPUT_DEPTH_C, CONV2_PHASES, INPUT_LENGTH_D}};

PI_L1 int16_t *input_CL_A;
PI_L1 int16_t *input_CL_B;
PI_L1 signed char *filter_CL;
PI_L1 signed char *bias_CL;
PI_L1 int32_t *maximum;
PI_L1 pi_cl_dma_cmd_t dmaCp;

PI_L2 struct pi_device cluster_dev;
PI_L2 struct pi_cluster_conf cl_conf;

#ifdef GAPUINO
#include "GAPuino/gpio_mgmt.h"
#elif defined(GAP9)
#include "GAP9/peripherals_sleep.h"
#endif

#include "pmsis.h"
    
// ===========================> Functions Prototype <===============================
void conv1d(const int16_t *data, const signed char *filter, int16_t *map_out, const signed char *bias, int32_t filter_size,
            int32_t input_len, int32_t input_depth, int32_t output_len, int32_t n_filter, int32_t strides, int32_t relu);

void conv_max1d(const int16_t *data, const signed char *filter, int16_t *map_out, const signed char *bias,
                int32_t input_len, int32_t input_depth, int32_t output_len);

void batch_normalization(const int16_t *data, const signed char *gamma, const signed char *beta, const signed char *mean, const signed char *var,
                         int16_t *map_out, int32_t input_len);

int16_t forward_propagation(int16_t *data, int16_t *intermediate);

// =================================================================================


// Convolution / maximum layer in the cluster
void CL_conv_max1d(int32_t *arg[]) {

    // fetch the parameters from arg
    const int16_t *data = (int16_t *)arg[0];
    const signed char *filter = (signed char *)arg[1];
    const signed char *bias = (signed char *)arg[2];
    int16_t *map_out = (int16_t *)arg[3];
    int32_t chunk_length = (int32_t) *arg[4];
    int32_t chunk_offset = (int32_t) *arg[5];
    int32_t input_length = (int32_t) *arg[6];
    int32_t input_depth = (int32_t) *arg[7];
    int32_t output_len = (int32_t) *arg[8];
     
    register int id = pi_core_id();
    register int32_t sum, mult, chunk = FILTER_NUMBER / NUM_CORES;
    register signed char *filter_address;
    
    // Initial set
    if (chunk_offset % 4 == 0)  {
        for (int32_t w_n = id * chunk; w_n < (id + 1) * chunk; w_n++) {
            maximum[w_n] = NEG_INF;
        }
    }
    
    pi_cl_team_barrier();
    
    for (int32_t start_index = 1; start_index < chunk_length + 1; start_index++) {
        filter_address = filter + id * chunk * input_depth * 3;					// reset filter address every time we change position
        register int32_t offset = start_index - 1;						// precalculate this addition
        
        for (int32_t w_n = id * chunk; w_n < (id+1) * chunk; w_n++) {
            sum = 0;
            for (int32_t w_j = 0; w_j < input_depth; w_j++) {
                register int16_t *data_address = &data[(chunk_length+2) * w_j + offset];		// reset data address every time we go to a deeper layer
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
            
            if ( (start_index - 1 + chunk_offset) % 4 == 3) {
                mem2d(map_out, output_len, w_n, (start_index - 1 + chunk_offset) / 4) = (int16_t) maximum[w_n];
                maximum[w_n] = NEG_INF;
            }
        }
    }
}


// DMA transfer of continuous memory from L2 to L1
static void fCore0_DmaTransfer(int32_t **arg)	{
    pi_cl_dma_cmd_t dmaCp;
    pi_cl_dma_cmd((unsigned int)arg[0], (unsigned int)arg[1], *arg[2], PI_CL_DMA_DIR_EXT2LOC, &dmaCp);
    // Wait for dma to finish
    pi_cl_dma_wait(&dmaCp);
}

// DMA transfer of continuous memory from L2 to L1
static void fCore0_DmaTransfer_Large_Block(int32_t **arg)	{
    pi_cl_dma_cmd_t dmaCp;
    pi_cl_dma_cmd((unsigned int)arg[0], (unsigned int)arg[1], *arg[2] / 2, PI_CL_DMA_DIR_EXT2LOC, &dmaCp);
    // Wait for dma to finish
    pi_cl_dma_wait(&dmaCp);
    
    pi_cl_dma_cmd((unsigned int) ((char *)arg[0] + *arg[2] / 2 ), (unsigned int) ( (char *) arg[1] + *arg[2] / 2 ), *arg[2] / 2, PI_CL_DMA_DIR_EXT2LOC, &dmaCp);
    // Wait for dma to finish
    pi_cl_dma_wait(&dmaCp);
}

// DMA transfer non-continuous memory blocks from L2 to L1
static void fCore0_DmaTransfer_chunks_blocking(int32_t **arg)	{
    
    const int32_t chunk_length = *arg[2];
    const int32_t input_depth = *arg[3];
    const int32_t offset = *arg[4];
    const int32_t input_length = *arg[5];
    
    // must take care of the padding
    int32_t size_of_transfer = (chunk_length + 2) * sizeof(int16_t), padding_offset_Origin = -1, padding_offset_Target = 0;
    
    pi_cl_dma_cmd_2d((unsigned int) ( (int16_t *)arg[0] + offset + padding_offset_Origin ), (unsigned int) ( (int16_t *)arg[1] + padding_offset_Target ),
                     size_of_transfer * input_depth, input_length * sizeof(int16_t), size_of_transfer, PI_CL_DMA_DIR_EXT2LOC, &dmaCp);
}

// this is for the first transfer
static void fCore0_DmaTransfer_chunks_blocking_First(int32_t **arg)	{
    
    pi_cl_dma_cmd_t dmaCp;
    
    const int32_t chunk_length = *arg[2];
    const int32_t input_depth = *arg[3];
    const int32_t offset = *arg[4];
    const int32_t input_length = *arg[5];
    
    // must take care of the padding
    int32_t size_of_transfer = (chunk_length + 1) * sizeof(int16_t), padding_offset_Origin = 0, padding_offset_Target = 1;
    
    
    for (int i = 0; i < input_depth; i++)   {
        pi_cl_dma_cmd((unsigned int) ( (int16_t *)arg[0] + i * input_length + offset + padding_offset_Origin ), (unsigned int) ( (int16_t *)arg[1] + i * (chunk_length + 2) + padding_offset_Target ), size_of_transfer, PI_CL_DMA_DIR_EXT2LOC, &dmaCp);
        // Wait for dma to finish
        pi_cl_dma_wait(&dmaCp);
        // full with 0s for padding and simpler code in the cluster without ifs
        if (offset == 0)
            *((int16_t *)arg[1] + i * (chunk_length + 2)) = 0;
        //printf("Dma epoch: %d/%d\n", i+1, input_depth);
    }
    
}


// Intermediate functions of all modules for calling the cluster cores
static void conv_layer_1_fork(int32_t **arg)	{
    pi_cl_team_fork(NUM_CORES, CL_conv_max1d, arg);
}


static int Convolution_Cluster(int32_t *arg[])  {
    
    // fetching the arguments
    int16_t * layer_in = (int16_t *)arg[0];
    int16_t * layer_out = (int16_t *)arg[1];
    signed char *filters = (signed char *)arg[2];
    signed char *bias = (signed char *)arg[3];
    int32_t no_conv = *arg[4];
    int32_t input_length = conv_layer_dimensions[no_conv][0];
    int32_t input_depth = conv_layer_dimensions[no_conv][1];
    int32_t no_phases = conv_layer_dimensions[no_conv][2];
    int32_t output_len = conv_layer_dimensions[no_conv][3];
    
    int32_t *arguments_DMA[7];
    int32_t *arguments_Conv[9];
    int32_t size_transfer, offset, offset_DMA, chunk_length, blocking = 1;
    
    
#ifdef PRINT_CLUSTER_PROGRESS
    printf("\n -------------------- Conv Block %d -----------------\n", no_conv);
#endif
    chunk_length = input_length / no_phases;

    // Memory Allocation - L1 arrays (need padding +2 columns for input_CL_arrays)
    input_CL_A = (int16_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int16_t) * input_depth * (chunk_length + 2) ); 
    if (input_CL_A == NULL) {
        printf(" Input Array A: L1 allocation error \n");
        return -1;
    }
    
    input_CL_B = (int16_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int16_t) * input_depth * (chunk_length + 2) ); 
    if (input_CL_B == NULL) {
        printf(" Input Array A: L1 allocation error \n");
        return -1;
    }
    
    filter_CL = (signed char *) pi_cl_l1_malloc(&cluster_dev, sizeof(char) * FILTER_LENGTH * input_depth * FILTER_NUMBER);
    if (filter_CL == NULL)  {
        printf(" Filters: L1 allocation error \n");
        return -1;
    }
    
    bias_CL = (signed char *) pi_cl_l1_malloc(&cluster_dev, sizeof(char) * FILTER_NUMBER);
    if (bias_CL == NULL)    {
        printf(" Bias: L1 allocation error \n");
        return -1;
    }
    
    maximum = (int32_t *) pi_cl_l1_malloc(&cluster_dev, sizeof(int32_t) * FILTER_NUMBER);
    if (maximum == NULL)    {
        printf(" Maximum helper array error: L1 allocation error \n");
        return -1;
    }
    
    // ------------- DMA TRANSFERS CONV BLOCK 0 ------
#ifdef PRINT_CLUSTER_PROGRESS
    printf("Starting Filter and Bias DMA transfers.... \n");
#endif
    // bias
    size_transfer = sizeof(char) * FILTER_NUMBER;
    arguments_DMA[0] = (int32_t *)bias;
    arguments_DMA[1] = (int32_t *)bias_CL;
    arguments_DMA[2] = (int32_t *)&size_transfer;
    
    fCore0_DmaTransfer( arguments_DMA );
    
    // filter
    size_transfer = sizeof(char) * FILTER_LENGTH * input_depth * FILTER_NUMBER;
    arguments_DMA[0] = (int32_t *)filters;
    arguments_DMA[1] = (int32_t *)filter_CL;
    arguments_DMA[2] = (int32_t *)&size_transfer;
    
    fCore0_DmaTransfer_Large_Block( arguments_DMA );
    
    // ------------ END OF STATIC DMA TRANSFERS -----------
    
    
    // Transfer first block
#ifdef PRINT_CLUSTER_PROGRESS
    printf("Starting first Input block DMA transfer.... \n");
#endif
    offset_DMA = 0;
    blocking = 1;
    arguments_DMA[0] = (int32_t *)layer_in;
    arguments_DMA[1] = (int32_t *)input_CL_A;
    arguments_DMA[2] = (int32_t *)&chunk_length;
    arguments_DMA[3] = (int32_t *)&input_depth;
    arguments_DMA[4] = (int32_t *)&offset_DMA;
    arguments_DMA[5] = (int32_t *)&input_length;
    arguments_DMA[6] = (int32_t *)&blocking;
    
    fCore0_DmaTransfer_chunks_blocking_First(arguments_DMA);
    
    //COMPUTATION
    offset = 0;
    arguments_Conv[0] = (int32_t *)input_CL_A;
    arguments_Conv[1] = (int32_t *)filter_CL;
    arguments_Conv[2] = (int32_t *)bias_CL;
    arguments_Conv[3] = (int32_t *)layer_out;
    arguments_Conv[4] = (int32_t *)&chunk_length;
    arguments_Conv[5] = (int32_t *)&offset;
    arguments_Conv[6] = (int32_t *)&input_length;
    arguments_Conv[7] = (int32_t *)&input_depth;
    arguments_Conv[8] = (int32_t *)&output_len;
    
    
#ifdef PRINT_CLUSTER_PROGRESS
    printf("Starting computations (interchanging with DMA transfers) .... \n");
#endif
    for (int i = 0; i < no_phases; i++)   {
        
        //wait for previous transfer
        if (i!=0)
            pi_cl_dma_wait(&dmaCp);
        // DMA next input in parallel
        if ( i != no_phases - 1)    {
            if (i % 2 == 0)
                arguments_DMA[1] = input_CL_B;
            else
                arguments_DMA[1] = input_CL_A;
            
            blocking = 0;
            
            offset_DMA += chunk_length;
            
            fCore0_DmaTransfer_chunks_blocking(arguments_DMA);
        }
        else    {
            for (int i = 0; i < input_depth; i++)   {
                    *(input_CL_B + i * (chunk_length + 2) + chunk_length + 1) = 0;
            }
        }
        
        conv_layer_1_fork(arguments_Conv);
        
        // Change the buffer input (Circular buffer)
        if (i % 2 == 0)
            arguments_Conv[0] = input_CL_B;
        else
            arguments_Conv[0] = input_CL_A;
        
        offset += chunk_length;
    }
    
    pi_cl_l1_free(&cluster_dev, (void *) input_CL_A, sizeof(int16_t) * input_depth * (chunk_length + 2) );
    pi_cl_l1_free(&cluster_dev, (void *) input_CL_B, sizeof(int16_t) * input_depth * (chunk_length + 2) );
    pi_cl_l1_free(&cluster_dev, (void *) filter_CL, sizeof(char) * FILTER_LENGTH * input_depth * FILTER_NUMBER);
    pi_cl_l1_free(&cluster_dev, (void *) bias_CL, sizeof(char) * FILTER_NUMBER);
    pi_cl_l1_free(&cluster_dev, (void *) maximum, sizeof(int32_t) * FILTER_NUMBER);
    
#ifdef PRINT_CLUSTER_PROGRESS
    printf("... finished\n-------------------------------------------------- \n");
#endif

    return 0;
}


void conv1d(const int16_t *data, const signed char *filter, int16_t *map_out, const signed char *bias, const int32_t filter_size,
            const int32_t input_len, const int32_t input_depth, const int32_t output_len, const int32_t n_filter, const int32_t strides,
            const int32_t relu) {
    register int32_t sum;
    int32_t mult;
    for (int32_t start_index = 0; start_index < input_len; start_index += strides) {
	register signed char *filter_address = filter;		// Ali's optimization for faster addressing of filter elements
	//register int32_t offset = start_index;		// precalculate this addition
        for (int32_t w_n = 0; w_n < n_filter; w_n++) {
            sum = 0;
            for (int32_t w_j = 0; w_j < input_depth; w_j++) {
            register int16_t *data_address = &data[input_len * w_j + start_index];		// similar to Ali's optimization for faster addressing of data elements
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
                #ifdef PRINT_OVERFLOW
                    printf("Overflow %d\n", sum);
                #endif
                    sum = (1<<15) -1;
            }else if (sum < -(1 << 15)){
                #ifdef PRINT_OVERFLOW
                    printf("Overflow %d\n", sum);
                #endif
                    sum = -(1<<15) +1;
            }
            mem2d(map_out, output_len, w_n, start_index / strides) = (int16_t) sum;
            #ifdef SERIAL_AVAILABLE
                printf("FC out %d : %x\n", w_n, sum);
            #endif
        }
    }
}


void batch_normalization(const int16_t *data, const signed char *gamma, const signed char *beta, const signed char *mean, const signed char *var,
                         int16_t *map_out, const int32_t input_len) {

    int16_t *data_address = data;
    int16_t *map_out_address = map_out;
    for (int32_t w_j = 0; w_j < 128; w_j++) {
	for (int32_t start_index = 0; start_index < input_len; start_index++) {
            int16_t normalized = *(data_address++) -
		((int16_t) mean[w_j] << (NUM_FRACTION_DATA - NUM_FRACTION_BN));
            int16_t standardized = MUL(normalized, var[w_j], NUM_FRACTION_BN);
            int16_t new_standardized = MUL(standardized, gamma[w_j], NUM_FRACTION_BN);
            *(map_out_address++) =
                    (int16_t) (new_standardized + ((int16_t) beta[w_j] << (NUM_FRACTION_DATA - NUM_FRACTION_BN)));
        }
    }

}

void relu(int16_t * const data, int32_t input_len) {
    for (int32_t i = 0; i < input_len; i++)
        data[i] = (data[i] < 0) ? 0 : data[i];
}

void conv_block(const int32_t block, int16_t * const layer_in, int16_t * const conv1d_out){
    int32_t depth_size[6] = {23, 128, 128, 128, 100, 2};
    int32_t map_size[6] = {1024, 256, 64, 16, 1, 1};
    signed char* filter = conv1d_w[block];
    signed char* bias = conv1d_b[block];
#ifdef SERIAL_AVAILABLE
    printf("Block %d\n", block);
#endif
    conv_max1d(layer_in, filter, conv1d_out, bias, map_size[block], depth_size[block], map_size[block+1]);

    batch_normalization(conv1d_out, bn[block * 4], bn[block * 4 + 1], bn[block * 4 + 2], bn[block * 4 + 3],
                        conv1d_out, map_size[block+1]);

    relu(conv1d_out, map_size[block+1] * depth_size[block + 1]);
}

void conv_block_post_CL(const int32_t block, int16_t * const conv1d_out){
    int32_t depth_size[6] = {23, 128, 128, 128, 100, 2};
    int32_t map_size[6] = {1024, 256, 64, 16, 1, 1};
    signed char* filter = conv1d_w[block];
    signed char* bias = conv1d_b[block];
#ifdef SERIAL_AVAILABLE
    printf("Block %d\n", block);
#endif
    
    batch_normalization(conv1d_out, bn[block * 4], bn[block * 4 + 1], bn[block * 4 + 2], bn[block * 4 + 3],
                        conv1d_out, map_size[block+1]);

    relu(conv1d_out, map_size[block+1] * depth_size[block + 1]);
}

int16_t forward_propagation(int16_t *data, int16_t *intermediate) {
    int32_t fc_depth_size[3] = {128, 100, 2};
    int32_t fc_map_size[3] = {16, 1, 1};
    int16_t *intermediate_map0 = intermediate;
    int16_t *intermediate_map1 = data;
    
    // Cluster variables
    int32_t *arguments[5];
    int32_t no_convolution_layer;
    
    // Power On cluster
    struct pi_cluster_task cl_task;
    
    // Offload CONV LAYER 0 to the cluster
    no_convolution_layer = 0;
    arguments[0] = intermediate_map1;
    arguments[1] = intermediate_map0;
    arguments[2] = conv1d_w[0];
    arguments[3] = conv1d_b[0];
    arguments[4] = &no_convolution_layer;
    
    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, Convolution_Cluster, arguments));
    
    #ifdef GAPUINO
    pi_cluster_close(&cluster_dev);
    #endif

    conv_block_post_CL(no_convolution_layer, intermediate_map0);
    
    // Offload CONV LAYER 1 to the cluster
    no_convolution_layer = 1;
    arguments[0] = intermediate_map0;
    arguments[1] = intermediate_map1;
    arguments[2] = conv1d_w[1];
    arguments[3] = conv1d_b[1];
    
    #ifdef GAPUINO
    /* Configure & open cluster. */
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        printf("Cluster open failed !\n");
        pmsis_exit(-1);
    }
    #endif

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, Convolution_Cluster, arguments));
    
    #ifdef GAPUINO
    pi_cluster_close(&cluster_dev);
    #endif

    conv_block_post_CL(no_convolution_layer, intermediate_map1);
    
    // Offload CONV LAYER 2 to the cluster
    no_convolution_layer = 2;
    arguments[0] = intermediate_map1;
    arguments[1] = intermediate_map0;
    arguments[2] = conv1d_w[2];
    arguments[3] = conv1d_b[2];
    
    #ifdef GAPUINO
    /* Configure & open cluster. */
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        printf("Cluster open failed !\n");
        pmsis_exit(-1);
    }
    #endif

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, Convolution_Cluster, arguments));
    
    conv_block_post_CL(no_convolution_layer, intermediate_map0);
    
    pi_cluster_close(&cluster_dev);
    
    //  ************  BLOCK 2  ************ //
    int16_t *layer_out;// = intermediate_map1;
    int16_t *layer_in;// = intermediate_map0;
    //conv_block(1, layer_in, layer_out);
    
    
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
    #ifdef GAP9
    setup_frequency_voltage( 240000000, 650);
    #endif

#if defined(GAPUINO) || defined(GAP9)    
    gpio_ini();
    printf(" Geia \n");
    gpio_enable(); // set frequency to max (250MHz for GAPuino) and enable GPIO signal for accurate measurements
#endif
    
#ifdef SERIAL_AVAILABLE
    printf("Input Array : %x\n", input_array[0]);
#endif

#ifdef PROFILING_ON
    profile_start();
#endif
    
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
    int16_t predict = forward_propagation(input_array, intermediate_map);
    
#ifdef PRINT_PREDICTION
    printf("Prediction : %d\n", predict);
#endif

#ifdef PROFILING_ON
    profile_stop();
#endif
    
#if defined(GAPUINO) || defined(GAP9)
    gpio_disable(); // disable GPIO to end measurements
#endif

    return 0;
}
