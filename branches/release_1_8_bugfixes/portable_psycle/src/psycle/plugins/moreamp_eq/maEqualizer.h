//////////////////////////////////////////////////////////////////////
// Modified for PSYCLE by Sartorius
//

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

#define EQSLIDERFIRST 0					// first pcm equalizer slider index
#define EQSLIDERLAST (EQBANDS31 - 1)	// last  pcm equalizer slider index
#define EQNAMELEN 32					// max length of name of equalizer preset
#define EQSLIDERMAX 64					// pcm equalizer slider limits
#define EQSLIDERMIN 0
#define EQNUMPRESETS 16					// sets of pcm equalizer slider values

#define EQ_MAX_BANDS EQBANDS31
#define EQ_CHANNELS 2

typedef struct 
{
	float beta;
	float alpha; 
	float gamma;
}sIIRCoefficients;

/* Coefficient history for the IIR filter */
typedef struct
{
	float x[3]; /* x[n], x[n-1], x[n-2] */
	float y[3]; /* y[n], y[n-1], y[n-2] */
}sXYData;

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
	{ (float)(2.4201241845e-01), (float)(3.7899379077e-01), (float)(-8.0847117831e-01) },	// 16000.0 Hz
};

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
	{ (float)(6.1776462189e-01), (float)(1.9111768906e-01), (float)(-1.5492495659e+00) },	// 20000.0 Hz
};

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

//double DBfromScaleGain(int value);

inline float DBfromScaleGain(int value)
{
	if(value >= EQSLIDERMIN && value <= EQSLIDERMAX)
		return (float)slidertodb[value];
	return 0.0f;
}

/* Round function provided by Frank Klemm which saves around 100K
 * CPU cycles in my PIII for each call to the IIR function
 */
inline int round_trick(float floatvalue_to_round)
{
	float	floattmp ;
	int  rounded_value ;
                                                                                
        floattmp      = (int) 0x00FD8000L + (floatvalue_to_round);
        rounded_value = *(int*)(&floattmp) - (int)0x4B7D8000L;
                                                                                
        if(rounded_value != (short) rounded_value )
            rounded_value = ( rounded_value >> 31 ) ^ 0x7FFF;
		return rounded_value;
}

#endif // __MAEQUALIZER_H__

// eof

