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


//////////////////////////////////////////////////
// Author:          ESL team                    //
// Optimizations:   Dimitrios Samakovlis        //
/////////////////////////////////////////////////



#ifndef _FFT_LOOK_UP_TABLE_
#define _FFT_LOOK_UP_TABLE_

#include "fft.hpp"

static cn roots[FLOMB_MAX_SIZE >> 1] = {
{32768, 0}, {32767, 99}, {32767, 200}, {32766, 300}, {32765, 401}, {32764, 501}, {32762, 602}, {32760, 702}, {32758, 803}, {32755, 903}, {32752, 1004}, {32749, 1104}, {32745, 1205}, {32741, 1305}, {32737, 1406}, {32733, 1506}, {32728, 1607}, {32723, 1708}, {32718, 1808}, {32712, 1908}, {32706, 2008}, {32700, 2109}, {32693, 2209}, {32686, 2310}, {32679, 2409}, {32671, 2510}, {32663, 2610}, {32655, 2710}, {32647, 2810}, {32638, 2911}, {32629, 3010}, {32619, 3111}, {32610, 3211}, {32600, 3311}, {32589, 3411}, {32579, 3511}, {32568, 3611}, {32557, 3711}, {32545, 3811}, {32533, 3910}, {32521, 4010}, {32509, 4110}, {32496, 4210}, {32483, 4309}, {32469, 4409}, {32456, 4508}, {32442, 4608}, {32428, 4707}, {32413, 4807}, {32398, 4907}, {32383, 5006}, {32367, 5106}, {32351, 5204}, {32335, 5304}, {32319, 5403}, {32302, 5502}, {32285, 5601}, {32268, 5700}, {32250, 5799}, {32232, 5898}, {32214, 5997}, {32195, 6096}, {32177, 6194}, {32157, 6293}, {32138, 6392}, {32118, 6490}, {32098, 6589}, {32078, 6687}, {32057, 6786}, {32036, 6884}, {32015, 6983}, {31993, 7080}, {31971, 7179}, {31949, 7276}, {31927, 7375}, {31904, 7472}, {31881, 7570}, {31858, 7668}, {31834, 7766}, {31810, 7863}, {31786, 7961}, {31761, 8059}, {31736, 8156}, {31711, 8254}, {31686, 8350}, {31660, 8448}, {31634, 8545}, {31607, 8642}, {31581, 8738}, {31554, 8836}, {31527, 8932}, {31499, 9029}, {31471, 9125}, {31443, 9222}, {31415, 9318}, {31386, 9415}, {31357, 9512}, {31327, 9607}, {31298, 9704}, {31268, 9799}, {31237, 9896}, {31207, 9991}, {31176, 10087}, {31145, 10182}, {31114, 10278}, {31082, 10373}, {31050, 10469}, {31018, 10563}, {30985, 10659}, {30953, 10753}, {30919, 10849}, {30886, 10943}, {30852, 11038}, {30818, 11133}, {30784, 11227}, {30749, 11322}, {30714, 11416}, {30679, 11511}, {30644, 11604}, {30608, 11699}, {30572, 11792}, {30536, 11886}, {30499, 11979}, {30462, 12073}, {30425, 12166}, {30387, 12260}, {30350, 12352}, {30312, 12446}, {30273, 12539}, {30235, 12632}, {30196, 12725}, {30157, 12817}, {30117, 12910}, {30078, 13002}, {30037, 13094}, {29997, 13186}, {29956, 13278}, {29916, 13370}, {29874, 13462}, {29833, 13553}, {29791, 13645}, {29749, 13736}, {29707, 13827}, {29665, 13918}, {29622, 14009}, {29578, 14100}, {29535, 14191}, {29491, 14282}, {29448, 14372}, {29403, 14462}, {29359, 14552}, {29314, 14642}, {29269, 14732}, {29223, 14822}, {29178, 14911}, {29132, 15001}, {29086, 15090}, {29039, 15179}, {28993, 15268}, {28946, 15357}, {28898, 15446}, {28851, 15534}, {28803, 15623}, {28755, 15711}, {28707, 15800}, {28658, 15887}, {28609, 15975}, {28560, 16063}, {28511, 16151}, {28461, 16238}, {28411, 16325}, {28361, 16412}, {28310, 16499}, {28260, 16585}, {28209, 16673}, {28158, 16759}, {28106, 16845}, {28054, 16932}, {28002, 17017}, {27949, 17104}, {27897, 17189}, {27844, 17275}, {27791, 17360}, {27737, 17445}, {27684, 17530}, {27630, 17615}, {27576, 17699}, {27521, 17784}, {27467, 17868}, {27412, 17953}, {27357, 18036}, {27301, 18120}, {27245, 18204}, {27189, 18288}, {27133, 18371}, {27077, 18454}, {27020, 18537}, {26963, 18620}, {26905, 18703}, {26848, 18785}, {26790, 18867}, {26733, 18949}, {26674, 19031}, {26616, 19113}, {26557, 19195}, {26498, 19276}, {26439, 19357}, {26379, 19438}, {26319, 19519}, {26259, 19600}, {26199, 19680}, {26138, 19761}, {26078, 19840}, {26017, 19921}, {25956, 20000}, {25894, 20080}, {25832, 20159}, {25770, 20238}, {25708, 20317}, {25646, 20396}, {25583, 20474}, {25520, 20553}, {25457, 20631}, {25393, 20709}, {25329, 20787}, {25266, 20865}, {25201, 20942}, {25137, 21019}, {25073, 21097}, {25008, 21173}, {24943, 21250}, {24878, 21326}, {24812, 21403}, {24746, 21478}, {24680, 21554}, {24614, 21630}, {24547, 21705}, {24481, 21780}, {24414, 21855}, {24347, 21930}, {24279, 22005}, {24211, 22080}, {24144, 22153}, {24075, 22228}, {24007, 22301}, {23939, 22375}, {23870, 22448}, {23801, 22521}, {23732, 22594}, {23662, 22667}, {23593, 22739}, {23523, 22812}, {23453, 22883}, {23382, 22955}, {23312, 23027}, {23241, 23098}, {23170, 23170}, {23099, 23241}, {23027, 23312}, {22956, 23382}, {22884, 23453}, {22812, 23522}, {22740, 23592}, {22668, 23662}, {22594, 23732}, {22522, 23800}, {22448, 23870}, {22376, 23938}, {22302, 24007}, {22228, 24075}, {22154, 24143}, {22080, 24211}, {22005, 24279}, {21931, 24346}, {21856, 24413}, {21781, 24480}, {21706, 24547}, {21630, 24614}, {21555, 24680}, {21479, 24746}, {21403, 24811}, {21327, 24877}, {21251, 24942}, {21174, 25007}, {21097, 25072}, {21020, 25137}, {20943, 25201}, {20865, 25265}, {20787, 25330}, {20710, 25393}, {20631, 25457}, {20554, 25519}, {20475, 25583}, {20397, 25645}, {20318, 25708}, {20239, 25770}, {20160, 25832}, {20081, 25893}, {20001, 25955}, {19921, 26016}, {19841, 26077}, {19761, 26138}, {19681, 26199}, {19601, 26258}, {19520, 26319}, {19438, 26379}, {19358, 26438}, {19276, 26498}, {19195, 26556}, {19113, 26615}, {19032, 26673}, {18950, 26732}, {18868, 26790}, {18785, 26848}, {18703, 26905}, {18620, 26962}, {18538, 27019}, {18455, 27076}, {18372, 27132}, {18288, 27189}, {18204, 27245}, {18121, 27301}, {18037, 27356}, {17953, 27411}, {17869, 27466}, {17785, 27521}, {17700, 27576}, {17616, 27629}, {17530, 27684}, {17446, 27737}, {17360, 27791}, {17275, 27843}, {17189, 27897}, {17104, 27949}, {17018, 28001}, {16933, 28053}, {16846, 28105}, {16759, 28157}, {16673, 28208}, {16586, 28259}, {16500, 28310}, {16413, 28361}, {16326, 28411}, {16238, 28461}, {16151, 28510}, {16063, 28560}, {15976, 28609}, {15888, 28658}, {15800, 28706}, {15712, 28755}, {15624, 28803}, {15535, 28851}, {15446, 28898}, {15358, 28945}, {15269, 28993}, {15180, 29039}, {15090, 29086}, {15002, 29132}, {14912, 29178}, {14823, 29223}, {14732, 29269}, {14643, 29313}, {14553, 29358}, {14463, 29403}, {14372, 29447}, {14282, 29491}, {14191, 29535}, {14101, 29578}, {14010, 29621}, {13919, 29664}, {13828, 29707}, {13736, 29749}, {13646, 29791}, {13554, 29833}, {13463, 29874}, {13370, 29915}, {13279, 29956}, {13187, 29997}, {13095, 30037}, {13002, 30077}, {12911, 30117}, {12818, 30156}, {12726, 30195}, {12632, 30234}, {12539, 30273}, {12447, 30311}, {12353, 30350}, {12261, 30387}, {12167, 30425}, {12074, 30462}, {11980, 30499}, {11887, 30535}, {11793, 30572}, {11699, 30608}, {11605, 30644}, {11511, 30679}, {11417, 30714}, {11323, 30749}, {11228, 30784}, {11134, 30818}, {11039, 30852}, {10944, 30886}, {10850, 30919}, {10754, 30952}, {10660, 30985}, {10564, 31018}, {10469, 31050}, {10374, 31082}, {10279, 31113}, {10183, 31145}, {10088, 31176}, {9992, 31207}, {9896, 31237}, {9800, 31268}, {9705, 31297}, {9608, 31327}, {9511, 31357}, {9416, 31385}, {9319, 31414}, {9223, 31443}, {9126, 31471}, {9030, 31499}, {8933, 31526}, {8837, 31553}, {8739, 31580}, {8643, 31607}, {8545, 31633}, {8449, 31659}, {8351, 31685}, {8254, 31711}, {8157, 31736}, {8060, 31761}, {7962, 31785}, {7864, 31810}, {7767, 31834}, {7669, 31857}, {7571, 31881}, {7473, 31904}, {7376, 31927}, {7277, 31949}, {7180, 31971}, {7081, 31993}, {6983, 32015}, {6885, 32036}, {6787, 32057}, {6688, 32078}, {6590, 32098}, {6491, 32118}, {6392, 32138}, {6294, 32157}, {6195, 32177}, {6097, 32195}, {5997, 32214}, {5899, 32232}, {5800, 32250}, {5701, 32268}, {5602, 32285}, {5503, 32302}, {5404, 32319}, {5305, 32335}, {5205, 32351}, {5106, 32367}, {5007, 32383}, {4908, 32398}, {4808, 32413}, {4708, 32427}, {4609, 32442}, {4509, 32456}, {4410, 32469}, {4310, 32483}, {4211, 32496}, {4110, 32509}, {4011, 32521}, {3911, 32533}, {3812, 32545}, {3711, 32557}, {3612, 32568}, {3512, 32579}, {3412, 32589}, {3312, 32600}, {3211, 32610}, {3112, 32619}, {3011, 32629}, {2912, 32638}, {2811, 32647}, {2711, 32655}, {2611, 32663}, {2511, 32671}, {2410, 32679}, {2310, 32686}, {2210, 32693}, {2110, 32699}, {2009, 32706}, {1909, 32712}, {1808, 32718}, {1709, 32723}, {1608, 32728}, {1507, 32733}, {1407, 32737}, {1306, 32741}, {1206, 32745}, {1105, 32749}, {1005, 32752}, {904, 32755}, {804, 32758}, {703, 32760}, {603, 32762}, {502, 32764}, {402, 32765}, {301, 32766}, {201, 32767}, {100, 32767}, {0, 32768}, {-100, 32767}, {-201, 32767}, {-301, 32766}, {-402, 32765}, {-502, 32764}, {-603, 32762}, {-703, 32760}, {-804, 32758}, {-904, 32755}, {-1004, 32752}, {-1104, 32749}, {-1205, 32745}, {-1305, 32741}, {-1406, 32737}, {-1506, 32733}, {-1607, 32728}, {-1708, 32723}, {-1808, 32718}, {-1909, 32712}, {-2008, 32706}, {-2109, 32700}, {-2209, 32693}, {-2310, 32686}, {-2409, 32679}, {-2510, 32671}, {-2610, 32663}, {-2711, 32655}, {-2810, 32647}, {-2911, 32638}, {-3010, 32629}, {-3111, 32619}, {-3211, 32610}, {-3311, 32600}, {-3411, 32589}, {-3511, 32579}, {-3611, 32568}, {-3711, 32557}, {-3811, 32545}, {-3910, 32533}, {-4011, 32521}, {-4110, 32509}, {-4210, 32496}, {-4309, 32483}, {-4409, 32469}, {-4508, 32456}, {-4608, 32442}, {-4707, 32428}, {-4807, 32413}, {-4907, 32398}, {-5006, 32383}, {-5106, 32367}, {-5205, 32351}, {-5304, 32335}, {-5403, 32319}, {-5502, 32302}, {-5601, 32285}, {-5700, 32268}, {-5799, 32250}, {-5898, 32232}, {-5997, 32214}, {-6096, 32195}, {-6194, 32177}, {-6293, 32157}, {-6392, 32138}, {-6490, 32118}, {-6589, 32098}, {-6687, 32078}, {-6786, 32057}, {-6884, 32036}, {-6983, 32015}, {-7080, 31993}, {-7179, 31971}, {-7276, 31949}, {-7375, 31927}, {-7472, 31904}, {-7571, 31881}, {-7668, 31858}, {-7766, 31834}, {-7863, 31810}, {-7961, 31786}, {-8059, 31761}, {-8156, 31736}, {-8254, 31711}, {-8350, 31686}, {-8448, 31660}, {-8545, 31634}, {-8642, 31607}, {-8739, 31581}, {-8836, 31554}, {-8932, 31526}, {-9029, 31499}, {-9125, 31471}, {-9222, 31443}, {-9318, 31415}, {-9415, 31386}, {-9512, 31356}, {-9607, 31327}, {-9704, 31298}, {-9799, 31268}, {-9896, 31237}, {-9991, 31207}, {-10087, 31176}, {-10182, 31145}, {-10278, 31114}, {-10373, 31082}, {-10469, 31050}, {-10564, 31018}, {-10659, 30985}, {-10754, 30953}, {-10849, 30919}, {-10943, 30886}, {-11038, 30852}, {-11133, 30818}, {-11227, 30784}, {-11322, 30749}, {-11416, 30714}, {-11511, 30679}, {-11604, 30644}, {-11699, 30608}, {-11792, 30572}, {-11886, 30536}, {-11979, 30499}, {-12073, 30462}, {-12166, 30425}, {-12260, 30387}, {-12353, 30350}, {-12446, 30312}, {-12539, 30273}, {-12632, 30235}, {-12725, 30196}, {-12817, 30157}, {-12910, 30117}, {-13002, 30077}, {-13094, 30037}, {-13186, 29997}, {-13278, 29956}, {-13370, 29916}, {-13462, 29874}, {-13553, 29833}, {-13645, 29791}, {-13736, 29749}, {-13827, 29707}, {-13918, 29665}, {-14009, 29622}, {-14101, 29578}, {-14191, 29535}, {-14282, 29491}, {-14372, 29447}, {-14462, 29403}, {-14552, 29359}, {-14642, 29314}, {-14732, 29269}, {-14822, 29223}, {-14911, 29178}, {-15001, 29132}, {-15090, 29086}, {-15179, 29039}, {-15268, 28993}, {-15357, 28946}, {-15446, 28898}, {-15535, 28851}, {-15623, 28803}, {-15711, 28755}, {-15800, 28707}, {-15887, 28658}, {-15976, 28609}, {-16063, 28560}, {-16151, 28511}, {-16238, 28461}, {-16325, 28411}, {-16412, 28361}, {-16499, 28310}, {-16586, 28260}, {-16673, 28209}, {-16759, 28157}, {-16845, 28106}, {-16932, 28054}, {-17017, 28002}, {-17104, 27949}, {-17189, 27897}, {-17275, 27844}, {-17360, 27791}, {-17445, 27737}, {-17530, 27684}, {-17615, 27630}, {-17699, 27576}, {-17784, 27521}, {-17868, 27467}, {-17953, 27412}, {-18036, 27357}, {-18121, 27301}, {-18205, 27245}, {-18288, 27189}, {-18371, 27133}, {-18454, 27077}, {-18537, 27020}, {-18620, 26963}, {-18703, 26905}, {-18785, 26848}, {-18868, 26790}, {-18949, 26732}, {-19032, 26674}, {-19113, 26616}, {-19195, 26557}, {-19276, 26498}, {-19357, 26438}, {-19438, 26379}, {-19519, 26319}, {-19600, 26259}, {-19680, 26199}, {-19761, 26138}, {-19841, 26078}, {-19921, 26016}, {-20000, 25956}, {-20080, 25894}, {-20159, 25832}, {-20239, 25770}, {-20317, 25708}, {-20396, 25645}, {-20474, 25583}, {-20553, 25520}, {-20631, 25457}, {-20709, 25393}, {-20787, 25329}, {-20865, 25266}, {-20942, 25201}, {-21019, 25137}, {-21097, 25072}, {-21173, 25008}, {-21250, 24943}, {-21326, 24878}, {-21403, 24812}, {-21478, 24746}, {-21554, 24680}, {-21630, 24614}, {-21705, 24547}, {-21780, 24481}, {-21856, 24414}, {-21930, 24347}, {-22005, 24279}, {-22080, 24211}, {-22153, 24144}, {-22228, 24075}, {-22301, 24007}, {-22375, 23939}, {-22448, 23870}, {-22521, 23801}, {-22594, 23732}, {-22667, 23662}, {-22739, 23593}, {-22812, 23523}, {-22883, 23453}, {-22956, 23382}, {-23027, 23312}, {-23099, 23241}, {-23170, 23170}, {-23241, 23099}, {-23312, 23027}, {-23382, 22956}, {-23453, 22884}, {-23522, 22812}, {-23593, 22740}, {-23662, 22667}, {-23732, 22594}, {-23800, 22522}, {-23870, 22448}, {-23938, 22375}, {-24007, 22302}, {-24075, 22228}, {-24143, 22154}, {-24211, 22080}, {-24279, 22005}, {-24347, 21930}, {-24413, 21856}, {-24481, 21781}, {-24547, 21706}, {-24614, 21630}, {-24680, 21555}, {-24746, 21479}, {-24811, 21403}, {-24877, 21326}, {-24942, 21250}, {-25008, 21173}, {-25072, 21097}, {-25137, 21020}, {-25201, 20943}, {-25265, 20865}, {-25330, 20787}, {-25393, 20710}, {-25457, 20631}, {-25520, 20554}, {-25583, 20475}, {-25645, 20397}, {-25708, 20318}, {-25770, 20239}, {-25832, 20159}, {-25893, 20080}, {-25955, 20001}, {-26016, 19921}, {-26077, 19841}, {-26138, 19761}, {-26199, 19681}, {-26259, 19601}, {-26319, 19520}, {-26379, 19438}, {-26438, 19358}, {-26498, 19276}, {-26556, 19195}, {-26615, 19113}, {-26674, 19032}, {-26732, 18950}, {-26790, 18868}, {-26848, 18785}, {-26905, 18703}, {-26963, 18620}, {-27019, 18538}, {-27076, 18455}, {-27132, 18372}, {-27189, 18288}, {-27245, 18204}, {-27301, 18121}, {-27356, 18037}, {-27411, 17953}, {-27467, 17869}, {-27521, 17785}, {-27576, 17700}, {-27629, 17616}, {-27684, 17530}, {-27737, 17446}, {-27791, 17360}, {-27844, 17275}, {-27897, 17189}, {-27949, 17104}, {-28002, 17018}, {-28053, 16932}, {-28105, 16846}, {-28157, 16759}, {-28208, 16673}, {-28259, 16586}, {-28310, 16500}, {-28361, 16412}, {-28411, 16326}, {-28461, 16238}, {-28510, 16151}, {-28560, 16063}, {-28609, 15976}, {-28658, 15888}, {-28706, 15800}, {-28755, 15712}, {-28803, 15624}, {-28851, 15535}, {-28898, 15446}, {-28945, 15358}, {-28993, 15268}, {-29039, 15180}, {-29086, 15090}, {-29132, 15001}, {-29178, 14912}, {-29223, 14823}, {-29269, 14732}, {-29313, 14643}, {-29358, 14553}, {-29403, 14463}, {-29447, 14372}, {-29491, 14282}, {-29535, 14191}, {-29578, 14101}, {-29621, 14010}, {-29664, 13918}, {-29707, 13828}, {-29749, 13736}, {-29791, 13645}, {-29833, 13554}, {-29874, 13462}, {-29915, 13370}, {-29956, 13279}, {-29997, 13187}, {-30037, 13095}, {-30077, 13002}, {-30117, 12910}, {-30156, 12818}, {-30195, 12725}, {-30234, 12632}, {-30273, 12539}, {-30311, 12447}, {-30350, 12353}, {-30387, 12260}, {-30425, 12167}, {-30462, 12074}, {-30499, 11980}, {-30535, 11887}, {-30572, 11793}, {-30608, 11699}, {-30644, 11605}, {-30679, 11511}, {-30714, 11417}, {-30749, 11323}, {-30784, 11228}, {-30818, 11134}, {-30852, 11039}, {-30886, 10944}, {-30919, 10849}, {-30952, 10754}, {-30985, 10660}, {-31018, 10564}, {-31050, 10469}, {-31082, 10374}, {-31113, 10279}, {-31145, 10183}, {-31176, 10088}, {-31207, 9991}, {-31237, 9896}, {-31268, 9800}, {-31297, 9704}, {-31327, 9608}, {-31357, 9511}, {-31385, 9416}, {-31414, 9319}, {-31443, 9223}, {-31471, 9126}, {-31499, 9030}, {-31526, 8933}, {-31553, 8836}, {-31581, 8739}, {-31607, 8643}, {-31634, 8545}, {-31659, 8449}, {-31685, 8351}, {-31711, 8254}, {-31736, 8157}, {-31761, 8060}, {-31785, 7962}, {-31810, 7864}, {-31834, 7767}, {-31857, 7668}, {-31881, 7571}, {-31904, 7473}, {-31927, 7375}, {-31949, 7277}, {-31971, 7179}, {-31993, 7081}, {-32015, 6983}, {-32036, 6885}, {-32057, 6787}, {-32078, 6688}, {-32098, 6590}, {-32118, 6491}, {-32138, 6392}, {-32157, 6294}, {-32177, 6195}, {-32195, 6096}, {-32214, 5997}, {-32232, 5899}, {-32250, 5799}, {-32268, 5701}, {-32285, 5602}, {-32302, 5503}, {-32319, 5403}, {-32335, 5305}, {-32351, 5205}, {-32367, 5106}, {-32383, 5007}, {-32398, 4908}, {-32413, 4808}, {-32427, 4708}, {-32442, 4609}, {-32456, 4509}, {-32469, 4410}, {-32483, 4310}, {-32496, 4211}, {-32509, 4110}, {-32521, 4011}, {-32533, 3911}, {-32545, 3812}, {-32557, 3711}, {-32568, 3612}, {-32579, 3511}, {-32589, 3412}, {-32600, 3312}, {-32610, 3211}, {-32619, 3112}, {-32629, 3011}, {-32638, 2911}, {-32647, 2811}, {-32655, 2711}, {-32663, 2610}, {-32671, 2511}, {-32679, 2410}, {-32686, 2310}, {-32693, 2210}, {-32699, 2110}, {-32706, 2009}, {-32712, 1909}, {-32718, 1808}, {-32723, 1708}, {-32728, 1608}, {-32733, 1507}, {-32737, 1407}, {-32741, 1306}, {-32745, 1206}, {-32749, 1105}, {-32752, 1005}, {-32755, 904}, {-32758, 804}, {-32760, 703}, {-32762, 603}, {-32764, 502}, {-32765, 402}, {-32766, 301}, {-32767, 201}, {-32767, 100}
};

#endif