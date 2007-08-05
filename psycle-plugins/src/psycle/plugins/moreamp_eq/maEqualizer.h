//////////////////////////////////////////////////////////////////////
// Modified for PSYCLE by Sartorius
//

// maEqualizer.h

/*
* MoreAmp
* Copyright     (C)     2004-2005 pmisteli
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the     GNU     General Public License as published     by
* the Free Software     Foundation;     either version 2 of     the     License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY;     without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public     License
* along with this program; if not, write to     the     Free Software
* Foundation, Inc.,     59 Temple Place, Suite 330,     Boston, MA 02111-1307 USA
*
*/

/*
*   Copyright (C) 2002-2006  Felipe Rivera <liebremx at users.sourceforge.net>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*
*   Coefficient stuff
*
*   $Id: maEqualizer.h 3616 2006-12-17 20:15:14Z johan-boule $
*/

#ifndef __MAEQUALIZER_H__
#define __MAEQUALIZER_H__

#if defined DIVERSALIS__COMPILER__MICROSOFT
#define MSVC_ALIGN __declspec(align(16))
#else
#define MSVC_ALIGN
#endif
#if defined DIVERSALIS__COMPILER__GNU
#define GNU_ALIGN __attribute__((aligned))
#else
#define GNU_ALIGN
#endif

#if !defined M_PI
#define M_PI       3.14159265358979323846
#endif

#if !defined M_SQRT2
#define M_SQRT2    1.41421356237309504880
#endif

#define EQBANDS31 31                                    // larger number of pcm equalizer bands (sliders)
#define EQBANDS10 10                                    // lesser number of pcm equalizer bands

#define EQSLIDERMAX 64                                  // pcm equalizer slider limits
#define EQSLIDERMIN 0

#define EQ_MAX_BANDS EQBANDS31
#define EQ_CHANNELS 2


struct sIIRCoefficients
{
	float beta;
	float alpha; 
	float gamma;
	float dummy;
};

/* Coefficient history for the IIR filter */
struct sXYData
{
	float x[3]; /* x[n], x[n-1], x[n-2] */
	float y[3]; /* y[n], y[n-1], y[n-2] */
	float dummy[2];
};

// [bohan] shouldn't be in a header file
static sIIRCoefficients MSVC_ALIGN iir_cf10[EQBANDS10] GNU_ALIGN;

// [bohan] shouldn't be in a header file
static sIIRCoefficients MSVC_ALIGN iir_cf31[EQBANDS31] GNU_ALIGN;

// [bohan] shouldn't be in a header file
static const double band_f010[] = { 31.,62.,125.,250.,500.,1000.,2000.,4000.,8000.,16000. };

// [bohan] shouldn't be in a header file
static const double band_f031[] = { 20.,25.,31.5,40.,50.,63.,80.,100.,125.,160.,200.,250.,315.,400.,500.,630.,800.,1000.,1250.,1600.,2000.,2500.,3150.,4000.,5000.,6300.,8000.,10000.,12500.,16000.,20000. };

/* History for two filters */
// [bohan] shouldn't be in a header file
sXYData MSVC_ALIGN data_history[EQ_MAX_BANDS][EQ_CHANNELS] GNU_ALIGN;
// [bohan] shouldn't be in a header file
sXYData MSVC_ALIGN data_history2[EQ_MAX_BANDS][EQ_CHANNELS] GNU_ALIGN;

struct band
{
	sIIRCoefficients *coeffs;
	const double *cfs;
	double octave;
	int band_count;
};

// [bohan] shouldn't be in a header file
band bands[] =
{
	{ iir_cf10,	band_f010,	1.0,		10 },
	{ iir_cf31,	band_f031,	1.0/3.0,	31 },
	{ 0 }
};

// [bohan] shouldn't be in a header file
static float slidertodb[] =
{
-16.0, -15.5, -15.0, -14.5, -14.0, -13.5, -13.0, -12.5, -12.0, -11.5, -11.0, -10.5, -10.0,  -9.5, -9.0, -8.5,
 -8.0,  -7.5,  -7.0,  -6.5,  -6.0,  -5.5,  -5.0,  -4.5,  -4.0,  -3.5,  -3.0,  -2.5,  -2.0,  -1.5, -1.0, -0.5,
  0.0,   0.5,   1.0,   1.5,   2.0,   2.5,   3.0,   3.5,   4.0,   4.5,   5.0,   5.5,   6.0,   6.5,  7.0,  7.5,
  8.0,   8.5,   9.0,   9.5,  10.0,  10.5,  11.0,  11.5,  12.0,  12.5,  13.0,  13.5,  14.0,  14.5, 15.0, 15.5,
 16.0
};

inline float DBfromScaleGain(int value)
{
	if(value >= EQSLIDERMIN && value <= EQSLIDERMAX)
			return slidertodb[value];
	return 0.0f;
}
#endif // __MAEQUALIZER_H__
