//////////////////////////////////////////////////////////////////////
// Modified for PSYCLE by Sartorius
//
// you can try to use eq-xmms's coeffs instead of MoreAmp's.

// maEqualizer.h

/*
 * MoreAmp
 * Copyright (C) 2004-2005 pmisteli
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef __MAEQUALIZER_H__
#define __MAEQUALIZER_H__

#define EQBANDS31 31					// larger number of pcm equalizer bands (sliders)
#define EQBANDS10 10					// lesser number of pcm equalizer bands

#define EQSLIDERMAX 64					// pcm equalizer slider limits
#define EQSLIDERMIN 0

#define EQ_MAX_BANDS EQBANDS31
#define EQ_CHANNELS 2

typedef struct 
{
	float beta;
	float alpha; 
	float gamma;
}sIIRCoefficients;

/*
typedef struct 
{
	double beta;
	double alpha; 
	double gamma;
}sIIRCoefficients;
*/
/* Coefficient history for the IIR filter */
typedef struct
{
	float x[3]; /* x[n], x[n-1], x[n-2] */
	float y[3]; /* y[n], y[n-1], y[n-2] */
}sXYData;

//typedef struct
//{
//	double x[3]; /* x[n], x[n-1], x[n-2] */
//	double y[3]; /* y[n], y[n-1], y[n-2] */
//}sXYData;


static sIIRCoefficients iir_cf10[]
= {
	{ (float)(9.9723137967e-01), (float)(1.3843101653e-03), (float)(1.9972115835e+00) },	//	 31.2 Hz
	{ (float)(9.9406142155e-01), (float)(2.9692892229e-03), (float)(1.9939823631e+00) },	//	 62.5 Hz
	{ (float)(9.8774277725e-01), (float)(6.1286113769e-03), (float)(1.9874275518e+00) },	//   125.0 Hz
	{ (float)(9.7522112569e-01), (float)(1.2389437156e-02), (float)(1.9739682661e+00) },	//   250.0 Hz
	{ (float)(9.5105628526e-01), (float)(2.4471857368e-02), (float)(1.9461077269e+00) },	//   500.0 Hz
	{ (float)(9.0450844499e-01), (float)(4.7745777504e-02), (float)(1.8852109613e+00) },	//  1000.0 Hz
	{ (float)(8.1778971701e-01), (float)(9.1105141497e-02), (float)(1.7444877599e+00) },	//  2000.0 Hz
	{ (float)(6.6857185264e-01), (float)(1.6571407368e-01), (float)(1.4048592171e+00) },	//  4000.0 Hz
	{ (float)(4.4861333678e-01), (float)(2.7569333161e-01), (float)(6.0518718075e-01) },	//  8000.0 Hz
	{ (float)(2.4201241845e-01), (float)(3.7899379077e-01), (float)(-8.0847117831e-01) }	// 16000.0 Hz
};

/* //from eq-xmms
static sIIRCoefficients iir_cf10[] = {
{ 9.9688176273e-01, 1.5591186337e-03, 1.9968622855e+00 },
{ 9.9377323686e-01, 3.1133815717e-03, 1.9936954495e+00 },
{ 9.8748575691e-01, 6.2571215431e-03, 1.9871705722e+00 },
{ 9.7512812040e-01, 1.2435939802e-02, 1.9738753198e+00 },
{ 9.5087485437e-01, 2.4562572817e-02, 1.9459267562e+00 },
{ 9.0416308662e-01, 4.7918456688e-02, 1.8848691023e+00 },
{ 8.1751373987e-01, 9.1243130064e-02, 1.7442229115e+00 },
{ 6.6840529852e-01, 1.6579735074e-01, 1.4047189863e+00 },
{ 4.4858358977e-01, 2.7570820511e-01, 6.0517475334e-01 },
{ 2.4198119087e-01, 3.7900940457e-01, -8.0845085113e-01 },
};
*/

static sIIRCoefficients iir_cf31[]
= {
	{ (float)(9.9939854113e-01), (float)(3.0072943419e-04), (float)(1.9993904238e+00) },	//	  20.0 Hz
	{ (float)(9.9940551149e-01), (float)(2.9724425274e-04), (float)(1.9993928282e+00) },	//	  25.0 Hz
	{ (float)(9.9925718616e-01), (float)(3.7140692231e-04), (float)(1.9992370516e+00) },	//	  31.5 Hz
	{ (float)(9.9879758671e-01), (float)(6.0120664727e-04), (float)(1.9987651273e+00) },	//	  40.0 Hz
	{ (float)(9.9849722245e-01), (float)(7.5138877401e-04), (float)(1.9984465124e+00) },	//	  50.0 Hz
	{ (float)(9.9820192714e-01), (float)(8.9903643123e-04), (float)(1.9981214319e+00) },	//	  63.0 Hz
	{ (float)(9.9759663784e-01), (float)(1.2016810790e-03), (float)(1.9974668794e+00) },	//	  80.0 Hz
	{ (float)(9.9699670283e-01), (float)(1.5016485839e-03), (float)(1.9967940174e+00) },	//   100.0 Hz
	{ (float)(9.9608832771e-01), (float)(1.9558361473e-03), (float)(1.9957717788e+00) },	//   125.0 Hz
	{ (float)(9.9488106021e-01), (float)(2.5594698955e-03), (float)(1.9943627488e+00) },	//   160.0 Hz
	{ (float)(9.9368508558e-01), (float)(3.1574572116e-03), (float)(1.9928757293e+00) },	//   200.0 Hz
	{ (float)(9.9187386982e-01), (float)(4.0630650882e-03), (float)(1.9906104476e+00) },	//   250.0 Hz
	{ (float)(9.8977837468e-01), (float)(5.1108126620e-03), (float)(1.9877748003e+00) },	//   315.0 Hz
	{ (float)(9.8709369779e-01), (float)(6.4531511061e-03), (float)(1.9838676311e+00) },	//   400.0 Hz
	{ (float)(9.8381454728e-01), (float)(8.0927263624e-03), (float)(1.9787829025e+00) },	//   500.0 Hz
	{ (float)(9.7966277109e-01), (float)(1.0168614455e-02), (float)(1.9716932308e+00) },	//   630.0 Hz
	{ (float)(9.7404386878e-01), (float)(1.2978065611e-02), (float)(1.9612347544e+00) },	//   800.0 Hz
	{ (float)(9.6758591875e-01), (float)(1.6207040627e-02), (float)(1.9476493009e+00) },	//  1000.0 Hz
	{ (float)(9.5973086514e-01), (float)(2.0134567431e-02), (float)(1.9287337831e+00) },	//  1250.0 Hz
	{ (float)(9.4878629133e-01), (float)(2.5606854334e-02), (float)(1.8983695001e+00) },	//  1600.0 Hz
	{ (float)(9.3627068388e-01), (float)(3.1864658062e-02), (float)(1.8581910087e+00) },	//  2000.0 Hz
	{ (float)(9.2117604381e-01), (float)(3.9411978094e-02), (float)(1.8005882747e+00) },	//  2500.0 Hz
	{ (float)(9.0172987031e-01), (float)(4.9135064847e-02), (float)(1.7133994083e+00) },	//  3150.0 Hz
	{ (float)(8.7697241507e-01), (float)(6.1513792463e-02), (float)(1.5803227133e+00) },	//  4000.0 Hz
	{ (float)(8.4900435915e-01), (float)(7.5497820425e-02), (float)(1.3993428287e+00) },	//  5000.0 Hz
	{ (float)(8.1426291228e-01), (float)(9.2868543858e-02), (float)(1.1311744237e+00) },	//  6300.0 Hz
	{ (float)(7.7194867684e-01), (float)(1.1402566158e-01), (float)(7.4026698287e-01) },	//  8000.0 Hz
	{ (float)(7.2627335006e-01), (float)(1.3686332497e-01), (float)(2.5120594308e-01) },	// 10000.0 Hz
	{ (float)(6.7690084582e-01), (float)(1.6154957709e-01), (float)(-3.4981568641e-01) },	// 12500.0 Hz
	{ (float)(6.2492532874e-01), (float)(1.8753733563e-01), (float)(-1.0577231561e+00) },	// 16000.0 Hz
	{ (float)(6.1776462189e-01), (float)(1.9111768906e-01), (float)(-1.5492495659e+00) }	// 20000.0 Hz
};

/* // from eq-xmms
static sIIRCoefficients iir_cf31[]  = {

{ 9.9934037157e-01, 3.2981421662e-04, 1.9993322545e+00 },
{ 9.9917555233e-01, 4.1222383516e-04, 1.9991628705e+00 },
{ 9.9896129025e-01, 5.1935487310e-04, 1.9989411587e+00 },
{ 9.9868118265e-01, 6.5940867495e-04, 1.9986487252e+00 },
{ 9.9835175161e-01, 8.2412419683e-04, 1.9983010452e+00 },
{ 9.9792365217e-01, 1.0381739160e-03, 1.9978431682e+00 },
{ 9.9736411067e-01, 1.3179446674e-03, 1.9972343673e+00 },
{ 9.9670622662e-01, 1.6468866919e-03, 1.9965035707e+00 },
{ 9.9588448566e-01, 2.0577571681e-03, 1.9955679690e+00 },
{ 9.9473519326e-01, 2.6324033689e-03, 1.9942169198e+00 },
{ 9.9342335280e-01, 3.2883236020e-03, 1.9926141028e+00 },
{ 9.9178600786e-01, 4.1069960678e-03, 1.9905226414e+00 },
{ 9.8966154150e-01, 5.1692292513e-03, 1.9876580847e+00 },
{ 9.8689036168e-01, 6.5548191616e-03, 1.9836646251e+00 },
{ 9.8364027156e-01, 8.1798642207e-03, 1.9786090689e+00 },
{ 9.7943153305e-01, 1.0284233476e-02, 1.9714629236e+00 },
{ 9.7395577681e-01, 1.3022111597e-02, 1.9611472340e+00 },
{ 9.6755437936e-01, 1.6222810321e-02, 1.9476180811e+00 },
{ 9.5961458750e-01, 2.0192706249e-02, 1.9286193446e+00 },
{ 9.4861481164e-01, 2.5692594182e-02, 1.8982024567e+00 },
{ 9.3620971896e-01, 3.1895140519e-02, 1.8581325022e+00 },
{ 9.2095325455e-01, 3.9523372724e-02, 1.8003794694e+00 },
{ 9.0153642498e-01, 4.9231787512e-02, 1.7132251201e+00 },
{ 8.7685876255e-01, 6.1570618727e-02, 1.5802270232e+00 },
{ 8.4886734822e-01, 7.5566325889e-02, 1.3992391376e+00 },
{ 8.1417575446e-01, 9.2912122771e-02, 1.1311200817e+00 },
{ 7.7175298860e-01, 1.1412350570e-01, 7.4018523020e-01 },
{ 7.2627049462e-01, 1.3686475269e-01, 2.5120552756e-01 },
{ 6.7674787974e-01, 1.6162606013e-01, -3.4978377639e-01 },
{ 6.2482197550e-01, 1.8758901225e-01, -1.0576558797e+00 },
{ 6.1776148240e-01, 1.9111925880e-01, -1.5492465594e+00 },
};
*/
/* History for two filters */
sXYData data_history[EQ_MAX_BANDS][EQ_CHANNELS];
sXYData data_history2[EQ_MAX_BANDS][EQ_CHANNELS];

/* Coefficients */
sIIRCoefficients *iir_cf;

static float slidertodb[] = {
-16.0, -15.5, -15.0, -14.5, -14.0, -13.5, -13.0, -12.5, -12.0, -11.5, -11.0, -10.5, -10.0,  -9.5, -9.0, -8.5,
 -8.0,  -7.5,  -7.0,  -6.5,  -6.0,  -5.5,  -5.0,  -4.5,  -4.0,  -3.5,  -3.0,  -2.5,  -2.0,  -1.5, -1.0, -0.5,
  0.0,   0.5,   1.0,   1.5,   2.0,   2.5,   3.0,   3.5,   4.0,   4.5,   5.0,   5.5,   6.0,   6.5,  7.0,  7.5,
  8.0,   8.5,   9.0,   9.5,  10.0,  10.5,  11.0,  11.5,  12.0,  12.5,  13.0,  13.5,  14.0,  14.5, 15.0, 15.5,
 16.0
};

inline float DBfromScaleGain(int value)
{
	if(value >= EQSLIDERMIN && value <= EQSLIDERMAX)
		return (float)slidertodb[value];
	return 0.0f;
}

/* Round function provided by Frank Klemm which saves around 100K
 * CPU cycles in my PIII for each call to the IIR function
 */
/*
inline static int round_trick(float floatvalue_to_round)
{
	float	floattmp ;
	int  rounded_value ;
                                                                                
        floattmp      = (int) 0x00FD8000L + (floatvalue_to_round);
        rounded_value = *(int*)(&floattmp) - (int)0x4B7D8000L;
                                                                                
        if(rounded_value != (short) rounded_value )
            rounded_value = ( rounded_value >> 31 ) ^ 0x7FFF;
		return rounded_value;
}
*/

#endif // __MAEQUALIZER_H__

// eof

