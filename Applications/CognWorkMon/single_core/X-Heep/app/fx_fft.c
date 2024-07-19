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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <fixmath.h>
#include <fx_fft.h>
#include <window_definitions.h>

#define FFT_SIZE WINDOW_LENGTH

// radix 2
my_int real_w_fxp_r2[FFT_SIZE / 2] =
{
  262144, 262124, 262065, 261966, 261828, 261650, 261433, 261177, 260881, 260546, 260172, 259759, 259306, 258815, 258284, 257715,
  257106, 256459, 255774, 255050, 254287, 253487, 252648, 251771, 250856, 249903, 248913, 247885, 246820, 245717, 244578, 243402,
  242189, 240940, 239654, 238332, 236975, 235581, 234153, 232689, 231190, 229656, 228087, 226485, 224848, 223177, 221473, 219735,
  217964, 216160, 214324, 212456, 210555, 208623, 206659, 204665, 202639, 200583, 198497, 196381, 194235, 192060, 189856, 187624,
  185363, 183074, 180758, 178415, 176044, 173647, 171224, 168776, 166301, 163802, 161278, 158730, 156158, 153562, 150944, 148302,
  145638, 142953, 140245, 137517, 134768, 131998, 129209, 126400, 123573, 120726, 117862, 114979, 112080, 109163, 106230, 103281,
  100317, 97337, 94343, 91335, 88312, 85277, 82228, 79168, 76095, 73011, 69916, 66810, 63694, 60569, 57435, 54292, 51140, 47981,
  44815, 41642, 38463, 35278, 32088, 28893, 25693, 22490, 19283, 16073, 12861, 9647, 6432, 3215, -1, -3218, -6434, -9649, -12863,
  -16076, -19285, -22492, -25695, -28895, -32090, -35280, -38465, -41644, -44817, -47984, -51143, -54294, -57437, -60571, -63697,
  -66812, -69918, -73013, -76097, -79170, -82231, -85279, -88314, -91337, -94345, -97339, -100319, -103283, -106232, -109165,
  -112082, -114981, -117864, -120728, -123575, -126402, -129211, -132000, -134770, -137519, -140247, -142955, -145640, -148304,
  -150945, -153564, -156160, -158732, -161280, -163804, -166303, -168777, -171226, -173649, -176046, -178416, -180760, -183076,
  -185365, -187625, -189858, -192062, -194237, -196382, -198498, -200585, -202641, -204666, -206661, -208624, -210557, -212457,
  -214326, -216162, -217965, -219736, -221474, -223178, -224849, -226486, -228089, -229657, -231191, -232690, -234154, -235582,
  -236976, -238333, -239655, -240940, -242190, -243403, -244579, -245718, -246820, -247885, -248913, -249904, -250856, -251771,
  -252648, -253487, -254288, -255050, -255774, -256460, -257107, -257715, -258284, -258815, -259307, -259759, -260172, -260546,
  -260881, -261177, -261433, -261650, -261828, -261966, -262065, -262124, -262144, -262124, -262064, -261966, -261828, -261650,
  -261433, -261177, -260881, -260546, -260172, -259758, -259306, -258814, -258284, -257714, -257106, -256459, -255773, -255049,
  -254287, -253486, -252647, -251770, -250855, -249902, -248912, -247884, -246819, -245716, -244577, -243401, -242188, -240939,
  -239653, -238331, -236974, -235580, -234152, -232688, -231189, -229655, -228086, -226483, -224847, -223176, -221471, -219734,
  -217963, -216159, -214323, -212454, -210554, -208622, -206658, -204663, -202638, -200582, -198495, -196379, -194233, -192058,
  -189855, -187622, -185361, -183073, -180756, -178413, -176042, -173646, -171223, -168774, -166300, -163800, -161276, -158728,
  -156156, -153560, -150942, -148300, -145636, -142951, -140243, -137515, -134766, -131996, -129207, -126398, -123571, -120724,
  -117860, -114977, -112078, -109161, -106228, -103279, -100315, -97335, -94341, -91332, -88310, -85275, -82226, -79165, -76093,
  -73009, -69913, -66808, -63692, -60567, -57432, -54289, -51138, -47979, -44813, -41640, -38461, -35276, -32085, -28890, -25691,
  -22487, -19281, -16071, -12859, -9645, -6429, -3213, 3, 3220, 6436, 9652, 12866, 16078, 19288, 22494, 25698, 28897, 32092, 35283,
  38468, 41647, 44820, 47986, 51145, 54296, 57439, 60574, 63699, 66815, 69920, 73015, 76099, 79172, 82233, 85281, 88317, 91339,
  94347, 97342, 100321, 103286, 106235, 109167, 112084, 114984, 117866, 120730, 123577, 126404, 129213, 132002, 134772, 137521,
  140249, 142956, 145642, 148306, 150947, 153566, 156162, 158734, 161282, 163806, 166305, 168779, 171228, 173651, 176048, 178418,
  180761, 183078, 185366, 187627, 189859, 192063, 194238, 196384, 198500, 200586, 202642, 204668, 206662, 208626, 210558, 212458,
  214327, 216163, 217967, 219737, 221475, 223179, 224850, 226487, 228090, 229658, 231192, 232691, 234155, 235584, 236977, 238334,
  239656, 240941, 242191, 243403, 244580, 245719, 246821, 247886, 248914, 249904, 250857, 251772, 252649, 253488, 254288, 255051,
  255775, 256460, 257107, 257716, 258285, 258815, 259307, 259759, 260173, 260547, 260882, 261177, 261434, 261650, 261828, 261966,
  262065, 262124
};

my_int imag_w_fxp_r2[FFT_SIZE / 2] =
{
  0, -3216, -6433, -9648, -12862, -16074, -19284, -22491, -25694, -28894, -32089, -35279, -38464, -41643, -44816, -47982, -51141,
  -54293, -57436, -60570, -63695, -66811, -69917, -73012, -76096, -79169, -82230, -85278, -88313, -91336, -94344, -97338, -100318,
  -103282, -106231, -109164, -112081, -114980, -117863, -120727, -123574, -126401, -129210, -131999, -134769, -137518, -140246,
  -142954, -145639, -148303, -150945, -153563, -156159, -158731, -161279, -163803, -166302, -168776, -171225, -173648, -176045,
  -178415, -180759, -183075, -185364, -187624, -189857, -192061, -194236, -196382, -198498, -200584, -202640, -204665, -206660,
  -208624, -210556, -212456, -214325, -216161, -217965, -219735, -221473, -223178, -224848, -226485, -228088, -229656, -231190,
  -232689, -234153, -235582, -236975, -238333, -239654, -240940, -242189, -243402, -244578, -245718, -246820, -247885, -248913,
  -249903, -250856, -251771, -252648, -253487, -254288, -255050, -255774, -256460, -257107, -257715, -258284, -258815, -259306,
  -259759, -260172, -260546, -260881, -261177, -261433, -261650, -261828, -261966, -262065, -262124, -262144, -262124, -262065,
  -261966, -261828, -261650, -261433, -261177, -260881, -260546, -260172, -259759, -259306, -258814, -258284, -257714, -257106,
  -256459, -255774, -255050, -254287, -253486, -252647, -251770, -250855, -249903, -248912, -247884, -246819, -245717, -244577,
  -243401, -242188, -240939, -239654, -238332, -236974, -235581, -234152, -232688, -231189, -229655, -228087, -226484, -224847,
  -223176, -221472, -219734, -217963, -216160, -214324, -212455, -210555, -208622, -206659, -204664, -202638, -200582, -198496,
  -196380, -194234, -192059, -189855, -187623, -185362, -183073, -180757, -178414, -176043, -173646, -171223, -168775, -166300,
  -163801, -161277, -158729, -156157, -153561, -150943, -148301, -145637, -142952, -140244, -137516, -134767, -131997, -129208,
  -126399, -123572, -120725, -117861, -114978, -112079, -109162, -106229, -103280, -100316, -97336, -94342, -91333, -88311,
  -85276, -82227, -79167, -76094, -73010, -69915, -66809, -63693, -60568, -57433, -54290, -51139, -47980, -44814, -41641, -38462,
  -35277, -32086, -28891, -25692, -22488, -19282, -16072, -12860, -9646, -6431, -3214, 2, 3219, 6435, 9651, 12865, 16077, 19286,
  22493, 25697, 28896, 32091, 35281, 38466, 41646, 44819, 47985, 51144, 54295, 57438, 60572, 63698, 66813, 69919, 73014, 76098,
  79171, 82232, 85280, 88316, 91338, 94346, 97341, 100320, 103285, 106233, 109166, 112083, 114983, 117865, 120729, 123576, 126403,
  129212, 132001, 134771, 137520, 140248, 142955, 145641, 148305, 150946, 153565, 156161, 158733, 161281, 163805, 166304, 168778,
  171227, 173650, 176047, 178417, 180761, 183077, 185365, 187626, 189859, 192062, 194237, 196383, 198499, 200585, 202641, 204667,
  206662, 208625, 210557, 212458, 214326, 216162, 217966, 219737, 221474, 223179, 224850, 226486, 228089, 229658, 231191, 232690,
  234154, 235583, 236976, 238334, 239655, 240941, 242190, 243403, 244579, 245718, 246821, 247886, 248914, 249904, 250857, 251772,
  252648, 253487, 254288, 255051, 255775, 256460, 257107, 257715, 258285, 258815, 259307, 259759, 260172, 260547, 260882, 261177,
  261433, 261650, 261828, 261966, 262065, 262124, 262144, 262124, 262064, 261966, 261828, 261650, 261433, 261177, 260881, 260546,
  260172, 259758, 259306, 258814, 258284, 257714, 257106, 256459, 255773, 255049, 254286, 253486, 252647, 251770, 250855, 249902,
  248911, 247884, 246818, 245716, 244577, 243400, 242188, 240938, 239653, 238331, 236973, 235580, 234151, 232687, 231188, 229654,
  228086, 226483, 224846, 223175, 221471, 219733, 217962, 216158, 214322, 212454, 210553, 208621, 206657, 204663, 202637, 200581,
  198495, 196378, 194233, 192058, 189854, 187621, 185360, 183072, 180755, 178412, 176042, 173645, 171222, 168773, 166299, 163799,
  161275, 158727, 156155, 153559, 150941, 148299, 145635, 142950, 140242, 137514, 134765, 131995, 129206, 126397, 123570, 120723,
  117859, 114976, 112077, 109160, 106227, 103278, 100314, 97334, 94340, 91331, 88309, 85273, 82225, 79164, 76092, 73007, 69912,
  66807, 63691, 60566, 57431, 54288, 51137, 47978, 44812, 41639, 38460, 35274, 32084, 28889, 25690, 22486, 19279, 16070, 12858,
  9644, 6428, 3212
};

// split ops factors arrays
my_int A_r[FFT_SIZE / 2] =
{
  131072, 130269, 129465, 128661, 127857, 127053, 126250, 125446, 124643, 123840, 123038, 122235, 121433, 120632, 119830, 119030,
  118229, 117429, 116630, 115831, 115033, 114235, 113438, 112642, 111846, 111051, 110257, 109464, 108671, 107879, 107088, 106298,
  105509, 104721, 103934, 103148, 102363, 101579, 100796, 100015, 99234, 98455, 97677, 96900, 96124, 95350, 94577, 93806, 93036,
  92267, 91500, 90734, 89970, 89207, 88446, 87687, 86929, 86173, 85418, 84665, 83914, 83165, 82417, 81672, 80928, 80186, 79446,
  78708, 77972, 77238, 76506, 75776, 75048, 74322, 73599, 72877, 72158, 71441, 70726, 70013, 69303, 68595, 67890, 67186, 66485,
  65787, 65091, 64398, 63707, 63018, 62332, 61649, 60969, 60291, 59615, 58943, 58273, 57605, 56941, 56279, 55620, 54964, 54311,
  53661, 53014, 52369, 51728, 51089, 50454, 49822, 49192, 48566, 47943, 47323, 46706, 46092, 45482, 44874, 44270, 43669, 43072,
  42478, 41887, 41299, 40715, 40135, 39557, 38983, 38413, 37846, 37283, 36723, 36167, 35614, 35065, 34519, 33977, 33439, 32904,
  32373, 31846, 31323, 30803, 30287, 29775, 29267, 28762, 28262, 27765, 27272, 26783, 26298, 25817, 25340, 24867, 24397, 23932,
  23471, 23014, 22561, 22112, 21667, 21227, 20790, 20358, 19929, 19505, 19085, 18670, 18258, 17851, 17448, 17049, 16655, 16265,
  15879, 15498, 15121, 14748, 14380, 14016, 13656, 13301, 12951, 12604, 12263, 11925, 11592, 11264, 10940, 10621, 10306, 9996,
  9690, 9389, 9093, 8801, 8513, 8231, 7953, 7679, 7410, 7146, 6887, 6632, 6382, 6136, 5895, 5659, 5428, 5201, 4980, 4762, 4550,
  4342, 4140, 3942, 3748, 3560, 3376, 3197, 3023, 2854, 2689, 2530, 2375, 2225, 2080, 1940, 1804, 1674, 1548, 1428, 1312, 1201,
  1095, 993, 897, 806, 719, 637, 561, 489, 422, 360, 303, 251, 204, 161, 124, 92, 64, 42, 24, 11, 3, 1, 3, 10, 22, 38, 60, 87,
  119, 155, 197, 243, 294, 351, 412, 478, 549, 625, 706, 792, 882, 978, 1078, 1184, 1294, 1409, 1529, 1654, 1784, 1918, 2058,
  2202, 2351, 2505, 2664, 2828, 2996, 3169, 3348, 3531, 3718, 3911, 4108, 4310, 4517, 4729, 4945, 5166, 5392, 5623, 5858, 6098,
  6343, 6592, 6846, 7105, 7368, 7636, 7909, 8187, 8469, 8755, 9046, 9342, 9643, 9947, 10257, 10571, 10890, 11213, 11540, 11872,
  12209, 12550, 12896, 13246, 13600, 13959, 14322, 14690, 15062, 15438, 15819, 16204, 16593, 16987, 17385, 17787, 18194, 18605,
  19020, 19439, 19862, 20290, 20722, 21157, 21598, 22042, 22490, 22942, 23399, 23859, 24324, 24792, 25265, 25741, 26222, 26706,
  27195, 27687, 28183, 28683, 29187, 29695, 30206, 30722, 31241, 31763, 32290, 32820, 33354, 33892, 34433, 34978, 35527, 36079,
  36635, 37194, 37757, 38323, 38893, 39467, 40043, 40623, 41207, 41794, 42384, 42978, 43575, 44175, 44779, 45386, 45996, 46609,
  47225, 47845, 48467, 49093, 49722, 50354, 50989, 51627, 52268, 52912, 53559, 54209, 54861, 55517, 56175, 56836, 57500, 58167,
  58837, 59509, 60184, 60861, 61542, 62225, 62910, 63598, 64289, 64982, 65677, 66375, 67076, 67778, 68484, 69191, 69901, 70613,
  71328, 72045, 72764, 73485, 74208, 74933, 75661, 76391, 77122, 77856, 78592, 79330, 80069, 80811, 81554, 82300, 83047, 83796,
  84547, 85299, 86053, 86809, 87567, 88326, 89087, 89849, 90613, 91379, 92146, 92914, 93684, 94456, 95228, 96002, 96778, 97554,
  98332, 99111, 99892, 100673, 101456, 102239, 103024, 103810, 104597, 105385, 106174, 106964, 107754, 108546, 109339, 110132,
  110926, 111721, 112516, 113312, 114109, 114907, 115705, 116504, 117303, 118103, 118903, 119704, 120505, 121307, 122109, 122911,
  123714, 124516, 125320, 126123, 126926, 127730, 128534, 129338, 130142
};

my_int A_i[FFT_SIZE / 2] =
{
  -131072, -131069, -131062, -131049, -131032, -131010, -130983, -130951, -130914, -130872, -130825, -130773, -130717, -130655,
  -130588, -130517, -130441, -130360, -130273, -130182, -130086, -129986, -129880, -129769, -129654, -129533, -129408, -129278,
  -129143, -129003, -128858, -128709, -128555, -128395, -128231, -128062, -127889, -127710, -127527, -127339, -127146, -126948,
  -126746, -126538, -126326, -126110, -125888, -125662, -125431, -125195, -124955, -124710, -124460, -124206, -123946, -123683,
  -123414, -123141, -122863, -122581, -122294, -122003, -121706, -121406, -121100, -120790, -120476, -120157, -119834, -119506,
  -119173, -118836, -118495, -118149, -117799, -117444, -117085, -116721, -116353, -115981, -115604, -115223, -114838, -114448,
  -114054, -113655, -113253, -112846, -112435, -112020, -111600, -111176, -110748, -110316, -109880, -109440, -108995, -108546,
  -108094, -107637, -107176, -106711, -106243, -105770, -105293, -104812, -104327, -103839, -103346, -102850, -102349, -101845,
  -101337, -100825, -100310, -99790, -99267, -98740, -98210, -97675, -97137, -96596, -96051, -95502, -94949, -94393, -93834,
  -93270, -92704, -92134, -91560, -90983, -90403, -89819, -89232, -88641, -88047, -87450, -86849, -86245, -85638, -85028, -84415,
  -83798, -83178, -82555, -81929, -81300, -80668, -80033, -79395, -78753, -78109, -77462, -76812, -76159, -75503, -74845, -74183,
  -73519, -72852, -72182, -71510, -70835, -70157, -69477, -68794, -68108, -67420, -66729, -66036, -65340, -64642, -63941, -63238,
  -62533, -61825, -61115, -60402, -59688, -58971, -58252, -57530, -56807, -56081, -55353, -54624, -53892, -53158, -52422, -51684,
  -50944, -50203, -49459, -48713, -47966, -47217, -46466, -45713, -44959, -44203, -43445, -42686, -41925, -41162, -40398, -39633,
  -38866, -38097, -37327, -36556, -35783, -35009, -34233, -33457, -32679, -31899, -31119, -30337, -29555, -28771, -27986, -27200,
  -26413, -25625, -24836, -24046, -23255, -22463, -21671, -20878, -20083, -19289, -18493, -17697, -16900, -16102, -15304, -14505,
  -13706, -12906, -12106, -11305, -10504, -9702, -8900, -8098, -7295, -6492, -5689, -4886, -4082, -3279, -2475, -1671, -867, -63,
  741, 1545, 2348, 3152, 3956, 4760, 5563, 6366, 7169, 7972, 8774, 9576, 10378, 11179, 11980, 12780, 13580, 14380, 15178, 15977,
  16774, 17572, 18368, 19164, 19959, 20753, 21546, 22339, 23131, 23922, 24712, 25501, 26289, 27076, 27862, 28647, 29431, 30214,
  30996, 31777, 32556, 33334, 34111, 34887, 35661, 36434, 37206, 37976, 38745, 39512, 40278, 41042, 41805, 42566, 43326, 44084,
  44840, 45595, 46348, 47099, 47849, 48596, 49342, 50086, 50828, 51568, 52306, 53042, 53777, 54509, 55239, 55967, 56693, 57417,
  58139, 58858, 59575, 60290, 61003, 61713, 62422, 63127, 63831, 64532, 65230, 65927, 66620, 67311, 68000, 68686, 69370, 70050,
  70729, 71404, 72077, 72747, 73415, 74079, 74741, 75400, 76056, 76710, 77360, 78008, 78652, 79294, 79933, 80569, 81201, 81831,
  82457, 83081, 83701, 84318, 84932, 85543, 86150, 86755, 87356, 87954, 88548, 89139, 89727, 90311, 90892, 91470, 92044, 92615,
  93182, 93745, 94306, 94862, 95415, 95965, 96511, 97053, 97591, 98126, 98657, 99185, 99709, 100228, 100745, 101257, 101766,
  102271, 102771, 103269, 103762, 104251, 104736, 105218, 105695, 106169, 106638, 107104, 107565, 108023, 108476, 108925, 109370,
  109811, 110248, 110681, 111110, 111534, 111954, 112370, 112782, 113189, 113593, 113992, 114386, 114777, 115163, 115545, 115922,
  116295, 116664, 117028, 117388, 117743, 118094, 118441, 118783, 119121, 119454, 119783, 120107, 120427, 120742, 121052, 121358,
  121660, 121957, 122249, 122537, 122820, 123098, 123372, 123641, 123906, 124166, 124421, 124671, 124917, 125158, 125395, 125626,
  125853, 126076, 126293, 126506, 126714, 126917, 127115, 127309, 127498, 127682, 127861, 128036, 128205, 128370, 128530, 128685,
  128836, 128981, 129122, 129258, 129388, 129514, 129636, 129752, 129863, 129970, 130071, 130168, 130260, 130347, 130429, 130506,
  130578, 130645, 130708, 130765, 130818, 130865, 130908, 130946, 130979, 131006, 131029, 131047, 131061, 131069
};

my_int B_r[FFT_SIZE / 2] = 
{
  131072, 131876, 132680, 133484, 134288, 135091, 135895, 136698, 137501, 138304, 139107, 139909, 140711, 141513, 142314, 143115,
  143915, 144715, 145515, 146313, 147112, 147909, 148706, 149503, 150298, 151093, 151887, 152681, 153473, 154265, 155056, 155846,
  156635, 157423, 158210, 158996, 159781, 160565, 161348, 162130, 162910, 163690, 164468, 165245, 166020, 166794, 167567, 168339,
  169109, 169877, 170645, 171410, 172175, 172937, 173698, 174458, 175216, 175972, 176726, 177479, 178230, 178980, 179727, 180473,
  181216, 181958, 182698, 183436, 184172, 184906, 185638, 186368, 187096, 187822, 188546, 189267, 189987, 190704, 191419, 192131,
  192841, 193549, 194255, 194958, 195659, 196357, 197053, 197747, 198438, 199126, 199812, 200495, 201176, 201854, 202529, 203202,
  203872, 204539, 205204, 205865, 206524, 207180, 207833, 208483, 209131, 209775, 210417, 211055, 211691, 212323, 212952, 213579,
  214202, 214822, 215439, 216052, 216663, 217270, 217874, 218475, 219073, 219667, 220258, 220845, 221429, 222010, 222587, 223161,
  223731, 224298, 224862, 225422, 225978, 226531, 227080, 227625, 228167, 228706, 229240, 229771, 230298, 230822, 231341, 231857,
  232369, 232878, 233382, 233883, 234380, 234872, 235361, 235846, 236328, 236805, 237278, 237747, 238212, 238673, 239130, 239583,
  240032, 240477, 240918, 241354, 241787, 242215, 242639, 243059, 243475, 243886, 244293, 244696, 245095, 245489, 245879, 246265,
  246647, 247024, 247396, 247765, 248129, 248488, 248843, 249194, 249540, 249882, 250219, 250552, 250880, 251204, 251524, 251838,
  252149, 252454, 252755, 253052, 253344, 253631, 253914, 254192, 254465, 254734, 254998, 255258, 255513, 255763, 256008, 256249,
  256485, 256716, 256943, 257165, 257382, 257594, 257802, 258005, 258203, 258396, 258585, 258768, 258947, 259121, 259291, 259455,
  259615, 259769, 259919, 260064, 260205, 260340, 260471, 260596, 260717, 260833, 260944, 261050, 261151, 261247, 261339, 261425,
  261507, 261584, 261656, 261722, 261784, 261841, 261894, 261941, 261983, 262020, 262053, 262080, 262103, 262121, 262133, 262141,
  262144, 262142, 262135, 262123, 262106, 262084, 262058, 262026, 261989, 261948, 261901, 261850, 261794, 261733, 261666, 261595,
  261519, 261439, 261353, 261262, 261167, 261066, 260961, 260851, 260736, 260616, 260491, 260361, 260226, 260087, 259943, 259793,
  259639, 259481, 259317, 259148, 258975, 258797, 258614, 258426, 258234, 258036, 257834, 257627, 257416, 257199, 256978, 256753,
  256522, 256287, 256047, 255802, 255552, 255298, 255040, 254776, 254508, 254235, 253958, 253676, 253389, 253098, 252802, 252502,
  252197, 251888, 251573, 251255, 250932, 250604, 250272, 249935, 249594, 249249, 248899, 248544, 248186, 247822, 247455, 247083,
  246706, 246326, 245941, 245551, 245158, 244760, 244357, 243951, 243540, 243125, 242706, 242282, 241855, 241423, 240987, 240547,
  240103, 239655, 239202, 238746, 238285, 237821, 237352, 236880, 236403, 235923, 235438, 234950, 234458, 233961, 233461, 232958,
  232450, 231938, 231423, 230904, 230381, 229854, 229324, 228790, 228252, 227711, 227166, 226618, 226065, 225510, 224950, 224387,
  223821, 223251, 222678, 222101, 221521, 220937, 220350, 219760, 219167, 218570, 217969, 217366, 216759, 216149, 215536, 214919,
  214300, 213677, 213051, 212422, 211791, 211156, 210518, 209877, 209233, 208586, 207936, 207283, 206628, 205969, 205308, 204644,
  203977, 203308, 202636, 201961, 201283, 200603, 199920, 199235, 198547, 197856, 197163, 196467, 195769, 195069, 194366, 193661,
  192953, 192243, 191531, 190817, 190100, 189381, 188660, 187936, 187211, 186483, 185754, 185022, 184288, 183553, 182815, 182075,
  181334, 180590, 179845, 179098, 178349, 177598, 176845, 176091, 175335, 174578, 173818, 173057, 172295, 171531, 170766, 169999,
  169230, 168460, 167689, 166916, 166142, 165367, 164590, 163812, 163033, 162253, 161471, 160689, 159905, 159120, 158334, 157547,
  156759, 155971, 155181, 154390, 153598, 152806, 152013, 151219, 150424, 149628, 148832, 148035, 147238, 146439, 145641, 144841,
  144042, 143241, 142441, 141639, 140838, 140036, 139234, 138431, 137628, 136825, 136022, 135218, 134414, 133611, 132807, 132003
};
my_int B_i[FFT_SIZE / 2] = 
{
  131072, 131070, 131062, 131050, 131033, 131010, 130983, 130951, 130914, 130872, 130825, 130774, 130717, 130655, 130589, 130518,
  130441, 130360, 130274, 130183, 130087, 129986, 129880, 129770, 129654, 129534, 129409, 129278, 129143, 129004, 128859, 128709,
  128555, 128396, 128232, 128063, 127889, 127711, 127527, 127339, 127146, 126949, 126746, 126539, 126327, 126110, 125889, 125662,
  125432, 125196, 124955, 124710, 124461, 124206, 123947, 123683, 123415, 123142, 122864, 122582, 122295, 122003, 121707, 121406,
  121101, 120791, 120477, 120158, 119834, 119506, 119174, 118837, 118495, 118149, 117799, 117444, 117085, 116722, 116354, 115981,
  115605, 115223, 114838, 114448, 114054, 113656, 113253, 112847, 112435, 112020, 111601, 111177, 110749, 110317, 109880, 109440,
  108996, 108547, 108094, 107638, 107177, 106712, 106243, 105770, 105293, 104813, 104328, 103839, 103347, 102850, 102350, 101846,
  101338, 100826, 100310, 99791, 99268, 98741, 98210, 97676, 97138, 96596, 96051, 95502, 94950, 94394, 93834, 93271, 92704, 92134,
  91561, 90984, 90403, 89819, 89232, 88641, 88048, 87450, 86850, 86246, 85639, 85029, 84415, 83799, 83179, 82556, 81930, 81301,
  80669, 80033, 79395, 78754, 78110, 77463, 76813, 76160, 75504, 74845, 74184, 73520, 72853, 72183, 71510, 70835, 70158, 69477,
  68794, 68108, 67420, 66729, 66036, 65340, 64642, 63942, 63239, 62533, 61825, 61115, 60403, 59688, 58971, 58252, 57531, 56807,
  56082, 55354, 54624, 53892, 53158, 52422, 51685, 50945, 50203, 49459, 48714, 47967, 47217, 46467, 45714, 44960, 44203, 43446,
  42686, 41925, 41163, 40399, 39633, 38866, 38097, 37327, 36556, 35783, 35009, 34234, 33457, 32679, 31900, 31119, 30338, 29555,
  28771, 27986, 27200, 26413, 25625, 24836, 24046, 23256, 22464, 21671, 20878, 20084, 19289, 18493, 17697, 16900, 16103, 15304,
  14506, 13706, 12906, 12106, 11305, 10504, 9703, 8901, 8098, 7296, 6493, 5690, 4886, 4083, 3279, 2475, 1671, 867, 63, -740,
  -1544, -2348, -3152, -3955, -4759, -5562, -6366, -7169, -7971, -8774, -9576, -10377, -11178, -11979, -12780, -13580, -14379,
  -15178, -15976, -16774, -17571, -18367, -19163, -19958, -20752, -21546, -22338, -23130, -23921, -24711, -25500, -26289, -27076,
  -27862, -28647, -29431, -30214, -30996, -31776, -32556, -33334, -34111, -34886, -35661, -36434, -37205, -37976, -38744, -39512,
  -40278, -41042, -41805, -42566, -43326, -44084, -44840, -45595, -46347, -47099, -47848, -48596, -49341, -50085, -50827, -51568,
  -52306, -53042, -53776, -54508, -55238, -55967, -56693, -57416, -58138, -58857, -59575, -60290, -61003, -61713, -62421, -63127,
  -63830, -64531, -65230, -65926, -66620, -67311, -67999, -68686, -69369, -70050, -70728, -71404, -72077, -72747, -73414, -74079,
  -74741, -75400, -76056, -76709, -77360, -78007, -78652, -79294, -79932, -80568, -81201, -81830, -82457, -83080, -83700, -84318,
  -84932, -85542, -86150, -86754, -87355, -87953, -88547, -89139, -89726, -90311, -90892, -91469, -92043, -92614, -93181, -93745,
  -94305, -94862, -95415, -95964, -96510, -97052, -97591, -98126, -98657, -99184, -99708, -100228, -100744, -101257, -101765,
  -102270, -102771, -103268, -103761, -104251, -104736, -105217, -105695, -106168, -106638, -107103, -107565, -108022, -108475,
  -108925, -109370, -109811, -110248, -110680, -111109, -111533, -111954, -112370, -112781, -113189, -113592, -113991, -114386,
  -114776, -115162, -115544, -115922, -116295, -116663, -117028, -117387, -117743, -118094, -118441, -118783, -119120, -119454,
  -119782, -120106, -120426, -120741, -121052, -121358, -121659, -121956, -122248, -122536, -122819, -123098, -123371, -123641,
  -123905, -124165, -124420, -124671, -124917, -125158, -125394, -125626, -125853, -126075, -126293, -126505, -126713, -126917,
  -127115, -127309, -127498, -127682, -127861, -128035, -128205, -128370, -128530, -128685, -128835, -128981, -129121, -129257,
  -129388, -129514, -129635, -129751, -129863, -129969, -130071, -130168, -130259, -130346, -130428, -130505, -130578, -130645,
  -130707, -130765, -130817, -130865, -130907, -130945, -130978, -131006, -131029, -131047, -131060, -131068
};

// tmp array for real fft
my_int fft_re_tmp[FFT_SIZE];
my_int fft_im_tmp[FFT_SIZE];

uint32_t NumBits;

void init_fft()
{
  // Real FFT of size FFT_SIZE is transform to a complex FFT of size FFT_SIZE/2
  NumBits = NumberOfBitsNeeded(FFT_SIZE / 2);

  // *** INITIALIZATION OF LOOK-UP TABLES ***
  // Not needed anymore - All table initialized statically
  
  /*
  // Create twiddle factors radix-2
  my_int num = fx_mulx(fx_itox(2, N_DEC_POW), fx_ftox(M_PI, N_DEC_POW), N_DEC_POW);     // 2. * M_PI
  my_int den = fx_divx(fx_itox(FFT_SIZE, N_DEC_POW), fx_itox(2, N_DEC_POW), N_DEC_POW); // FFT_SIZE / 2.
  for (int i = 0; i < FFT_SIZE / 2; i++)
  {
    my_int arg = fx_mulx(fx_divx(num, den, N_DEC_POW), fx_itox(i, N_DEC_POW), N_DEC_POW);
    real_w_fxp_r2[i] = fx_cosx(arg, N_DEC_POW);
    imag_w_fxp_r2[i] = -fx_sinx(arg, N_DEC_POW);
  }

  // The recovery additional step to recover the correct output is called split ops
  // The factors are precomputed here
  den = fx_divx(fx_mulx(fx_itox(2, N_DEC_POW), fx_itox(FFT_SIZE, N_DEC_POW), N_DEC_POW), fx_itox(2, N_DEC_POW), N_DEC_POW);
  for (int i = 0, j = FFT_SIZE / 2 - 1; i < FFT_SIZE / 2; i++, j--)
  {
    my_int arg = fx_mulx(fx_divx(num, den, N_DEC_POW), fx_itox(i, N_DEC_POW), N_DEC_POW);
    my_int sinx = fx_sinx(arg, N_DEC_POW);
    my_int cosx = fx_cosx(arg, N_DEC_POW);

    A_r[i] = fx_mulx(fx_ftox(0.5, N_DEC_POW), (fx_itox(1, N_DEC_POW) - sinx), N_DEC_POW);
    A_i[i] = fx_mulx(fx_ftox(0.5, N_DEC_POW), fx_mulx(fx_itox(-1, N_DEC_POW), cosx, N_DEC_POW), N_DEC_POW);

    B_r[i] = fx_mulx(fx_ftox(0.5, N_DEC_POW), (fx_itox(1, N_DEC_POW) + sinx), N_DEC_POW);
    B_i[i] = fx_mulx(fx_ftox(0.5, N_DEC_POW), fx_mulx(fx_itox(1, N_DEC_POW), cosx, N_DEC_POW), N_DEC_POW);
  }*/
}

void fft(my_int *input, my_int *outputR, my_int *outputI)
{
  int i, j;
  // Real input is transformed to complex input
  for (i = 0; i < FFT_SIZE / 2; i++)
  {
    j = ReverseBits(i, NumBits);
    fft_re_tmp[j] = input[2 * i];
    fft_im_tmp[j] = input[2 * i + 1];
  }

  fft_cplx_radix2_iter(fft_re_tmp, fft_im_tmp, FFT_SIZE / 2, NumBits, real_w_fxp_r2, imag_w_fxp_r2);

  fft_re_tmp[FFT_SIZE / 2] = fft_re_tmp[0];
  fft_im_tmp[FFT_SIZE / 2] = fft_im_tmp[0];

  // Split ops
  for (i = 0; i < FFT_SIZE / 2; i++)
  {
    outputR[i] = fx_mulx(fft_re_tmp[i], A_r[i], N_DEC_POW) - fx_mulx(fft_im_tmp[i], A_i[i], N_DEC_POW) + fx_mulx(fft_re_tmp[FFT_SIZE / 2 - i], B_r[i], N_DEC_POW) + fx_mulx(fft_im_tmp[FFT_SIZE / 2 - i], B_i[i], N_DEC_POW);
    outputI[i] = fx_mulx(fft_im_tmp[i], A_r[i], N_DEC_POW) + fx_mulx(fft_re_tmp[i], A_i[i], N_DEC_POW) + fx_mulx(fft_re_tmp[FFT_SIZE / 2 - i], B_i[i], N_DEC_POW) - fx_mulx(fft_im_tmp[FFT_SIZE / 2 - i], B_r[i], N_DEC_POW);
  }

  // Copying the complex conjugate
  outputR[FFT_SIZE / 2] = outputR[0] - outputI[0];
  outputI[FFT_SIZE / 2] = 0;

  for (i = 1; i < FFT_SIZE / 2; i++)
  {
    outputR[FFT_SIZE - i] = outputR[i];
    outputI[FFT_SIZE - i] = -outputI[i];
  }
}

uint32_t ReverseBits(uint32_t index, uint32_t NumBits)
{
  uint32_t i, rev;

  for (i = rev = 0; i < NumBits; i++)
  {
    rev = (rev << 1) | (index & 1);
    index >>= 1;
  }

  return rev;
}

uint32_t NumberOfBitsNeeded(uint32_t nsample)
{
  uint32_t i;

  if (nsample < 2)
  {
    fprintf(stderr, ">>> Error : argument %d to NumberOfBitsNeeded is too small.\n", nsample);
    exit(1);
    return 0;
  }

  for (i = 0;; i++)
  {
    if (nsample & (1 << i))
      return i;
  }
}

void fft_cplx_radix2_iter(my_int *Real_Out, my_int *Imag_Out, int32_t fft_size, int32_t nbits, my_int *re_factors, my_int *im_factors)
{
  int32_t n_group = fft_size / 2;
  int32_t group_size = 1;
  int32_t base_idx;

  for (int32_t i = 1; i <= nbits; i++)
  {
    for (int32_t k = 0; k < n_group; k++)
    {
      base_idx = k << i;

      for (int32_t j = 0; j < group_size; j++)
      {
        // Even inputs
        my_int even_real = Real_Out[base_idx + j];
        my_int even_imag = Imag_Out[base_idx + j];
        // Odd inputs
        my_int odd_real = Real_Out[base_idx + j + group_size];
        my_int odd_imag = Imag_Out[base_idx + j + group_size];

        // use look-up table
        my_int w_r = re_factors[j * n_group];
        my_int w_i = im_factors[j * n_group];

        // Butterfly
        my_int p_sum_r = fx_mulx(w_r, odd_real, N_DEC_POW) - fx_mulx(w_i, odd_imag, N_DEC_POW);
        my_int p_sum_i = fx_mulx(w_i, odd_real, N_DEC_POW) + fx_mulx(w_r, odd_imag, N_DEC_POW);

        Real_Out[base_idx + j] = even_real + p_sum_r;
        Real_Out[base_idx + j + group_size] = even_real - p_sum_r;

        Imag_Out[base_idx + j] = even_imag + p_sum_i;
        Imag_Out[base_idx + j + group_size] = even_imag - p_sum_i;
      }
    }
    n_group >>= 1;
    group_size <<= 1;
  }
}
