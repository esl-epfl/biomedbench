// Porting to X-Heep : Francesco Poluzzi
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

#ifndef WINDOW_DEFINITIONS_H_
#define WINDOW_DEFINITIONS_H_

#include <inttypes.h>

#define WINDOW_LENGTH 1024	//4*256		//better define it multiple of 2 for FFT execution

#define TOTAL_WINDOW_LENGTH	4*256 //56*256		//better define it multiple of 2 for FFT execution

#define N_BATCHES	14 //(56/4)
#define N_BINS	20

#define DATA_POOL_LENGHT		(uint16_t)(3.0/2*WINDOW_LENGTH+3*EEG_ARRAY_SIZE) //it is needed 1/2*WINDOW_LENGTH of data for the blink removal approach

#define NO_OVERLAPING_LENGTH	256  //number of new samples used at each processing execution
#define SAMPLES_OVERLAPING		(WINDOW_LENGTH - NO_OVERLAPING_LENGTH)


#define 	WINDOW_SCALING_FACTOR 	0.00245974262237096 //window power

// These samples are represented in fixed-point with 18 bits for the decimal part
static const int32_t hamming_win[WINDOW_LENGTH]={//1024
20972,
20974,
20981,
20992,
21008,
21028,
21053,
21083,
21117,
21156,
21199,
21247,
21299,
21356,
21417,
21483,
21553,
21628,
21708,
21792,
21880,
21973,
22071,
22173,
22279,
22390,
22506,
22626,
22750,
22879,
23013,
23151,
23293,
23440,
23591,
23747,
23907,
24072,
24241,
24414,
24592,
24775,
24961,
25153,
25348,
25548,
25752,
25961,
26174,
26391,
26613,
26839,
27070,
27304,
27543,
27787,
28034,
28286,
28542,
28803,
29067,
29336,
29609,
29887,
30168,
30454,
30744,
31038,
31337,
31639,
31946,
32256,
32571,
32890,
33213,
33541,
33872,
34207,
34547,
34890,
35237,
35589,
35944,
36304,
36667,
37035,
37406,
37781,
38160,
38543,
38930,
39321,
39716,
40114,
40516,
40923,
41333,
41746,
42164,
42585,
43010,
43439,
43871,
44307,
44747,
45190,
45637,
46088,
46542,
47000,
47461,
47926,
48395,
48867,
49342,
49821,
50304,
50789,
51279,
51771,
52268,
52767,
53270,
53776,
54285,
54798,
55314,
55833,
56356,
56882,
57411,
57943,
58478,
59016,
59558,
60102,
60650,
61201,
61754,
62311,
62871,
63434,
63999,
64568,
65139,
65714,
66291,
66871,
67454,
68039,
68628,
69219,
69813,
70410,
71009,
71611,
72215,
72823,
73433,
74045,
74660,
75277,
75897,
76520,
77145,
77772,
78402,
79034,
79668,
80305,
80944,
81586,
82229,
82875,
83523,
84174,
84826,
85481,
86137,
86796,
87457,
88120,
88785,
89452,
90121,
90792,
91464,
92139,
92816,
93494,
94174,
94856,
95540,
96225,
96912,
97601,
98292,
98984,
99678,
100373,
101070,
101768,
102468,
103169,
103872,
104576,
105282,
105989,
106697,
107407,
108118,
108830,
109544,
110258,
110974,
111691,
112409,
113128,
113849,
114570,
115292,
116016,
116740,
117465,
118191,
118919,
119646,
120375,
121105,
121835,
122566,
123298,
124030,
124763,
125497,
126231,
126966,
127702,
128438,
129174,
129911,
130648,
131386,
132124,
132863,
133602,
134341,
135080,
135820,
136560,
137300,
138040,
138781,
139521,
140262,
141002,
141743,
142484,
143224,
143965,
144705,
145445,
146186,
146926,
147665,
148405,
149144,
149883,
150622,
151360,
152098,
152836,
153573,
154310,
155046,
155782,
156517,
157252,
157986,
158719,
159452,
160184,
160915,
161646,
162376,
163105,
163833,
164561,
165287,
166013,
166738,
167462,
168184,
168906,
169627,
170347,
171065,
171783,
172499,
173215,
173929,
174642,
175353,
176063,
176772,
177480,
178186,
178891,
179595,
180297,
180998,
181697,
182394,
183091,
183785,
184478,
185169,
185859,
186547,
187233,
187918,
188601,
189282,
189961,
190638,
191314,
191988,
192660,
193329,
193997,
194663,
195327,
195989,
196649,
197307,
197962,
198616,
199267,
199917,
200564,
201208,
201851,
202491,
203129,
203765,
204398,
205029,
205658,
206284,
206907,
207529,
208147,
208763,
209377,
209988,
210597,
211203,
211806,
212407,
213005,
213600,
214192,
214782,
215369,
215954,
216535,
217114,
217690,
218262,
218832,
219400,
219964,
220525,
221083,
221638,
222191,
222740,
223286,
223829,
224369,
224906,
225439,
225970,
226497,
227021,
227542,
228060,
228574,
229085,
229593,
230098,
230599,
231096,
231591,
232082,
232569,
233054,
233534,
234011,
234485,
234955,
235422,
235885,
236345,
236801,
237253,
237702,
238147,
238589,
239027,
239461,
239892,
240319,
240742,
241161,
241577,
241988,
242396,
242801,
243201,
243598,
243990,
244379,
244764,
245145,
245523,
245896,
246265,
246631,
246992,
247349,
247703,
248052,
248398,
248739,
249076,
249410,
249739,
250064,
250385,
250702,
251015,
251324,
251628,
251929,
252225,
252517,
252805,
253089,
253368,
253643,
253914,
254181,
254444,
254702,
254956,
255206,
255451,
255692,
255929,
256162,
256390,
256614,
256833,
257049,
257259,
257466,
257668,
257866,
258059,
258248,
258433,
258613,
258788,
258960,
259127,
259289,
259447,
259600,
259750,
259894,
260034,
260170,
260301,
260428,
260550,
260668,
260781,
260890,
260994,
261094,
261189,
261280,
261366,
261448,
261525,
261598,
261666,
261730,
261789,
261843,
261893,
261939,
261980,
262016,
262048,
262075,
262098,
262116,
262130,
262139,
262143,
262143,
262139,
262130,
262116,
262098,
262075,
262048,
262016,
261980,
261939,
261893,
261843,
261789,
261730,
261666,
261598,
261525,
261448,
261366,
261280,
261189,
261094,
260994,
260890,
260781,
260668,
260550,
260428,
260301,
260170,
260034,
259894,
259750,
259600,
259447,
259289,
259127,
258960,
258788,
258613,
258433,
258248,
258059,
257866,
257668,
257466,
257259,
257049,
256833,
256614,
256390,
256162,
255929,
255692,
255451,
255206,
254956,
254702,
254444,
254181,
253914,
253643,
253368,
253089,
252805,
252517,
252225,
251929,
251628,
251324,
251015,
250702,
250385,
250064,
249739,
249410,
249076,
248739,
248398,
248052,
247703,
247349,
246992,
246631,
246265,
245896,
245523,
245145,
244764,
244379,
243990,
243598,
243201,
242801,
242396,
241988,
241577,
241161,
240742,
240319,
239892,
239461,
239027,
238589,
238147,
237702,
237253,
236801,
236345,
235885,
235422,
234955,
234485,
234011,
233534,
233054,
232569,
232082,
231591,
231096,
230599,
230098,
229593,
229085,
228574,
228060,
227542,
227021,
226497,
225970,
225439,
224906,
224369,
223829,
223286,
222740,
222191,
221638,
221083,
220525,
219964,
219400,
218832,
218262,
217690,
217114,
216535,
215954,
215369,
214782,
214192,
213600,
213005,
212407,
211806,
211203,
210597,
209988,
209377,
208763,
208147,
207529,
206907,
206284,
205658,
205029,
204398,
203765,
203129,
202491,
201851,
201208,
200564,
199917,
199267,
198616,
197962,
197307,
196649,
195989,
195327,
194663,
193997,
193329,
192660,
191988,
191314,
190638,
189961,
189282,
188601,
187918,
187233,
186547,
185859,
185169,
184478,
183785,
183091,
182394,
181697,
180998,
180297,
179595,
178891,
178186,
177480,
176772,
176063,
175353,
174642,
173929,
173215,
172499,
171783,
171065,
170347,
169627,
168906,
168184,
167462,
166738,
166013,
165287,
164561,
163833,
163105,
162376,
161646,
160915,
160184,
159452,
158719,
157986,
157252,
156517,
155782,
155046,
154310,
153573,
152836,
152098,
151360,
150622,
149883,
149144,
148405,
147665,
146926,
146186,
145445,
144705,
143965,
143224,
142484,
141743,
141002,
140262,
139521,
138781,
138040,
137300,
136560,
135820,
135080,
134341,
133602,
132863,
132124,
131386,
130648,
129911,
129174,
128438,
127702,
126966,
126231,
125497,
124763,
124030,
123298,
122566,
121835,
121105,
120375,
119646,
118919,
118191,
117465,
116740,
116016,
115292,
114570,
113849,
113128,
112409,
111691,
110974,
110258,
109544,
108830,
108118,
107407,
106697,
105989,
105282,
104576,
103872,
103169,
102468,
101768,
101070,
100373,
99678,
98984,
98292,
97601,
96912,
96225,
95540,
94856,
94174,
93494,
92816,
92139,
91464,
90792,
90121,
89452,
88785,
88120,
87457,
86796,
86137,
85481,
84826,
84174,
83523,
82875,
82229,
81586,
80944,
80305,
79668,
79034,
78402,
77772,
77145,
76520,
75897,
75277,
74660,
74045,
73433,
72823,
72215,
71611,
71009,
70410,
69813,
69219,
68628,
68039,
67454,
66871,
66291,
65714,
65139,
64568,
63999,
63434,
62871,
62311,
61754,
61201,
60650,
60102,
59558,
59016,
58478,
57943,
57411,
56882,
56356,
55833,
55314,
54798,
54285,
53776,
53270,
52767,
52268,
51771,
51279,
50789,
50304,
49821,
49342,
48867,
48395,
47926,
47461,
47000,
46542,
46088,
45637,
45190,
44747,
44307,
43871,
43439,
43010,
42585,
42164,
41746,
41333,
40923,
40516,
40114,
39716,
39321,
38930,
38543,
38160,
37781,
37406,
37035,
36667,
36304,
35944,
35589,
35237,
34890,
34547,
34207,
33872,
33541,
33213,
32890,
32571,
32256,
31946,
31639,
31337,
31038,
30744,
30454,
30168,
29887,
29609,
29336,
29067,
28803,
28542,
28286,
28034,
27787,
27543,
27304,
27070,
26839,
26613,
26391,
26174,
25961,
25752,
25548,
25348,
25153,
24961,
24775,
24592,
24414,
24241,
24072,
23907,
23747,
23591,
23440,
23293,
23151,
23013,
22879,
22750,
22626,
22506,
22390,
22279,
22173,
22071,
21973,
21880,
21792,
21708,
21628,
21553,
21483,
21417,
21356,
21299,
21247,
21199,
21156,
21117,
21083,
21053,
21028,
21008,
20992,
20981,
20974,
20972
};

#endif /* WINDOW_DEFINITIONS_H_ */
