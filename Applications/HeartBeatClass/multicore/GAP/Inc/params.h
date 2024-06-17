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



#define NUM_CLASSES        3      
#define NUM_COEFFICIENTS   8      
#define OUTPUT_FREQUENCY   50      
#define INPUT_FREQUENCY    400      
#define OFFSET_BOTTOM      2      
#define OFFSET_MIDDLE      4144      
#define RESAMPLE_STEP    (INPUT_FREQUENCY / OUTPUT_FREQUENCY)
#define MAX_INPUT          511      
#define THR_FACTOR         0      
#define COMPRESSED_ELEMENTS 8      

uint16_t rp_mat[1][OUTPUT_FREQUENCY] = {
{16544, 4, 36870, 22532, 32800, 16388, 32784, 32772, 32, 4484, 520, 8452, 16392, 1029, 16384, 32804, 0, 681, 528, 32769, 1113, 2081, 41344, 18, 4608, 10242, 9236, 33160, 1026, 522, 8326, 16385, 8832, 2305, 42240, 513, 16658, 16453, 16392, 64, 1, 1104, 24832, 2080, 2113, 136, 24705, 8232, 33, 1040},
};

uint16_t center[NUM_CLASSES][NUM_COEFFICIENTS] = {
{34087, 33642, 32609, 31700, 32781, 32890, 30995, 33375},
{34540, 33456, 32593, 31846, 33139, 32902, 31472, 33043},
{34195, 33484, 32684, 31644, 33134, 32960, 31069, 33080}
};

uint16_t segment_size[NUM_CLASSES][NUM_COEFFICIENTS] = {
{357, 635, 775, 553, 730, 779, 609, 446},
{1484, 644, 624, 683, 674, 293, 485, 884},
{1376, 428, 623, 213, 383, 856, 717, 314}
};

uint16_t slope_top[NUM_CLASSES][NUM_COEFFICIENTS] = {
{172, 96, 79, 111, 84, 78, 100, 137},
{41, 95, 98, 89, 91, 209, 126, 69},
{44, 143, 98, 288, 160, 71, 85, 195}
};

uint16_t slope_bottom[NUM_CLASSES][NUM_COEFFICIENTS] = {
{11, 6, 5, 7, 5, 5, 6, 9},
{2, 6, 6, 6, 6, 14, 8, 4},
{3, 9, 6, 19, 10, 4, 5, 13}
};
