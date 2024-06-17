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

#ifndef	FILTER_PARAM_H
#define FILTER_PARAM_H

#include <eglass.h>

#define SAMPLING_FREQ			256

#define N_FILTER_COEF 			9
#define N_CHANNEL_USED			CH_TO_STORE


enum _eeg_channels_t{
	CHA=0,
	CHB,
	CHC,
	CHD
};


typedef struct{
	float coef_A[N_FILTER_COEF];
	float coef_B[N_FILTER_COEF];
} _eeg_band_coef_t;

typedef float sig_type;



////static const float Lo_D[FILTER_LENGTH] = {7.8089793e-01,
////0.0328830116669829,
////0.0308413818359870,
////-0.187034811718881,
////-0.0279837694169839,
////0.630880767929590,
////0.714846570552542,
////0.230377813308855};
////
////
//static const float Hi_D[FILTER_LENGTH] = {-0.230377813308855,
//0.714846570552542,
//-0.630880767929590,
//-0.0279837694169839,
//0.187034811718881,
//0.0308413818359870,
//-0.0328830116669829,
//-0.0105974017849973};
//
////coming from matlab, using sprintf to show the coefficients  to have similar precision
// 	example to convert:
//		sprintf('%1.50f', a(3))

//cheb2 hipass filter, 0.5Hz, 4th order, 40dB attenuation
//static _eeg_band_coef_t hipass_baseline = {.coef_A={1.0, -3.97127207771085322107751380826812237501144409179688, 5.91426704094419708468421958968974649906158447265625, -3.91471367985300799929859749681781977415084838867188, 0.97171873582408407887811563341529108583927154541016},
//																.coef_B={ 0.98575794991675547151999126072041690349578857421875, -3.94299288353502541681905313453171402215957641601562, 5.91446986742858360486252422560937702655792236328125, -3.94299288353502497272984328446909785270690917968750, 0.98575794991675513845308387317345477640628814697266}};


//cheb2 bandpass 10th order filter, 0.5-26Hz, including a natural notch on 50Hz, 40dB attenuation
//static _eeg_band_coef_t hipass_baseline = {.coef_A={1.0, -7.19461806731774444756410957779735326766967773437500, 23.37809561541504166370941675268113613128662109375000, -45.34763387837064385621488327160477638244628906250000, 58.32029325859667068243652465753257274627685546875000,
//																				-52.04490294662343075060562114231288433074951171875000, 32.65009483843373061517922906205058097839355468750000, -14.21531501501192984449062350904569029808044433593750, 4.10989316061487652831374361994676291942596435546875, -0.71123060188569375839051645016297698020935058593750, 0.05532363621116355434370603916249820031225681304932},
//																				.coef_B={0.02903181179848979623359106483349023619666695594788, -0.12499843244837355316079197109502274543046951293945, 0.23308920220582837745659787742624757811427116394043, -0.26299971994806248254406000341987237334251403808594, 0.18156650425489814781876418692263541743159294128418,
//																						0.00000000000000110232707796015345296530801355363853, -0.18156650425489959110869619962613796815276145935059, 0.26299971994806270458866492845118045806884765625000, -0.23308920220582801663411487425037194043397903442383, 0.12499843244837316458273335229023359715938568115234, -0.02903181179848967827239469841060781618580222129822}};

//cheb2 bandpass filter, 0.5-26Hz, 8th order, 40dB attenuation
static _eeg_band_coef_t filter_coeff = {.coef_A={1.0, -6.23514447448442421517711409251205623149871826171875, 17.08613209410433952939456503372639417648315429687500, -26.94659470160280179129586031194776296615600585937500, 26.80259212817515646065658074803650379180908203125000, -17.23220588760688443130675295833498239517211914062500, 6.99316449252849725581882012193091213703155517578125, -1.63788632042738213634436306165298447012901306152344, 0.16994267729268203170889250941399950534105300903320},
																.coef_B={0.02522296032835384671511036458468879573047161102295, -0.09084944970162771304966753405096824280917644500732, 0.14587388957391012045405886965454556047916412353516, -0.16940970721897979922587751389073673635721206665039, 0.17832461411647890225928847485192818567156791687012, -0.16940970721897977147030189826182322576642036437988, 0.14587388957391003718733202276780502870678901672363, -0.09084944970162769917187972623651148751378059387207, 0.02522296032835387447068598021360230632126331329346}};

//This is a coefficient matrix for my own implementation of a SOS filtering function
//PB 4th order Butter, [0.5 45]Hz @ Fs=250Hz
static float sos_filter[3*6]={0.013427733421538912139681087865028530359f, 1.00000000000000000000000000000000000000000f, 1.00000000000000000000000000000000000000000f,
										 0.026855466843077824279362175730057060719f, 0.00000000000000000000000000000000000000000f, -2.00000000000000000000000000000000000000000f,
										 0.013427733421538912139681087865028530359f, -1.0000000000000000000000000000000000000000f, 1.00000000000000000000000000000000000000000f,
										 1.000000000000000000000000000000000000000f, 1.00000000000000000000000000000000000000000f, 1.00000000000000000000000000000000000000000f,
										 -1.551018941718390165007690484344493597746f, -1.340073814272662700020077863882761448622f, -1.993766438945077545952244690852239727974f,
										 0.553852820401995682431106615695171058178f,  0.584327488328232003134132810373557731509f,  0.993806211147943097117263278050813823938f};




/*----------------------------------------------------------------*/
/*How to obtain filter coefficients:
 * http://matlab.izmiran.ru/help/toolbox/signal/ss2sos.html
 * With Matlab, execute:
 * order = 4;
	fc1= 0.5;
	fc2 = 40;
	[A,B,C,D] = butter(order,[fc1 fc2]/(fs/2));
	sos_bp =  ss2sos(A,B,C,D); %only for filtfilt(sos,g, data);
 *
 *sos_bp contains the coefficient matrix. Obs.: remove a0 and invert manually a1 and a2 coefficient signals as CMSIS only uses additions in its implementation
 *Print coefficients using sprintf. e.g.: sprintf('%1.50f', a(3))
 */


#define SOS_N_BIQUAD	4 //4th order
#define SOS_N_SAMPLES_NEEDED	4*SOS_N_BIQUAD  //Biquad Cascade IIR Filters Using Direct Form I Structure uses 4 state variables per stage

//Fs=250, 4th order, [0.5-40]Hz
static float sos_filter_arm_biquad[SOS_N_BIQUAD*6] =
		{0.02196126343374193295998608732588763814419507980347f,
		-0.04392252691297029998374057413457194343209266662598f,
		0.02196126347882462401250869277191668516024947166443f,
		0.62155528072656740512513806606875732541084289550781f,
		-0.13058435334361473412201348764938302338123321533203f,

		1.00000000000000000000000000000000000000000000000000f,
		2.00039113054029904503750003641471266746520996093750f,
		1.00039120704285888763251932687126100063323974609375f,
		0.81765887074618748275867119446047581732273101806641f,
		-0.51945971659458167746237222672789357602596282958984f,

		1.00000000000000000000000000000000000000000000000000f,
		1.99960886945970051087329011352267116308212280273438f,
		0.99960894594024474191229501229827292263507843017578f,
		1.97651448901705517613436313695274293422698974609375f,
		-0.97667685047967756073461487176246009767055511474609f,

		1.00000000000000000000000000000000000000000000000000f,
		-1.99999999792878746518454136094078421592712402343750f,
		0.99999999794717175927161179060931317508220672607422f,
		1.99042587386065328303175192559137940406799316406250f,
		-0.99058406025127909710903395534842275083065032958984f

		};
		
static float sos_filter_arm_biquad_second[SOS_N_BIQUAD*6] =
		{0.000740420144926795f,
		-0.00148087141911026f,
		0.000740451274183463f,
		1.49862036800099f,
		-0.585962616233252f,

		1.0f,
		2.00000000562823f,
		1.00000000540829f,
		1.62746875583534f,
		-0.790694121515007f,

		1.0f,
		-1.99995795730724f,
		0.999957959074783f,
		1.84595738323233,
		-0.854898345964462f,

		1.0f,
		1.99999999437177f,
		0.999999994591717f,
		1.95291228022445f,
		-0.958515665374536f

		};

#define SGFilter_NCOEF 41
static float sgFilt_b[SGFilter_NCOEF] = {
-0.03233125354509362031629748912564537022262811660767,
-0.02382302892796366380889772074169741244986653327942,
-0.01575112352196866502551664268594322493299841880798,
-0.00811553732710849559661703267465782118961215019226,
-0.00091627034338320647970099752654959956998936831951,
0.00584667742920721555249796708153553481679409742355,
0.01217330599066277288522464061770733678713440895081,
0.01806361534098346530163858858486491953954100608826,
0.02351760548016929019965459701779764145612716674805,
0.02853527640822025104871961787011969136074185371399,
0.03311662812513634090993974723460269160568714141846,
0.03726166063091757019165584097208920866250991821289,
0.04097037392556393542442094712896505370736122131348,
0.04424276800907542966934116179800184909254312515259,
0.04707884288145206680420429279365635011345148086548,
0.04947859854269381907343472448701504617929458618164,
0.05144203499280072117150197641421982552856206893921,
0.05296915223177276615951214466804231051355600357056,
0.05405995025960992628188961361956899054348468780518,
0.05471442907631222235531609499048499856144189834595,
0.05493258868187966131868549268801871221512556076050,
0.05471442907631222235531609499048499856144189834595,
0.05405995025960992628188961361956899054348468780518,
0.05296915223177275922061824076081393286585807800293,
0.05144203499280072811039588032144820317625999450684,
0.04947859854269383989011643620870017912238836288452,
0.04707884288145208068199210060811310540884733200073,
0.04424276800907544354712896961245860438793897628784,
0.04097037392556394236331485103619343135505914688110,
0.03726166063091757713054974487931758631020784378052,
0.03311662812513634784883365114183106925338506698608,
0.02853527640822025451816656982373388018459081649780,
0.02351760548016929713854850092502601910382509231567,
0.01806361534098346530163858858486491953954100608826,
0.01217330599066277288522464061770733678713440895081,
0.00584667742920721208305101512792134599294513463974,
-0.00091627034338321797224402587289660004898905754089,
-0.00811553732710851120912831646592167089693248271942,
-0.01575112352196867196441054659317160258069634437561,
-0.02382302892796368462557943246338254539296030998230,
-0.03233125354509357174404016177504672668874263763428};

#endif
