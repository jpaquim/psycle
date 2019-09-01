//============================================================================
//
//				CVoice.h
//				--------
//				druttis@darkface.pp.se
//
//============================================================================

#pragma once

#include "../CDsp.h"
#include "../CEnvelope.h"
#include "../DspLib/Phaser.h"
#include "../DspLib/Chorus.h"
#include "../DspLib/afloat.h"
#include <cmath>

//============================================================================
//				Defines
//============================================================================

#define NUMWAVEFORMS 8
#define WAVESIZE 4096
#define WAVEMASK 4095
extern float wavetable[NUMWAVEFORMS][WAVESIZE];

extern double coeff[5][11];

//============================================================================
//				Voice globals
//============================================================================

struct GLOBALS
{
	//
	//				This should always be here
	int												samplingrate;
	//
	//				Declare your global synth variables here
	int												osc_wave[6];
	float								osc_phase[6];
	float								osc_semi[6];
	float								osc_fine[6];
	float								osc_level[6];
	//
	float								vca_attack;
	float								vca_decay;
	float								vca_sustain;
	float								vca_release;
	float								amp_level;
	//
	float								vcf_attack;
	float								vcf_decay;
	float								vcf_sustain;
	float								vcf_release;
	float								vcf_amount;
	//
	int												filter_type;
	afloat								filter_freq;
	afloat								filter_res;
	float								filter_increment;
	float								filter_amount;
	float								buf[1024];

};
//============================================================================
//				CVoice class
//============================================================================
class CVoice
{
	//////////////////////////////////////////////////////////////////
	//				Variables
	//////////////////////////////////////////////////////////////////

public:

	//----------------------------------------------------------------
	//				These two should always be here
	//----------------------------------------------------------------

	GLOBALS								*globals;
	int												ticks_remaining;

	//----------------------------------------------------------------
	//				Declare your runtime variables here
	//----------------------------------------------------------------

	//				Velocity
	float								velocity;

	//				Oscilator
	float								osc_phase[6];
	float								osc_increment[6];

	//				VCA
	CEnvelope				vca;

	//				VCF
	CEnvelope				vcf;

	//				Filter
	FILTER								filter;
	float								filter_phase;

	double								memory[6][10];
	//////////////////////////////////////////////////////////////////
	//				Methods
	//////////////////////////////////////////////////////////////////

public:

	CVoice();
	~CVoice();
	void Stop();
	void NoteOff();
	void NoteOn(int note, int volume);

	//////////////////////////////////////////////////////////////////
	//				IsFinished
	//				returns true if voice is done playing
	//////////////////////////////////////////////////////////////////

	inline bool IsActive()
	{
		return !vca.IsFinished();
	}

	//////////////////////////////////////////////////////////////////
	//				GlobalTick
	//				Method to handle parameter inertia and suchs things
	//////////////////////////////////////////////////////////////////

	inline static void GlobalTick()
	{
	}

	//////////////////////////////////////////////////////////////////
	//				VoiceTick
	//				Method to handle voice specific things as LFO and envelopes
	//				* tips, dont handle amplitude envelopes or lfo's here
	//////////////////////////////////////////////////////////////////

	inline void VoiceTick()
	{
		//------------------------------------------------------------
		//				Setup filter
		//------------------------------------------------------------

		float filter_freq = globals->filter_freq.current;

		//				VCF
		filter_freq += (vcf.Next() * globals->vcf_amount);

		//				Filter - LFO (rate/amount)
		filter_freq += get_sample_l(wavetable[0], filter_phase, WAVEMASK) * globals->filter_amount;
		filter_phase = fand(filter_phase + globals->filter_increment, WAVEMASK);

		//				Init filter
		CDsp::InitFilter(&filter, filter_freq, globals->filter_res.current);

		//------------------------------------------------------------
		//				Setup phaser
		//------------------------------------------------------------
	}

	//////////////////////////////////////////////////////////////////
	//				Formant filter
	//////////////////////////////////////////////////////////////////

	inline float formant_filter(double *memory, float in, int vnum)
	{
		float ret = (float) (
			coeff[vnum][0] * in +
			coeff[vnum][1] * memory[0] +
			coeff[vnum][2] * memory[1] +
			coeff[vnum][3] * memory[2] +
			coeff[vnum][4] * memory[3] +
			coeff[vnum][5] * memory[4] +
			coeff[vnum][6] * memory[5] +
			coeff[vnum][7] * memory[6] +
			coeff[vnum][8] * memory[7] +
			coeff[vnum][9] * memory[8] +
			coeff[vnum][10] * memory[9]
		);
		memory[9] = memory[8];
		memory[8] = memory[7];
		memory[7] = memory[6];
		memory[6] = memory[5];
		memory[5] = memory[4];
		memory[4] = memory[3];
		memory[3] = memory[2];
		memory[2] = memory[1];
		memory[1] = memory[0];
		memory[0] = ret;
		return ret;
	}

	//////////////////////////////////////////////////////////////////
	//
	//
	//////////////////////////////////////////////////////////////////
	inline void generate_osc(int i, int nsamples)
	{
		float *pbuf = globals->buf;
		--pbuf;
		float a = globals->osc_level[i];
		int n = globals->osc_wave[i];
		double *mem = memory[i];
		if (n >= 8)
		{
			n -= 8;
			do
			{
				*++pbuf += formant_filter(mem, get_sample_l(wavetable[3], osc_phase[i], WAVEMASK), n) * 0.25f * a;
				osc_phase[i] += osc_increment[i];
			}
			while (--nsamples);
		}
		else
		{
			do
			{
				*++pbuf += get_sample_l(wavetable[n], osc_phase[i], WAVEMASK) * a;
				osc_phase[i] += osc_increment[i];
			}
			while (--nsamples);
		}
		osc_phase[i] = fand(osc_phase[i], WAVEMASK);
	}

	//				Work
	//				all sound generation is done here
	//////////////////////////////////////////////////////////////////

	inline void Work(float *psamplesleft, float *psamplesright, int numsamples)
	{
		register float out;
		//
		register int amount;
		//
		register float *pbuf;

		//--------------------------------------------------------
		//				Clear buf
		//--------------------------------------------------------
/*
		amount = numsamples;
		pbuf = globals->buf;
		--pbuf;
		do
		{
			*++pbuf = 0.0f;
		}
		while (--amount);
*/
		//--------------------------------------------------------
		//				Osc add
		//--------------------------------------------------------

		amount = numsamples;
		pbuf = globals->buf;
		--pbuf;

		do
		{
			out  = get_sample_l(wavetable[globals->osc_wave[0]], osc_phase[0], WAVEMASK) * globals->osc_level[0];
			out += get_sample_l(wavetable[globals->osc_wave[1]], osc_phase[1], WAVEMASK) * globals->osc_level[1];
			out += get_sample_l(wavetable[globals->osc_wave[2]], osc_phase[2], WAVEMASK) * globals->osc_level[2];
			out += get_sample_l(wavetable[globals->osc_wave[3]], osc_phase[3], WAVEMASK) * globals->osc_level[3];
			out += get_sample_l(wavetable[globals->osc_wave[4]], osc_phase[4], WAVEMASK) * globals->osc_level[4];
			out += get_sample_l(wavetable[globals->osc_wave[5]], osc_phase[5], WAVEMASK) * globals->osc_level[5];
			*++pbuf = out;
			osc_phase[0] += osc_increment[0];
			osc_phase[1] += osc_increment[1];
			osc_phase[2] += osc_increment[2];
			osc_phase[3] += osc_increment[3];
			osc_phase[4] += osc_increment[4];
			osc_phase[5] += osc_increment[5];
		}
		while (--amount);

		osc_phase[0] = fand(osc_phase[0], WAVEMASK);
		osc_phase[1] = fand(osc_phase[1], WAVEMASK);
		osc_phase[2] = fand(osc_phase[2], WAVEMASK);
		osc_phase[3] = fand(osc_phase[3], WAVEMASK);
		osc_phase[4] = fand(osc_phase[4], WAVEMASK);
		osc_phase[5] = fand(osc_phase[5], WAVEMASK);
/*
		generate_osc(0, numsamples);
		generate_osc(1, numsamples);
		generate_osc(2, numsamples);
		generate_osc(3, numsamples);
		generate_osc(4, numsamples);
		generate_osc(5, numsamples);
*/
		//------------------------------------------------------------
		//				Filter
		//------------------------------------------------------------

		pbuf = globals->buf;

		switch (globals->filter_type)
		{
			case 0:
				CDsp::LPFilter12(&filter, pbuf, numsamples);
				break;
			case 1:
				CDsp::LPFilter24(&filter, pbuf, numsamples);
				break;
			case 2:
				CDsp::HPFilter12(&filter, pbuf, numsamples);
				break;
			case 3:
				CDsp::HPFilter24(&filter, pbuf, numsamples);
				break;
		}

		//------------------------------------------------------------
		//				Output
		//------------------------------------------------------------

		--pbuf;

		amount = numsamples;

		do {
			//--------------------------------------------------------
			//				Amp
			//--------------------------------------------------------
			
			out = *++pbuf;
			out *= vca.Next();
			out *= globals->amp_level;
			out *= velocity;
			out *= 16384;

			//--------------------------------------------------------
			//				Write
			//--------------------------------------------------------

			*++psamplesleft += out;
			*++psamplesright += out;

		} while (--numsamples);
	}
};
