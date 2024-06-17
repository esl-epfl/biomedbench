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


#ifndef _MEL_BASIS_SMALL_H_
#define _MEL_BASIS_SMALL_H_

// This file contains the MEL matrix for projecting a power spectrum into a mel basis
// The numbers are hardcoded and taken from python.

//////////////////////////////////////////////////////////////////////////////////
// Note that this only works when the RFFT size used is 2048 (so the output		//
// will be 1025 samples)														//
//////////////////////////////////////////////////////////////////////////////////

#define MEL_ROWS		128
#define MEL_COLUMNS		1025

#define MAX_NZ_ELEMS    50  // max number of non-zero elements 

/* 
    Indexes of the first and last non-zero elements for each row of the mel basis. 
    For each row, the non-zero elements are stored one after the other, so I just
    need to store the indexes of the first and last element
*/
static const int mel_nz_indexes[MEL_ROWS][2] = {
  	{1, 5},
	{3, 8},
	{6, 11},
	{9, 14},
	{12, 17},
	{15, 20},
	{18, 23},
	{21, 26},
	{24, 29},
	{27, 32},
	{30, 35},
	{33, 38},
	{36, 41},
	{39, 44},
	{42, 47},
	{45, 50},
	{48, 53},
	{51, 56},
	{54, 59},
	{57, 62},
	{60, 65},
	{63, 68},
	{66, 71},
	{69, 74},
	{72, 77},
	{75, 80},
	{78, 83},
	{81, 86},
	{84, 89},
	{87, 92},
	{90, 95},
	{93, 98},
	{96, 101},
	{99, 104},
	{102, 107},
	{105, 110},
	{108, 113},
	{111, 116},
	{114, 119},
	{117, 122},
	{120, 125},
	{123, 128},
	{126, 131},
	{129, 135},
	{132, 138},
	{136, 141},
	{139, 145},
	{142, 148},
	{146, 152},
	{149, 156},
	{153, 159},
	{157, 163},
	{160, 167},
	{164, 171},
	{168, 176},
	{172, 180},
	{177, 184},
	{181, 189},
	{185, 193},
	{190, 198},
	{194, 203},
	{199, 208},
	{204, 213},
	{209, 218},
	{214, 224},
	{219, 229},
	{225, 235},
	{230, 240},
	{236, 246},
	{241, 252},
	{247, 259},
	{253, 265},
	{260, 271},
	{266, 278},
	{272, 285},
	{279, 292},
	{286, 299},
	{293, 306},
	{300, 314},
	{307, 321},
	{315, 329},
	{322, 337},
	{330, 345},
	{338, 354},
	{346, 363},
	{355, 371},
	{364, 380},
	{372, 390},
	{381, 399},
	{391, 409},
	{400, 419},
	{410, 429},
	{420, 440},
	{430, 451},
	{441, 462},
	{452, 473},
	{463, 484},
	{474, 496},
	{485, 508},
	{497, 521},
	{509, 533},
	{522, 547},
	{534, 560},
	{548, 574},
	{561, 588},
	{575, 602},
	{589, 617},
	{603, 632},
	{618, 647},
	{633, 663},
	{648, 679},
	{664, 696},
	{680, 713},
	{697, 730},
	{714, 748},
	{731, 766},
	{749, 785},
	{767, 804},
	{786, 824},
	{805, 844},
	{825, 864},
	{845, 886},
	{865, 907},
	{887, 929},
	{908, 952},
	{930, 975},
	{953, 999},
	{976, 1023}  
};


static const float mel_basis[MEL_ROWS][MAX_NZ_ELEMS] = {
	{
		0.014289,
		0.028578,
		0.042666,
		0.028378,
		0.014089,
	},
	{
		0.000100,
		0.014389,
		0.028678,
		0.042566,
		0.028277,
		0.013989,
	},
	{
		0.000200,
		0.014489,
		0.028778,
		0.042466,
		0.028177,
		0.013888,
	},
	{
		0.000300,
		0.014589,
		0.028878,
		0.042366,
		0.028077,
		0.013788,
	},
	{
		0.000400,
		0.014689,
		0.028978,
		0.042266,
		0.027977,
		0.013688,
	},
	{
		0.000501,
		0.014789,
		0.029078,
		0.042166,
		0.027877,
		0.013588,
	},
	{
		0.000601,
		0.014890,
		0.029178,
		0.042066,
		0.027777,
		0.013488,
	},
	{
		0.000701,
		0.014990,
		0.029279,
		0.041966,
		0.027677,
		0.013388,
	},
	{
		0.000801,
		0.015090,
		0.029379,
		0.041865,
		0.027577,
		0.013288,
	},
	{
		0.000901,
		0.015190,
		0.029479,
		0.041765,
		0.027476,
		0.013188,
	},
	{
		0.001001,
		0.015290,
		0.029579,
		0.041665,
		0.027376,
		0.013087,
	},
	{
		0.001101,
		0.015390,
		0.029679,
		0.041565,
		0.027276,
		0.012987,
	},
	{
		0.001201,
		0.015490,
		0.029779,
		0.041465,
		0.027176,
		0.012887,
	},
	{
		0.001302,
		0.015590,
		0.029879,
		0.041365,
		0.027076,
		0.012787,
	},
	{
		0.001402,
		0.015691,
		0.029979,
		0.041265,
		0.026976,
		0.012687,
	},
	{
		0.001502,
		0.015791,
		0.030080,
		0.041165,
		0.026876,
		0.012587,
	},
	{
		0.001602,
		0.015891,
		0.030180,
		0.041065,
		0.026776,
		0.012487,
	},
	{
		0.001702,
		0.015991,
		0.030280,
		0.040964,
		0.026676,
		0.012387,
	},
	{
		0.001802,
		0.016091,
		0.030380,
		0.040864,
		0.026575,
		0.012287,
	},
	{
		0.001902,
		0.016191,
		0.030480,
		0.040764,
		0.026475,
		0.012186,
	},
	{
		0.002002,
		0.016291,
		0.030580,
		0.040664,
		0.026375,
		0.012086,
	},
	{
		0.002102,
		0.016391,
		0.030680,
		0.040564,
		0.026275,
		0.011986,
	},
	{
		0.002203,
		0.016491,
		0.030780,
		0.040464,
		0.026175,
		0.011886,
	},
	{
		0.002303,
		0.016592,
		0.030881,
		0.040364,
		0.026075,
		0.011786,
	},
	{
		0.002403,
		0.016692,
		0.030981,
		0.040264,
		0.025975,
		0.011686,
	},
	{
		0.002503,
		0.016792,
		0.031081,
		0.040163,
		0.025875,
		0.011586,
	},
	{
		0.002603,
		0.016892,
		0.031181,
		0.040063,
		0.025774,
		0.011486,
	},
	{
		0.002703,
		0.016992,
		0.031281,
		0.039963,
		0.025674,
		0.011385,
	},
	{
		0.002803,
		0.017092,
		0.031381,
		0.039863,
		0.025574,
		0.011285,
	},
	{
		0.002903,
		0.017192,
		0.031481,
		0.039763,
		0.025474,
		0.011185,
	},
	{
		0.003004,
		0.017292,
		0.031581,
		0.039663,
		0.025374,
		0.011085,
	},
	{
		0.003104,
		0.017393,
		0.031681,
		0.039563,
		0.025274,
		0.010985,
	},
	{
		0.003204,
		0.017493,
		0.031782,
		0.039463,
		0.025174,
		0.010885,
	},
	{
		0.003304,
		0.017593,
		0.031882,
		0.039363,
		0.025074,
		0.010785,
	},
	{
		0.003404,
		0.017693,
		0.031982,
		0.039262,
		0.024974,
		0.010685,
	},
	{
		0.003504,
		0.017793,
		0.032082,
		0.039162,
		0.024873,
		0.010585,
	},
	{
		0.003604,
		0.017893,
		0.032182,
		0.039062,
		0.024773,
		0.010484,
	},
	{
		0.003704,
		0.017993,
		0.032282,
		0.038962,
		0.024673,
		0.010384,
	},
	{
		0.003804,
		0.018093,
		0.032382,
		0.038862,
		0.024573,
		0.010284,
	},
	{
		0.003905,
		0.018194,
		0.032482,
		0.038762,
		0.024473,
		0.010184,
	},
	{
		0.004005,
		0.018294,
		0.032583,
		0.038662,
		0.024373,
		0.010084,
	},
	{
		0.004089,
		0.018321,
		0.032553,
		0.038441,
		0.024322,
		0.010203,
	},
	{
		0.004055,
		0.017833,
		0.031611,
		0.037897,
		0.024667,
		0.011436,
	},
	{
		0.003554,
		0.016365,
		0.029177,
		0.038554,
		0.026048,
		0.013542,
		0.001035,
	},
	{
		0.001656,
		0.013864,
		0.026072,
		0.038281,
		0.028361,
		0.016443,
		0.004526,
	},
	{
		0.010670,
		0.022304,
		0.033937,
		0.031312,
		0.019956,
		0.008599,
	},
	{
		0.006876,
		0.017961,
		0.029047,
		0.034814,
		0.023993,
		0.013171,
		0.002350,
	},
	{
		0.002564,
		0.013128,
		0.023692,
		0.034255,
		0.028476,
		0.018164,
		0.007852,
	},
	{
		0.007881,
		0.017947,
		0.028013,
		0.033335,
		0.023508,
		0.013682,
		0.003855,
	},
	{
		0.002288,
		0.011880,
		0.021473,
		0.031065,
		0.029138,
		0.019775,
		0.010411,
		0.001047,
	},
	{
		0.005555,
		0.014695,
		0.023836,
		0.032977,
		0.026073,
		0.017150,
		0.008227,
	},
	{
		0.007737,
		0.016447,
		0.025157,
		0.032526,
		0.024023,
		0.015520,
		0.007017,
	},
	{
		0.000647,
		0.008947,
		0.017247,
		0.025548,
		0.030982,
		0.022880,
		0.014777,
		0.006675,
	},
	{
		0.001382,
		0.009291,
		0.017201,
		0.025110,
		0.030266,
		0.022545,
		0.014824,
		0.007103,
	},
	{
		0.001328,
		0.008865,
		0.016402,
		0.023939,
		0.030283,
		0.022926,
		0.015568,
		0.008211,
		0.000854,
	},
	{
		0.000575,
		0.007757,
		0.014939,
		0.022122,
		0.029304,
		0.023939,
		0.016928,
		0.009917,
		0.002906,
	},
	{
		0.006050,
		0.012894,
		0.019738,
		0.026582,
		0.025507,
		0.018826,
		0.012145,
		0.005464,
	},
	{
		0.003818,
		0.010340,
		0.016862,
		0.023384,
		0.027559,
		0.021192,
		0.014826,
		0.008460,
		0.002093,
	},
	{
		0.001132,
		0.007346,
		0.013561,
		0.019776,
		0.025990,
		0.023962,
		0.017895,
		0.011829,
		0.005762,
	},
	{
		0.003975,
		0.009897,
		0.015819,
		0.021741,
		0.027075,
		0.021295,
		0.015514,
		0.009733,
		0.003952,
	},
	{
		0.000283,
		0.005927,
		0.011570,
		0.017213,
		0.022856,
		0.024971,
		0.019462,
		0.013953,
		0.008444,
		0.002935,
	},
	{
		0.001702,
		0.007079,
		0.012457,
		0.017834,
		0.023212,
		0.023625,
		0.018376,
		0.013126,
		0.007877,
		0.002627,
	},
	{
		0.002394,
		0.007518,
		0.012642,
		0.017767,
		0.022891,
		0.022957,
		0.017955,
		0.012953,
		0.007951,
		0.002949,
	},
	{
		0.002439,
		0.007322,
		0.012205,
		0.017088,
		0.021971,
		0.022892,
		0.018126,
		0.013359,
		0.008592,
		0.003826,
	},
	{
		0.001910,
		0.006564,
		0.011217,
		0.015870,
		0.020523,
		0.023361,
		0.018818,
		0.014276,
		0.009734,
		0.005191,
		0.000649,
	},
	{
		0.000875,
		0.005310,
		0.009744,
		0.014178,
		0.018612,
		0.023046,
		0.019970,
		0.015641,
		0.011313,
		0.006984,
		0.002656,
	},
	{
		0.003621,
		0.007847,
		0.012072,
		0.016297,
		0.020523,
		0.021522,
		0.017397,
		0.013272,
		0.009148,
		0.005023,
		0.000898,
	},
	{
		0.001556,
		0.005582,
		0.009608,
		0.013635,
		0.017661,
		0.021687,
		0.019490,
		0.015560,
		0.011629,
		0.007699,
		0.003769,
	},
	{
		0.003001,
		0.006838,
		0.010675,
		0.014511,
		0.018348,
		0.021873,
		0.018127,
		0.014382,
		0.010637,
		0.006891,
		0.003146,
	},
	{
		0.000150,
		0.003807,
		0.007463,
		0.011119,
		0.014775,
		0.018431,
		0.020931,
		0.017362,
		0.013793,
		0.010224,
		0.006655,
		0.003086,
	},
	{
		0.000558,
		0.004042,
		0.007526,
		0.011009,
		0.014493,
		0.017977,
		0.020529,
		0.017128,
		0.013728,
		0.010327,
		0.006926,
		0.003525,
		0.000124,
	},
	{
		0.000449,
		0.003769,
		0.007089,
		0.010409,
		0.013729,
		0.017049,
		0.020369,
		0.017367,
		0.014126,
		0.010885,
		0.007644,
		0.004403,
		0.001163,
	},
	{
		0.003049,
		0.006212,
		0.009376,
		0.012539,
		0.015703,
		0.018867,
		0.018021,
		0.014933,
		0.011845,
		0.008756,
		0.005668,
		0.002580,
	},
	{
		0.001933,
		0.004948,
		0.007963,
		0.010977,
		0.013992,
		0.017007,
		0.019040,
		0.016098,
		0.013155,
		0.010212,
		0.007269,
		0.004326,
		0.001383,
	},
	{
		0.000473,
		0.003346,
		0.006218,
		0.009091,
		0.011964,
		0.014837,
		0.017709,
		0.017574,
		0.014770,
		0.011965,
		0.009161,
		0.006357,
		0.003552,
		0.000748,
	},
	{
		0.001451,
		0.004188,
		0.006926,
		0.009663,
		0.012400,
		0.015138,
		0.017875,
		0.016646,
		0.013974,
		0.011302,
		0.008630,
		0.005957,
		0.003285,
		0.000613,
	},
	{
		0.001913,
		0.004521,
		0.007130,
		0.009738,
		0.012347,
		0.014956,
		0.017564,
		0.016200,
		0.013654,
		0.011107,
		0.008561,
		0.006014,
		0.003468,
		0.000921,
	},
	{
		0.001916,
		0.004401,
		0.006887,
		0.009373,
		0.011859,
		0.014344,
		0.016830,
		0.016181,
		0.013755,
		0.011328,
		0.008902,
		0.006475,
		0.004048,
		0.001622,
	},
	{
		0.001512,
		0.003880,
		0.006249,
		0.008618,
		0.010987,
		0.013355,
		0.015724,
		0.016541,
		0.014228,
		0.011916,
		0.009604,
		0.007291,
		0.004979,
		0.002667,
		0.000355,
	},
	{
		0.000748,
		0.003006,
		0.005263,
		0.007520,
		0.009777,
		0.012034,
		0.014292,
		0.016549,
		0.015029,
		0.012826,
		0.010622,
		0.008419,
		0.006216,
		0.004012,
		0.001809,
	},
	{
		0.001821,
		0.003972,
		0.006123,
		0.008274,
		0.010425,
		0.012576,
		0.014727,
		0.016116,
		0.014017,
		0.011917,
		0.009817,
		0.007718,
		0.005618,
		0.003518,
		0.001419,
	},
	{
		0.000367,
		0.002417,
		0.004466,
		0.006516,
		0.008566,
		0.010615,
		0.012665,
		0.014715,
		0.015450,
		0.013450,
		0.011449,
		0.009448,
		0.007447,
		0.005446,
		0.003446,
		0.001445,
	},
	{
		0.000633,
		0.002587,
		0.004540,
		0.006493,
		0.008446,
		0.010399,
		0.012352,
		0.014305,
		0.015186,
		0.013279,
		0.011373,
		0.009466,
		0.007560,
		0.005653,
		0.003746,
		0.001840,
	},
	{
		0.000517,
		0.002378,
		0.004240,
		0.006101,
		0.007962,
		0.009823,
		0.011684,
		0.013545,
		0.015278,
		0.013461,
		0.011644,
		0.009827,
		0.008010,
		0.006194,
		0.004377,
		0.002560,
		0.000743,
	},
	{
		0.000062,
		0.001836,
		0.003609,
		0.005383,
		0.007156,
		0.008930,
		0.010703,
		0.012477,
		0.014250,
		0.013953,
		0.012221,
		0.010490,
		0.008759,
		0.007028,
		0.005296,
		0.003565,
		0.001834,
		0.000102,
	},
	{
		0.000999,
		0.002689,
		0.004379,
		0.006069,
		0.007759,
		0.009449,
		0.011139,
		0.012829,
		0.014519,
		0.013067,
		0.011417,
		0.009767,
		0.008118,
		0.006468,
		0.004818,
		0.003168,
		0.001518,
	},
	{
		0.001515,
		0.003126,
		0.004736,
		0.006347,
		0.007957,
		0.009568,
		0.011178,
		0.012788,
		0.014146,
		0.012574,
		0.011001,
		0.009429,
		0.007857,
		0.006285,
		0.004713,
		0.003141,
		0.001569,
	},
	{
		0.000122,
		0.001657,
		0.003191,
		0.004726,
		0.006261,
		0.007795,
		0.009330,
		0.010865,
		0.012399,
		0.013928,
		0.012430,
		0.010931,
		0.009433,
		0.007935,
		0.006437,
		0.004939,
		0.003441,
		0.001943,
		0.000445,
	},
	{
		0.000003,
		0.001465,
		0.002928,
		0.004390,
		0.005853,
		0.007315,
		0.008777,
		0.010240,
		0.011702,
		0.013164,
		0.012595,
		0.011168,
		0.009740,
		0.008313,
		0.006885,
		0.005458,
		0.004030,
		0.002603,
		0.001175,
	},
	{
		0.000980,
		0.002373,
		0.003767,
		0.005160,
		0.006554,
		0.007947,
		0.009341,
		0.010734,
		0.012128,
		0.013034,
		0.011674,
		0.010313,
		0.008953,
		0.007593,
		0.006233,
		0.004872,
		0.003512,
		0.002152,
		0.000791,
	},
	{
		0.000235,
		0.001563,
		0.002891,
		0.004219,
		0.005547,
		0.006875,
		0.008202,
		0.009530,
		0.010858,
		0.012186,
		0.012416,
		0.011120,
		0.009824,
		0.008527,
		0.007231,
		0.005935,
		0.004639,
		0.003342,
		0.002046,
		0.000750,
	},
	{
		0.000529,
		0.001795,
		0.003060,
		0.004326,
		0.005591,
		0.006856,
		0.008122,
		0.009387,
		0.010653,
		0.011918,
		0.012129,
		0.010894,
		0.009658,
		0.008423,
		0.007188,
		0.005953,
		0.004718,
		0.003482,
		0.002247,
		0.001012,
	},
	{
		0.000508,
		0.001714,
		0.002920,
		0.004126,
		0.005332,
		0.006537,
		0.007743,
		0.008949,
		0.010155,
		0.011361,
		0.012135,
		0.010958,
		0.009781,
		0.008604,
		0.007427,
		0.006250,
		0.005073,
		0.003896,
		0.002719,
		0.001542,
		0.000365,
	},
	{
		0.000208,
		0.001357,
		0.002506,
		0.003655,
		0.004804,
		0.005953,
		0.007102,
		0.008251,
		0.009400,
		0.010549,
		0.011698,
		0.011280,
		0.010158,
		0.009037,
		0.007915,
		0.006793,
		0.005672,
		0.004550,
		0.003428,
		0.002307,
		0.001185,
		0.000063,
	},
	{
		0.000756,
		0.001851,
		0.002946,
		0.004041,
		0.005136,
		0.006230,
		0.007325,
		0.008420,
		0.009515,
		0.010610,
		0.011705,
		0.010759,
		0.009690,
		0.008621,
		0.007552,
		0.006483,
		0.005414,
		0.004345,
		0.003277,
		0.002208,
		0.001139,
		0.000070,
	},
	{
		0.000984,
		0.002028,
		0.003071,
		0.004115,
		0.005158,
		0.006201,
		0.007245,
		0.008288,
		0.009332,
		0.010375,
		0.011418,
		0.010535,
		0.009516,
		0.008498,
		0.007479,
		0.006461,
		0.005442,
		0.004424,
		0.003405,
		0.002387,
		0.001368,
		0.000350,
	},
	{
		0.000929,
		0.001923,
		0.002918,
		0.003912,
		0.004906,
		0.005900,
		0.006895,
		0.007889,
		0.008883,
		0.009877,
		0.010872,
		0.010576,
		0.009605,
		0.008634,
		0.007664,
		0.006693,
		0.005723,
		0.004752,
		0.003782,
		0.002811,
		0.001840,
		0.000870,
	},
	{
		0.000622,
		0.001570,
		0.002517,
		0.003465,
		0.004412,
		0.005359,
		0.006307,
		0.007254,
		0.008202,
		0.009149,
		0.010097,
		0.010850,
		0.009925,
		0.009000,
		0.008075,
		0.007150,
		0.006225,
		0.005301,
		0.004376,
		0.003451,
		0.002526,
		0.001601,
		0.000676,
	},
	{
		0.000094,
		0.000997,
		0.001899,
		0.002802,
		0.003705,
		0.004608,
		0.005511,
		0.006414,
		0.007316,
		0.008219,
		0.009122,
		0.010025,
		0.010448,
		0.009567,
		0.008685,
		0.007804,
		0.006923,
		0.006041,
		0.005160,
		0.004279,
		0.003397,
		0.002516,
		0.001635,
		0.000754,
	},
	{
		0.000231,
		0.001092,
		0.001952,
		0.002812,
		0.003673,
		0.004533,
		0.005393,
		0.006254,
		0.007114,
		0.007974,
		0.008835,
		0.009695,
		0.010309,
		0.009469,
		0.008629,
		0.007789,
		0.006949,
		0.006110,
		0.005270,
		0.004430,
		0.003590,
		0.002750,
		0.001910,
		0.001071,
		0.000231,
	},
	{
		0.000119,
		0.000939,
		0.001759,
		0.002578,
		0.003398,
		0.004218,
		0.005038,
		0.005858,
		0.006677,
		0.007497,
		0.008317,
		0.009137,
		0.009957,
		0.009601,
		0.008801,
		0.008001,
		0.007201,
		0.006400,
		0.005600,
		0.004800,
		0.003999,
		0.003199,
		0.002399,
		0.001599,
		0.000798,
	},
	{
		0.000567,
		0.001348,
		0.002129,
		0.002910,
		0.003691,
		0.004473,
		0.005254,
		0.006035,
		0.006816,
		0.007598,
		0.008379,
		0.009160,
		0.009937,
		0.009175,
		0.008412,
		0.007650,
		0.006887,
		0.006124,
		0.005362,
		0.004599,
		0.003837,
		0.003074,
		0.002311,
		0.001549,
		0.000786,
		0.000024,
	},
	{
		0.000002,
		0.000746,
		0.001491,
		0.002235,
		0.002980,
		0.003724,
		0.004468,
		0.005213,
		0.005957,
		0.006702,
		0.007446,
		0.008191,
		0.008935,
		0.009680,
		0.008998,
		0.008272,
		0.007545,
		0.006818,
		0.006092,
		0.005365,
		0.004638,
		0.003911,
		0.003185,
		0.002458,
		0.001731,
		0.001005,
		0.000278,
	},
	{
		0.000687,
		0.001397,
		0.002106,
		0.002816,
		0.003525,
		0.004234,
		0.004944,
		0.005653,
		0.006363,
		0.007072,
		0.007781,
		0.008491,
		0.009200,
		0.009044,
		0.008351,
		0.007659,
		0.006966,
		0.006274,
		0.005581,
		0.004889,
		0.004196,
		0.003504,
		0.002811,
		0.002119,
		0.001426,
		0.000734,
		0.000041,
	},
	{
		0.000417,
		0.001093,
		0.001769,
		0.002445,
		0.003121,
		0.003797,
		0.004473,
		0.005149,
		0.005825,
		0.006501,
		0.007177,
		0.007853,
		0.008529,
		0.009205,
		0.008625,
		0.007965,
		0.007306,
		0.006646,
		0.005986,
		0.005326,
		0.004666,
		0.004006,
		0.003346,
		0.002686,
		0.002026,
		0.001367,
		0.000707,
		0.000047,
	},
	{
		0.000606,
		0.001250,
		0.001894,
		0.002538,
		0.003182,
		0.003826,
		0.004471,
		0.005115,
		0.005759,
		0.006403,
		0.007047,
		0.007691,
		0.008336,
		0.008980,
		0.008441,
		0.007812,
		0.007184,
		0.006555,
		0.005926,
		0.005297,
		0.004668,
		0.004040,
		0.003411,
		0.002782,
		0.002153,
		0.001524,
		0.000896,
		0.000267,
	},
	{
		0.000570,
		0.001184,
		0.001798,
		0.002412,
		0.003026,
		0.003639,
		0.004253,
		0.004867,
		0.005481,
		0.006095,
		0.006709,
		0.007322,
		0.007936,
		0.008550,
		0.008465,
		0.007866,
		0.007267,
		0.006668,
		0.006068,
		0.005469,
		0.004870,
		0.004271,
		0.003672,
		0.003072,
		0.002473,
		0.001874,
		0.001275,
		0.000676,
		0.000076,
	},
	{
		0.000337,
		0.000922,
		0.001507,
		0.002092,
		0.002677,
		0.003261,
		0.003846,
		0.004431,
		0.005016,
		0.005601,
		0.006186,
		0.006771,
		0.007356,
		0.007941,
		0.008526,
		0.008102,
		0.007531,
		0.006960,
		0.006389,
		0.005818,
		0.005247,
		0.004676,
		0.004105,
		0.003534,
		0.002963,
		0.002392,
		0.001821,
		0.001250,
		0.000679,
		0.000108,
	},
	{
		0.000486,
		0.001044,
		0.001601,
		0.002158,
		0.002716,
		0.003273,
		0.003831,
		0.004388,
		0.004945,
		0.005503,
		0.006060,
		0.006618,
		0.007175,
		0.007732,
		0.008290,
		0.007955,
		0.007411,
		0.006867,
		0.006322,
		0.005778,
		0.005234,
		0.004690,
		0.004146,
		0.003602,
		0.003058,
		0.002514,
		0.001970,
		0.001426,
		0.000881,
		0.000337,
	},
	{
		0.000430,
		0.000961,
		0.001493,
		0.002024,
		0.002555,
		0.003086,
		0.003617,
		0.004148,
		0.004679,
		0.005211,
		0.005742,
		0.006273,
		0.006804,
		0.007335,
		0.007866,
		0.007998,
		0.007480,
		0.006961,
		0.006443,
		0.005925,
		0.005406,
		0.004888,
		0.004369,
		0.003851,
		0.003332,
		0.002814,
		0.002295,
		0.001777,
		0.001258,
		0.000740,
		0.000221,
	},
	{
		0.000192,
		0.000698,
		0.001205,
		0.001711,
		0.002217,
		0.002723,
		0.003229,
		0.003735,
		0.004241,
		0.004748,
		0.005254,
		0.005760,
		0.006266,
		0.006772,
		0.007278,
		0.007784,
		0.007717,
		0.007223,
		0.006729,
		0.006235,
		0.005741,
		0.005247,
		0.004752,
		0.004258,
		0.003764,
		0.003270,
		0.002776,
		0.002282,
		0.001788,
		0.001294,
		0.000800,
		0.000306,
	},
	{
		0.000277,
		0.000759,
		0.001241,
		0.001724,
		0.002206,
		0.002688,
		0.003170,
		0.003653,
		0.004135,
		0.004617,
		0.005100,
		0.005582,
		0.006064,
		0.006547,
		0.007029,
		0.007511,
		0.007630,
		0.007159,
		0.006689,
		0.006218,
		0.005747,
		0.005276,
		0.004805,
		0.004335,
		0.003864,
		0.003393,
		0.002922,
		0.002451,
		0.001980,
		0.001510,
		0.001039,
		0.000568,
		0.000097,
	},
	{
		0.000175,
		0.000635,
		0.001094,
		0.001554,
		0.002014,
		0.002473,
		0.002933,
		0.003392,
		0.003852,
		0.004312,
		0.004771,
		0.005231,
		0.005690,
		0.006150,
		0.006609,
		0.007069,
		0.007529,
		0.007268,
		0.006819,
		0.006370,
		0.005922,
		0.005473,
		0.005024,
		0.004576,
		0.004127,
		0.003678,
		0.003230,
		0.002781,
		0.002332,
		0.001884,
		0.001435,
		0.000987,
		0.000538,
		0.000089,
	},
	{
		0.000348,
		0.000785,
		0.001223,
		0.001661,
		0.002099,
		0.002537,
		0.002975,
		0.003413,
		0.003851,
		0.004289,
		0.004727,
		0.005165,
		0.005603,
		0.006041,
		0.006479,
		0.006917,
		0.007355,
		0.007099,
		0.006672,
		0.006244,
		0.005817,
		0.005389,
		0.004962,
		0.004534,
		0.004107,
		0.003679,
		0.003252,
		0.002824,
		0.002397,
		0.001969,
		0.001542,
		0.001114,
		0.000687,
		0.000259,
	},
	{
		0.000334,
		0.000752,
		0.001169,
		0.001586,
		0.002004,
		0.002421,
		0.002838,
		0.003256,
		0.003673,
		0.004090,
		0.004508,
		0.004925,
		0.005342,
		0.005760,
		0.006177,
		0.006594,
		0.007012,
		0.007104,
		0.006697,
		0.006289,
		0.005882,
		0.005475,
		0.005067,
		0.004660,
		0.004252,
		0.003845,
		0.003438,
		0.003030,
		0.002623,
		0.002215,
		0.001808,
		0.001401,
		0.000993,
		0.000586,
		0.000179,
	},
	{
		0.000157,
		0.000554,
		0.000952,
		0.001350,
		0.001747,
		0.002145,
		0.002543,
		0.002940,
		0.003338,
		0.003736,
		0.004133,
		0.004531,
		0.004929,
		0.005327,
		0.005724,
		0.006122,
		0.006520,
		0.006917,
		0.006873,
		0.006485,
		0.006097,
		0.005709,
		0.005321,
		0.004932,
		0.004544,
		0.004156,
		0.003768,
		0.003380,
		0.002991,
		0.002603,
		0.002215,
		0.001827,
		0.001438,
		0.001050,
		0.000662,
		0.000274,
	},
	{
		0.000213,
		0.000592,
		0.000971,
		0.001350,
		0.001729,
		0.002108,
		0.002487,
		0.002866,
		0.003245,
		0.003624,
		0.004003,
		0.004381,
		0.004760,
		0.005139,
		0.005518,
		0.005897,
		0.006276,
		0.006655,
		0.006814,
		0.006444,
		0.006074,
		0.005704,
		0.005334,
		0.004964,
		0.004594,
		0.004224,
		0.003854,
		0.003484,
		0.003114,
		0.002744,
		0.002374,
		0.002004,
		0.001635,
		0.001265,
		0.000895,
		0.000525,
		0.000155,
	},
	{
		0.000106,
		0.000467,
		0.000829,
		0.001190,
		0.001551,
		0.001912,
		0.002273,
		0.002634,
		0.002995,
		0.003356,
		0.003718,
		0.004079,
		0.004440,
		0.004801,
		0.005162,
		0.005523,
		0.005884,
		0.006245,
		0.006607,
		0.006553,
		0.006200,
		0.005848,
		0.005495,
		0.005143,
		0.004790,
		0.004438,
		0.004085,
		0.003733,
		0.003380,
		0.003028,
		0.002675,
		0.002323,
		0.001970,
		0.001617,
		0.001265,
		0.000912,
		0.000560,
		0.000207,
	},
	{
		0.000200,
		0.000544,
		0.000888,
		0.001232,
		0.001577,
		0.001921,
		0.002265,
		0.002609,
		0.002953,
		0.003297,
		0.003641,
		0.003985,
		0.004329,
		0.004674,
		0.005018,
		0.005362,
		0.005706,
		0.006050,
		0.006394,
		0.006458,
		0.006122,
		0.005787,
		0.005451,
		0.005115,
		0.004779,
		0.004443,
		0.004107,
		0.003771,
		0.003435,
		0.003099,
		0.002763,
		0.002427,
		0.002091,
		0.001756,
		0.001420,
		0.001084,
		0.000748,
		0.000412,
		0.000076,
	},
	{
		0.000135,
		0.000463,
		0.000791,
		0.001119,
		0.001447,
		0.001775,
		0.002102,
		0.002430,
		0.002758,
		0.003086,
		0.003414,
		0.003742,
		0.004070,
		0.004398,
		0.004726,
		0.005054,
		0.005382,
		0.005709,
		0.006037,
		0.006365,
		0.006192,
		0.005872,
		0.005552,
		0.005231,
		0.004911,
		0.004591,
		0.004271,
		0.003951,
		0.003631,
		0.003311,
		0.002991,
		0.002671,
		0.002351,
		0.002030,
		0.001710,
		0.001390,
		0.001070,
		0.000750,
		0.000430,
		0.000110,
	},
	{
		0.000242,
		0.000554,
		0.000867,
		0.001179,
		0.001492,
		0.001804,
		0.002117,
		0.002429,
		0.002742,
		0.003054,
		0.003367,
		0.003679,
		0.003991,
		0.004304,
		0.004616,
		0.004929,
		0.005241,
		0.005554,
		0.005866,
		0.006179,
		0.006086,
		0.005781,
		0.005476,
		0.005171,
		0.004866,
		0.004561,
		0.004256,
		0.003951,
		0.003645,
		0.003340,
		0.003035,
		0.002730,
		0.002425,
		0.002120,
		0.001815,
		0.001510,
		0.001205,
		0.000900,
		0.000595,
		0.000290,
	},
	{
		0.000196,
		0.000493,
		0.000791,
		0.001089,
		0.001387,
		0.001684,
		0.001982,
		0.002280,
		0.002578,
		0.002875,
		0.003173,
		0.003471,
		0.003769,
		0.004066,
		0.004364,
		0.004662,
		0.004960,
		0.005258,
		0.005555,
		0.005853,
		0.006122,
		0.005831,
		0.005541,
		0.005250,
		0.004959,
		0.004669,
		0.004378,
		0.004087,
		0.003797,
		0.003506,
		0.003215,
		0.002925,
		0.002634,
		0.002343,
		0.002053,
		0.001762,
		0.001471,
		0.001181,
		0.000890,
		0.000599,
		0.000309,
		0.000018,
	},
	{
		0.000014,
		0.000298,
		0.000581,
		0.000865,
		0.001149,
		0.001433,
		0.001716,
		0.002000,
		0.002284,
		0.002568,
		0.002851,
		0.003135,
		0.003419,
		0.003702,
		0.003986,
		0.004270,
		0.004554,
		0.004837,
		0.005121,
		0.005405,
		0.005689,
		0.005972,
		0.005730,
		0.005453,
		0.005176,
		0.004899,
		0.004622,
		0.004345,
		0.004068,
		0.003791,
		0.003514,
		0.003238,
		0.002961,
		0.002684,
		0.002407,
		0.002130,
		0.001853,
		0.001576,
		0.001299,
		0.001022,
		0.000745,
		0.000468,
		0.000191,
	},
	{
		0.000254,
		0.000524,
		0.000794,
		0.001065,
		0.001335,
		0.001605,
		0.001876,
		0.002146,
		0.002417,
		0.002687,
		0.002957,
		0.003228,
		0.003498,
		0.003769,
		0.004039,
		0.004309,
		0.004580,
		0.004850,
		0.005120,
		0.005391,
		0.005661,
		0.005765,
		0.005501,
		0.005237,
		0.004973,
		0.004709,
		0.004445,
		0.004182,
		0.003918,
		0.003654,
		0.003390,
		0.003126,
		0.002862,
		0.002598,
		0.002334,
		0.002070,
		0.001806,
		0.001542,
		0.001278,
		0.001014,
		0.000750,
		0.000486,
		0.000222,
	},
	{
		0.000080,
		0.000338,
		0.000596,
		0.000853,
		0.001111,
		0.001368,
		0.001626,
		0.001884,
		0.002141,
		0.002399,
		0.002657,
		0.002914,
		0.003172,
		0.003430,
		0.003687,
		0.003945,
		0.004203,
		0.004460,
		0.004718,
		0.004976,
		0.005233,
		0.005491,
		0.005668,
		0.005417,
		0.005165,
		0.004914,
		0.004662,
		0.004411,
		0.004159,
		0.003908,
		0.003656,
		0.003405,
		0.003153,
		0.002902,
		0.002650,
		0.002399,
		0.002147,
		0.001896,
		0.001644,
		0.001393,
		0.001141,
		0.000890,
		0.000638,
		0.000387,
		0.000135,
	},
	{
		0.000039,
		0.000284,
		0.000530,
		0.000775,
		0.001021,
		0.001266,
		0.001512,
		0.001757,
		0.002003,
		0.002248,
		0.002494,
		0.002739,
		0.002985,
		0.003230,
		0.003476,
		0.003721,
		0.003967,
		0.004212,
		0.004458,
		0.004704,
		0.004949,
		0.005195,
		0.005440,
		0.005461,
		0.005222,
		0.004982,
		0.004742,
		0.004503,
		0.004263,
		0.004023,
		0.003784,
		0.003544,
		0.003304,
		0.003065,
		0.002825,
		0.002585,
		0.002345,
		0.002106,
		0.001866,
		0.001626,
		0.001387,
		0.001147,
		0.000907,
		0.000668,
		0.000428,
		0.000188,
	},
	{
		0.000108,
		0.000342,
		0.000576,
		0.000810,
		0.001044,
		0.001278,
		0.001512,
		0.001746,
		0.001980,
		0.002214,
		0.002448,
		0.002682,
		0.002916,
		0.003150,
		0.003384,
		0.003618,
		0.003852,
		0.004086,
		0.004319,
		0.004553,
		0.004787,
		0.005021,
		0.005255,
		0.005390,
		0.005162,
		0.004934,
		0.004705,
		0.004477,
		0.004249,
		0.004020,
		0.003792,
		0.003563,
		0.003335,
		0.003107,
		0.002878,
		0.002650,
		0.002421,
		0.002193,
		0.001965,
		0.001736,
		0.001508,
		0.001280,
		0.001051,
		0.000823,
		0.000594,
		0.000366,
		0.000138,
	},
	{
		0.000048,
		0.000271,
		0.000494,
		0.000716,
		0.000939,
		0.001162,
		0.001385,
		0.001608,
		0.001831,
		0.002054,
		0.002277,
		0.002500,
		0.002723,
		0.002946,
		0.003169,
		0.003392,
		0.003615,
		0.003838,
		0.004061,
		0.004284,
		0.004507,
		0.004729,
		0.004952,
		0.005175,
		0.005223,
		0.005006,
		0.004788,
		0.004570,
		0.004353,
		0.004135,
		0.003917,
		0.003700,
		0.003482,
		0.003264,
		0.003047,
		0.002829,
		0.002612,
		0.002394,
		0.002176,
		0.001959,
		0.001741,
		0.001523,
		0.001306,
		0.001088,
		0.000871,
		0.000653,
		0.000435,
		0.000218,
	}
};

#endif