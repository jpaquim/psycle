//////////////////////////////////////////////////////////////////////
// MoreAmp EQ effect plugin for PSYCLE by Sartorius
//
//   Original

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

/*
 *   PCM time-domain equalizer
 *
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
 *   $Id: moreamp_eq.cpp 3616 2006-12-17 20:15:14Z johan-boule $$
 */
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "maEqualizer.h"

#define NUMPARAMETERS 36




CMachineParameter const paraB20 = 
{ 
	"20 Hz",
	"20 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB25 = 
{ 
	"25 Hz",
	"25 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB31 = 
{ 
	"* 31.5 Hz",
	"* 31.5 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB40 = 
{ 
	"40 Hz ",
	"40 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB50 = 
{ 
	"50 Hz",
	"50 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB62 = 
{ 
	"* 62.5 Hz",
	"* 62.5 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};
CMachineParameter const paraB80 = 
{ 
	"80 Hz",
	"80 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB100 = 
{ 
	"100 Hz",
	"100 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB125 = 
{ 
	"* 125 Hz",
	"* 125 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};
CMachineParameter const paraB160 = 
{ 
	"160 Hz",
	"160 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB200 = 
{ 
	"200 Hz",
	"200 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB250 = 
{ 
	"* 250 Hz",
	"* 250 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};
CMachineParameter const paraB315 = 
{ 
	"315 Hz",
	"315 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB400 = 
{ 
	"400 Hz",
	"400 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB500 = 
{ 
	"* 500 Hz",
	"* 500 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};
CMachineParameter const paraB630 = 
{ 
	"630 Hz",
	"630 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB800 = 
{ 
	"800 Hz",
	"800 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB1000 = 
{ 
	"* 1000 Hz",
	"* 1000 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};
CMachineParameter const paraB1250 = 
{ 
	"1250 Hz",
	"1250 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB1600 = 
{ 
	"1600 Hz",
	"1600 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB2000 = 
{ 
	"* 2000 Hz",
	"* 2000 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};
CMachineParameter const paraB2500 = 
{ 
	"2500 Hz",
	"2500 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB3150 = 
{ 
	"3150 Hz",
	"3150 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB4000 = 
{ 
	"* 4000 Hz",
	"* 4000 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};
CMachineParameter const paraB5000 = 
{ 
	"5000 Hz",
	"5000 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB6300 = 
{ 
	"6300 Hz",
	"6300 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB8000 = 
{ 
	"* 8000 Hz",
	"* 8000 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};
CMachineParameter const paraB10000 = 
{ 
	"10000 Hz",
	"10000 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB12500 = 
{ 
	"12500 Hz",
	"12500 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};


CMachineParameter const paraB16000 = 
{ 
	"* 16000 Hz",
	"* 16000 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraB20000 = 
{ 
	"20000 Hz",
	"20000 Hz",						// description
	0,												// MinValue	
	64,											// MaxValue
	MPF_STATE,										// Flags
	32,
};

CMachineParameter const paraDiv = 
{ 
	"",
	"",						// description
	0,												// MinValue	
	0,											// MaxValue
	MPF_STATE,										// Flags
	0,
};

CMachineParameter const paraPreAmp = 
{ 
	"Preamp",
	"Preamp",						// description
	-16,												// MinValue	
	16,											// MaxValue
	MPF_STATE,										// Flags
	0,
};

CMachineParameter const paraBands = 
{ 
	"Bands",
	"Bands",						// description
	0,												// MinValue	
	1,											// MaxValue
	MPF_STATE,										// Flags
	0,
};

CMachineParameter const paraExtra = 
{ 
	"Extra filtering",
	"Extra filtering",						// description
	0,												// MinValue	
	1,											// MaxValue
	MPF_STATE,										// Flags
	0,
};

CMachineParameter const paraLink = 
{ 
	"Link",
	"Link",						// description
	0,												// MinValue	
	1,											// MaxValue
	MPF_STATE,										// Flags
	0,
};
#define B20 0
#define B25 1
#define B31 2
#define B40 3
#define B50 4
#define B62 5
#define B80 6
#define B100 7
#define B125 8
#define B160 9
#define B200 10
#define B250 11
#define B315 12
#define B400 13
#define B500 14
#define B630 15
#define B800 16
#define B1000 17
#define B1250 18
#define B1600 19
#define B2000 20
#define B2500 21
#define B3150 22
#define B4000 23
#define B5000 24
#define B6300 25
#define B8000 26
#define B10000 27
#define B12500 28
#define B16000 29
#define B20000 30

#define _DIV 31
#define PRE_AMP 32
#define BANDS 33
#define EXTRA 34
#define LINK 35

CMachineParameter const *pParameters[] = 
{ 
	// global
		&paraB20,
		&paraB25,
		&paraB31,
		&paraB40,
		&paraB50,
		&paraB62,
		&paraB80,
		&paraB100,
		&paraB125,
		&paraB160,
		&paraB200,
		&paraB250,
		&paraB315,
		&paraB400,
		&paraB500,
		&paraB630,
		&paraB800,
		&paraB1000,
		&paraB1250,
		&paraB1600,
		&paraB2000,
		&paraB2500,
		&paraB3150,
		&paraB4000,
		&paraB5000,
		&paraB6300,
		&paraB8000,
		&paraB10000,
		&paraB12500,
		&paraB16000,
		&paraB20000,
		&paraDiv,
		&paraPreAmp,
		&paraBands,
		&paraExtra,
		&paraLink
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	EFFECT,										// flags
	NUMPARAMETERS,								// numParameters
	pParameters,								// Pointer to parameters
#ifdef _DEBUG
	"MoreAmp EQ (Debug build)",		// name
#else
	"MoreAmp EQ",						// name
#endif
	"maEQ",							// short name
	"Felipe Rivera/pmisteli/Sartorius",							// author
	"Help",								// A command, that could be use for open an editor, etc...
	3
};


class mi : public CMachineInterface
{
public:

	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);

private:
	virtual void maEqualizerReset();
	virtual void maEqualizerSetLevels();
	
	void calc_coeffs();
	inline void find_f1_and_f2(double f0, double octave_percent, double *f1, double *f2);
	inline int find_root(double a, double b, double c, double *x0);
	void clean_history();

	short bnds;
	sIIRCoefficients *iir_cf;
	float *gains31;
	float *gains10;

	float *gains;

	char g_eqi, g_eqj, g_eqk;
	int di;
	float dither[MAX_BUFFER_LENGTH];

	int srate;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[NUMPARAMETERS];
	//iir_cf = new sIIRCoefficients;
	this->gains31 = new float[EQBANDS31];
	this->gains10 = new float[EQBANDS10];
	this->gains = this->gains10;
	this->iir_cf = NULL;
}

mi::~mi()
{
	delete Vals;
	//delete iir_cf;
	delete gains31;
	delete gains10;
}

void mi::Init()
{
// Initialize your stuff here

	this->srate = pCB->GetSamplingRate();

	this->calc_coeffs();

	this->clean_history();

	this->bnds = EQBANDS10;

	this->maEqualizerSetLevels();

	this->maEqualizerReset();

}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
	if(this->srate != pCB->GetSamplingRate())
	{
		this->calc_coeffs();
		this->clean_history();
	}
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
char buffer[2048];

std::sprintf(
		buffer,"%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"Extra filtering:",
		"\nSet this to enable extra-filtering,",
		"\non either 31-band or 10-band equalization.",
		"\nNote that this doubles the processor time for filtering.",
		"\n",
		"\nLink:",
		"\nIf this is on and a knob with a (*) is moved,",
		"\nthen the 2 knobs on either side will be linked to it and move as well.",
		"\nThis means that you can set a fairly smooth",
		"\n31-point curve by moving only ten knobs.",
		"\nNote that 31 knobs are displayed even when the Bands is 31,",
		"\nin this case only the ten knobs with a (*) are active.",
		"\n"
		);

pCB->MessBox(buffer,"MoreAmp EQ",0);
}

void mi::ParameterTweak(int par, int val)
{
	int delta;
	Vals[par]=val;

	// terrible construction... 
	if (Vals[LINK]==1)
	{
		switch(par)
		{
			case B31:
				
			Vals[B20] = Vals[B25] = val;
					
			delta = (val - Vals[B62]) / 3;
			
			Vals[B40] = Vals[B62] + delta + delta;
						
			Vals[B50] = Vals[B62] + delta;		
			break;

			case B62:
			case B125:
			case B250:
			case B500:
			case B1000:
			case B2000:
			case B4000:
			case B8000:
				
				delta = (Vals[par] - Vals[par - 3]) / 3;

				Vals[par-2] = Vals[par - 3] + delta;		
				
				Vals[par - 1] = Vals[par - 3] + delta + delta;
							
				delta = (Vals[par] - Vals[par + 3]) / 3;
				
				Vals[par+1] = Vals[par + 3] + delta + delta;
							
				Vals[par+2] = Vals[par + 3] + delta;		
			break;

			case B16000:

				delta = (Vals[B16000] - Vals[B8000]) / 3;
				
				Vals[B10000] = Vals[B8000] + delta;
							
				Vals[B12500] = Vals[B8000] + delta + delta;
					
				Vals[B20000]= val;		
			break;

			default:
				break;
		}
		this->maEqualizerSetLevels();
	} else if (par<_DIV)
	{
		this->maEqualizerSetLevels();
	} else 
	{
		switch(par)
		{
			case _DIV: break;
			case PRE_AMP: this->maEqualizerSetLevels(); break; // set preamp
			case BANDS: bnds = (val==0?EQBANDS10:EQBANDS31); this->maEqualizerReset(); break;
			//case EXTRA: extra = (val==1); break;
			//case LINK: lnk = val; break;
			default:
				break;
		}
	}
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	short index, band;
	float out_l,out_r;

	int const extra = Vals[EXTRA];

	/**
	 * IIR filter equation is
	 * y[n] = 2 * (alpha*(x[n]-x[n-2]) + gamma*y[n-1] - beta*y[n-2])
	 *
	 * NOTE: The 2 factor was introduced in the coefficients to save
	 * 			a multiplication
	 *
	 * This algorithm cascades two filters to get nice filtering
	 * at the expense of extra CPU cycles
	 */
	/* 16bit, 2 bytes per sample, so divide by two the length of
	 * the buffer (length is in bytes)
	 */

	if(extra)
	{
			for(index = 0; index < numsamples; index++)
			{
					out_l = out_r = 0;
					*psamplesleft += dither[di];
					*psamplesright += dither[di];
					
					/* For each band */
					for(band = 0; band < bnds; band++)
					{
						// left ch

						/* Store Xi(n) */
						data_history[band][0].x[g_eqi] = *psamplesleft;
						/* Calculate and store Yi(n) */
			
						data_history[band][0].y[g_eqi] =
						(
						/* 		= alpha * [x(n)-x(n-2)] */
								iir_cf[band].alpha * ( data_history[band][0].x[g_eqi]
						- data_history[band][0].x[g_eqk])
						/* 		+ gamma * y(n-1) */
						+ iir_cf[band].gamma * data_history[band][0].y[g_eqj]
						/* 		- beta * y(n-2) */
						- iir_cf[band].beta * data_history[band][0].y[g_eqk]
								);

						/* 
						* The multiplication by 2.0 was 'moved' into the coefficients to save
						* CPU cycles here */
						/* Apply the gain  */
						out_l +=  data_history[band][0].y[g_eqi] * gains[band]; // * 2.0;
						//denormal = -denormal;


						// right ch

						/* Store Xi(n) */
						data_history[band][1].x[g_eqi] = *psamplesright;
						/* Calculate and store Yi(n) */
			
						data_history[band][1].y[g_eqi] =
							(
						/* 		= alpha * [x(n)-x(n-2)] */
								iir_cf[band].alpha * ( data_history[band][1].x[g_eqi]
								-  data_history[band][1].x[g_eqk])
						/* 		+ gamma * y(n-1) */
							+ iir_cf[band].gamma * data_history[band][1].y[g_eqj]
						/* 		- beta * y(n-2) */
							- iir_cf[band].beta * data_history[band][1].y[g_eqk]
								);//+ denormal;

						/* 
						* The multiplication by 2.0 was 'moved' into the coefficients to save
						* CPU cycles here */
						/* Apply the gain  */
						out_r +=  data_history[band][1].y[g_eqi] * gains[band]; // * 2.0;
						//denormal = -denormal;

					} /* For each band */


					/* Filter the sample again */
					for(band = 0; band < bnds; band++)
					{
						// left ch
						/* Store Xi(n) */
						data_history2[band][0].x[g_eqi] = out_l;
						/* Calculate and store Yi(n) */
						data_history2[band][0].y[g_eqi] = 
						(
						/* y(n) = alpha * [x(n)-x(n-2)] */
								iir_cf[band].alpha * (data_history2[band][0].x[g_eqi]
							-  data_history2[band][0].x[g_eqk])
						/*     + gamma * y(n-1) */
								+ iir_cf[band].gamma * data_history2[band][0].y[g_eqj]
				 		/* 		- beta * y(n-2) */
							- iir_cf[band].beta * data_history2[band][0].y[g_eqk]
								); // + denormal;


						/* Apply the gain */
						out_l +=  data_history2[band][0].y[g_eqi]*gains[band];

						// right ch

						/* Store Xi(n) */

						data_history2[band][1].x[g_eqi] = out_r;

						/* Calculate and store Yi(n) */
						data_history2[band][1].y[g_eqi] = 
						(
						/* y(n) = alpha * [x(n)-x(n-2)] */
								iir_cf[band].alpha * (data_history2[band][1].x[g_eqi]
							-  data_history2[band][1].x[g_eqk])
						/* 	    + gamma * y(n-1) */
						+ iir_cf[band].gamma * data_history2[band][1].y[g_eqj]
						/* 		- beta * y(n-2) */
								- iir_cf[band].beta * data_history2[band][1].y[g_eqk]
								); // + denormal;

						/* Apply the gain */
						out_r +=  data_history2[band][1].y[g_eqi]*gains[band];

					} /* For each band */
					/* Volume stuff
					Scale down original PCM sample and add it to the filters 
					output. This substitutes the multiplication by 0.25
					*/

					out_l += *psamplesleft*.25f; 
					out_r += *psamplesright*.25f;

					out_l -= dither[di]*.25f; 
					out_r -= dither[di]*.25f;

					/* Limit the output */
					if(out_l < -32768.f)
						out_l = -32768.f;
					else if(out_l > 32767.f)
						out_l = 32767.f;

					/* Limit the output */
					if(out_r < -32768.f)
						out_r = -32768.f;
					else if(out_r > 32767.f)
						out_r = 32767.f;

					*psamplesleft = out_l;
					*psamplesright = out_r;


				g_eqi = (g_eqi + 1) % 3;
				g_eqj = (g_eqj + 1) % 3;
				g_eqk = (g_eqk + 1) % 3;

				di = (di + 1) % MAX_BUFFER_LENGTH;

				++psamplesleft;
				++psamplesright;
				
			}/* For each pair of samples */
	} else {
			for(index = 0; index < numsamples; index++)
			{
					out_l = out_r = 0;

					*psamplesleft += dither[di];
					*psamplesright += dither[di];

					/* For each band */
					for(band = 0; band < bnds; band++)
					{
						// left ch

						/* Store Xi(n) */
						data_history[band][0].x[g_eqi] = *psamplesleft;
						/* Calculate and store Yi(n) */
		 
						data_history[band][0].y[g_eqi] =
						(
						/* 		= alpha * [x(n)-x(n-2)] */
								iir_cf[band].alpha * ( data_history[band][0].x[g_eqi]
								-  data_history[band][0].x[g_eqk])
						/* 		+ gamma * y(n-1) */
								+ iir_cf[band].gamma * data_history[band][0].y[g_eqj]
						/* 		- beta * y(n-2) */
								- iir_cf[band].beta * data_history[band][0].y[g_eqk]
								);

						/* 
						* The multiplication by 2.0 was 'moved' into the coefficients to save
						* CPU cycles here */
						/* Apply the gain  */
						out_l +=  data_history[band][0].y[g_eqi] * gains[band]; // * 2.0;
						//denormal = -denormal;


						// right ch

						/* Store Xi(n) */
						data_history[band][1].x[g_eqi] = *psamplesright;
						/* Calculate and store Yi(n) */

						data_history[band][1].y[g_eqi] =
							(
						/*		= alpha * [x(n)-x(n-2)] */
								iir_cf[band].alpha * ( data_history[band][1].x[g_eqi]
								-  data_history[band][1].x[g_eqk])
						/* 		+ gamma * y(n-1) */
								+ iir_cf[band].gamma * data_history[band][1].y[g_eqj]
						/* 		- beta * y(n-2) */
								- iir_cf[band].beta * data_history[band][1].y[g_eqk]
								);

						/* 
						* The multiplication by 2.0 was 'moved' into the coefficients to save
						* CPU cycles here */
						/* Apply the gain  */
						out_r +=  data_history[band][1].y[g_eqi] * gains[band]; // * 2.0;
						//denormal = -denormal;

					} /* For each band */

					/* Volume stuff
					Scale down original PCM sample and add it to the filters 
					output. This substitutes the multiplication by 0.25
					*/

					out_l += *psamplesleft*.25f; 
					out_r += *psamplesright*.25f;

					out_l -= dither[di]*.25f; 
					out_r -= dither[di]*.25f;

					/* Limit the output */
					if(out_l < -32768.f)
						out_l = -32768.f;
					else if(out_l > 32767.f)
						out_l = 32767.f;

					/* Limit the output */
					if(out_r < -32768.f)
						out_r = -32768.f;
					else if(out_r > 32767.f)
						out_r = 32767.f;

					*psamplesleft = out_l;
					*psamplesright = out_r;


				g_eqi = (g_eqi + 1) % 3;
				g_eqj = (g_eqj + 1) % 3;
				g_eqk = (g_eqk + 1) % 3;
				
				di = (di + 1) % MAX_BUFFER_LENGTH;

				++psamplesleft;
				++psamplesright;
				
			}/* For each pair of samples */
	}


}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{

	switch(param)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
			std::sprintf(txt,"%.1f dB",DBfromScaleGain(value));
			return true;
		case _DIV: return false;
		case PRE_AMP:
			std::sprintf(txt,"%i dB",value);
			return true;
		case BANDS:
			std::sprintf(txt,"%i",(value==0?EQBANDS10:EQBANDS31));
			return true;
		case EXTRA:
		case LINK:
			std::sprintf(txt,value?"On":"Off");
			return true;
		default:
			return false;
	}
}

void mi::maEqualizerReset()
{
	if(bnds == EQBANDS31)
	{
		iir_cf = iir_cf31;
		gains = gains31;
	}
	else
	{
		iir_cf = iir_cf10;
		gains = gains10;
	}
}

void mi::maEqualizerSetLevels()
{
	float const preamp = (float)Vals[PRE_AMP];
	for(int i=0;i<EQBANDS31;i++){
		float factor = std::pow(10.f,(preamp + DBfromScaleGain(Vals[i])) / 20.0f);
		//this->gains[i] = (factor==1.f)?0.0f:-0.2f + (factor / 5.125903437963185f);
		gains31[i] = -0.2f + (factor / 5.125903437963185f);
		}
	for(int i=0;i<EQBANDS10;i++){
		float factor = std::pow(10.f,(preamp + DBfromScaleGain(Vals[3*i+2])) / 20.0f);
		//this->gains[i] = (factor==1.f)?0.0f:-0.2f + (factor / 5.125903437963185f);
		gains10[i] = -0.2f + (factor / 5.125903437963185f);
		}
}

#define GAIN_F0 1.0
#define GAIN_F1 GAIN_F0 / M_SQRT2

//#define TETA(f) (2*M_PI*(double)f/bands[n].sfreq)
#define TETA(f) (2*M_PI*double(f/pCB->GetSamplingRate()))
#define TWOPOWER(value) (value * value)

#define BETA2(tf0, tf) \
	(TWOPOWER(GAIN_F1)*TWOPOWER(std::cos(tf0)) \
	- 2.0 * TWOPOWER(GAIN_F1) * std::cos(tf) * std::cos(tf0) \
	+ TWOPOWER(GAIN_F1) \
	- TWOPOWER(GAIN_F0) * TWOPOWER(std::sin(tf)))
#define BETA1(tf0, tf) \
	(2.0 * TWOPOWER(GAIN_F1) * TWOPOWER(std::cos(tf)) \
	+ TWOPOWER(GAIN_F1) * TWOPOWER(std::cos(tf0)) \
	- 2.0 * TWOPOWER(GAIN_F1) * std::cos(tf) * std::cos(tf0) \
	- TWOPOWER(GAIN_F1) + TWOPOWER(GAIN_F0) * TWOPOWER(sin(tf)))
#define BETA0(tf0, tf) \
	(0.25 * TWOPOWER(GAIN_F1) * TWOPOWER(std::cos(tf0)) \
	- 0.5 * TWOPOWER(GAIN_F1) * std::cos(tf) * std::cos(tf0) \
	+ 0.25 * TWOPOWER(GAIN_F1) \
	- 0.25 * TWOPOWER(GAIN_F0) * TWOPOWER(std::sin(tf)))

#define GAMMA(beta, tf0) ((0.5 + beta) * std::cos(tf0))
#define ALPHA(beta) ((0.5 - beta)/2.0)


/* Get the freqs at both sides of F0. These will be cut at -3dB */
inline void mi::find_f1_and_f2(double f0, double octave_percent, double *f1, double *f2)
{
	double octave_factor = std::pow(2.0, octave_percent/2.0);
	*f1 = f0/octave_factor;
	*f2 = f0*octave_factor;
}

/* Find the quadratic root
 * Always return the smallest root */
inline int mi::find_root(double a, double b, double c, double *x0) {
	double k = c-((b*b)/(4.*a));
	double h = -(b/(2.*a));
	double x1 = 0.;
	if (-(k/a) < 0.)
		return -1; 
	*x0 = h - std::sqrt(-(k/a));
	x1 = h + std::sqrt(-(k/a));
	if (x1 < *x0)
		*x0 = x1;
	return 0;
}

/* Calculate all the coefficients as specified in the bands[] array */
void mi::calc_coeffs()
{
	int i, n;
	double f1, f2;
	double x0;

	n = 0;
	for (; bands[n].cfs; n++)
	{
		double *freqs = (double *)bands[n].cfs;
		for (i=0; i<bands[n].band_count; i++)
		{
			if(freqs[i]<=srate/2)
			{

				/* Find -3dB frequencies for the center freq */
				find_f1_and_f2(freqs[i], bands[n].octave, &f1, &f2);
				/* Find Beta */
				if ( find_root(
					BETA2(TETA(freqs[i]), TETA(f1)), 
					BETA1(TETA(freqs[i]), TETA(f1)), 
					BETA0(TETA(freqs[i]), TETA(f1)), 
					&x0) == 0)
				{
					/* Got a solution, now calculate the rest of the factors */
					/* Take the smallest root always (find_root returns the smallest one)
					*
					* NOTE: The IIR equation is
					*	y[n] = 2 * (alpha*(x[n]-x[n-2]) + gamma*y[n-1] - beta*y[n-2])
					*  Now the 2 factor has been distributed in the coefficients
					*/
					/* Now store the coefficients */
					bands[n].coeffs[i].beta = 2.0 * x0;
					bands[n].coeffs[i].alpha = 2.0 * ALPHA(x0);
					bands[n].coeffs[i].gamma = 2.0 * GAMMA(x0, TETA(freqs[i]));
				} else {
					/* Shouldn't happen */
					bands[n].coeffs[i].beta = 0.;
					bands[n].coeffs[i].alpha = 0.;
					bands[n].coeffs[i].gamma = 0.;
				}
			} else {
				bands[n].coeffs[i].beta = 0.;
				bands[n].coeffs[i].alpha = 0.;
				bands[n].coeffs[i].gamma = 0.;
			}
		}// for i
	}//for n
}

void mi::clean_history()
{
	/* Zero the history arrays */
	std::memset(data_history, 0, sizeof(sXYData) * EQ_MAX_BANDS * EQ_CHANNELS);
	std::memset(data_history2, 0, sizeof(sXYData) * EQ_MAX_BANDS * EQ_CHANNELS);
	/* this is only needed if we use fpu code and there's no other place for
	the moment to init the dither array*/
	for (int n = 0; n < MAX_BUFFER_LENGTH; n++) {
		dither[n] = (rand() % 4) - 2;
	}
	di = 0;
	g_eqi = 0; g_eqj = 2; g_eqk = 1;
	//g_eqi = 2; g_eqj = 1; g_eqk = 0;
}
