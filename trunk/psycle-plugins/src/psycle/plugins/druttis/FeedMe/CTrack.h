//============================================================================
//
//	CTrack.h
//	--------
//	druttis@darkface.pp.se
//
//============================================================================
#pragma once
//
//
#define WAVESIZE 4096
#define WAVEMASK 4095
//
//
#include "../CDsp.h"
#include "../DspMath.h"
#include "../CEnvelope.h"
#include "../wtfmlib.h"
//============================================================================
//	Defines
//============================================================================
#define NUMWAVEFORMS 6
#define MAX_OVERTONES 6
#define GLOBAL_TICKS 32
//============================================================================
//	CTrack globals
//============================================================================
struct TRACKDATA
{
	//	Just some info
	int		samplingrate;
	//	The actual track info
	int		waveform;
	afloat	feedback;
	int		overtype;
	afloat	phase;
	float	chorus;
	int		distform;
	afloat	distortion;
	float	attack;
	float	release;
	float	amplitude;
	float	vibrato_rate;
	float	vibrato_amount;
	float	vibrato_delay;
	float	vcf_attack;
	float	vcf_decay;
	float	vcf_sustain;
	float	vcf_release;
	afloat	vcf_amount;
	int		filter_type;
	afloat	filter_freq;
	afloat	filter_res;
	float	inertia;
	int		note_cut;
	int		sync_mode;
};
//============================================================================
//	CTrack class
//============================================================================
class CTrack
{
	//------------------------------------------------------------------------
	//	Static variables
	//------------------------------------------------------------------------
private:
	static float	wavetable[NUMWAVEFORMS][WAVESIZE];
	static float	overtonemults[MAX_OVERTONES];
	//------------------------------------------------------------------------
	//	Data
	//------------------------------------------------------------------------
	//	Globals
public:
	TRACKDATA		*globals;
	//	Osc 1
	float			osc1_time;
	float			osc1_speed;
	float			osc1_target_speed;
	float			osc1_out[MAX_OVERTONES + 1];
	//	Osc 2
	float			osc2_time;
	float			osc2_speed;
	float			osc2_target_speed;
	float			osc2_out[MAX_OVERTONES + 1];
	//
	float			slide_speed;
	//	Vibrato
	float			vibrato_time;
	float			vibrato_osc1_speed;
	float			vibrato_osc2_speed;
	float			vibrato_dtime;
	//	Velocity
	float			velocity;
	//	VCA
	CEnvelope		vca_env;
	//	VCF & filter
	CEnvelope		vcf_env;
	FILTER			vcf_data1;
	FILTER			vcf_data2;
	//	Phase
	float			phase_time;
	//	Amplitude (scaled for output)
	float			amplitude;
	//
	int				note_cut;
	//	Work variables
	float			vibrato_out;
	float			vcf_out;
	int				ticks_remaining;
	//------------------------------------------------------------------------
	//	Methods
	//------------------------------------------------------------------------
public:
	CTrack();
	~CTrack();
	static void Init();
	static void Destroy();
	void Stop();
	void NoteOff();
	void NoteOn(int note, int volume);
//	void Work(float *psamplesleft, float *psamplesright, int numsamples);
	//------------------------------------------------------------------------
	//	GetSampleExp with overtones and feedback
	//------------------------------------------------------------------------
	static inline float GetSample(float* wavetable, int type, float* buf, float fb, float time)
	{
		register float out = 0.0f;
		register float tmp;
		switch (type)
		{
		case 0 :
			tmp = time;
			out += buf[0] = GetWTSample(wavetable, tmp + buf[0] * fb);
			break;
		case 1 :
			tmp = time;
			out += buf[1] = GetWTSample(wavetable, tmp + buf[1] * fb);
			tmp *= 2.0f;
			out += buf[0] = GetWTSample(wavetable, tmp + buf[0] * fb);
			break;
		case 2 :
			tmp = time;
			out += buf[2] = GetWTSample(wavetable, tmp + buf[2] * fb);
			tmp *= 2.0f;
			out += buf[1] = GetWTSample(wavetable, tmp + buf[1] * fb);
			tmp *= 2.0f;
			out += buf[0] = GetWTSample(wavetable, tmp + buf[0] * fb);
			break;
		case 3 :
			tmp = time;
			out += buf[3] = GetWTSample(wavetable, tmp + buf[3] * fb);
			tmp *= 2.0f;
			out += buf[2] = GetWTSample(wavetable, tmp + buf[2] * fb);
			tmp *= 2.0f;
			out += buf[1] = GetWTSample(wavetable, tmp + buf[1] * fb);
			tmp *= 2.0f;
			out += buf[0] = GetWTSample(wavetable, tmp + buf[0] * fb);
			break;
		case 4 :
			tmp = 1;
			out += buf[1] = GetWTSample(wavetable, time * tmp + buf[1] * fb);
			++tmp;
			out += buf[0] = GetWTSample(wavetable, time * tmp + buf[0] * fb);
			break;
		case 5:
			tmp = 1;
			out += buf[2] = GetWTSample(wavetable, time * tmp + buf[2] * fb);
			++tmp;
			out += buf[1] = GetWTSample(wavetable, time * tmp + buf[1] * fb);
			++tmp;
			out += buf[0] = GetWTSample(wavetable, time * tmp + buf[0] * fb);
			break;
		}
		return out * overtonemults[type];
	}
	//------------------------------------------------------------------------
	//	Setup slide
	//------------------------------------------------------------------------
	void SetFreq(int note)
	{
		osc1_target_speed = CDsp::GetFreq((float) (note - globals->chorus), WAVESIZE, globals->samplingrate);
		osc2_target_speed = CDsp::GetFreq((float) (note + globals->chorus), WAVESIZE, globals->samplingrate);
	}
	//------------------------------------------------------------------------
	//	IsFinished
	//------------------------------------------------------------------------
	inline bool IsFinished()
	{
		return vca_env.IsFinished();
	}
	//============================================================================
	//	Work
	//============================================================================
	inline void Work(float *psamplesleft, float *psamplesright, int numsamples)
	{
		//
		//
		static float vca_out;
		static float osc1_spd;
		static float osc2_spd;
		static float osc2_tme;
		static int amount;
		register int nsamples;
		static float dist;
		static float dist2;
		static float ndis;
		register float *pleft;
		register float *pright;
		static float out1[256]; //GLOBAL_TICKS];
		static float out2[256]; //GLOBAL_TICKS];
		//
		//	Waveform
		float *pwaveform = wavetable[globals->waveform];
		if (!pwaveform)
			return;
		//
		//	Distform
		float *pdistform = wavetable[globals->distform];
		if (!pdistform)
			return;
		//
		//	Loop
		do {
			//
			//	Tick handling
			if (!ticks_remaining) {
				ticks_remaining = GLOBAL_TICKS;
				//
				//	Handle note cut
				if (note_cut > 0) {
					note_cut--;
					if (!note_cut) {
						NoteOff();
					}
				}
				//
				//	Vibrato
				if (globals->vibrato_delay == 0.0f) {
					vibrato_out = 1.0f;
				} else {
					vibrato_out = vibrato_dtime;
					if (vibrato_dtime < 1.0f) {
						vibrato_dtime += 1.0f / globals->vibrato_delay;
						if (vibrato_dtime > 1.0f)
							vibrato_dtime = 1.0f;
					}
				}
				vibrato_out *= globals->vibrato_amount * GetWTSample(wavetable[0], vibrato_time);
				vibrato_time += globals->vibrato_rate;
				while (vibrato_time >= WAVESIZE)
					vibrato_time -= WAVESIZE;
				//
				//	Vibrato -> Freq
				vibrato_osc1_speed = vibrato_out * osc1_speed * 0.125f;
				vibrato_osc2_speed = vibrato_out * osc2_speed * 0.125f;
				//
				//	Filter
				vcf_out = vcf_env.Next() * globals->vcf_amount.current;
				CDsp::InitFilter(&vcf_data1, globals->filter_freq.current + vcf_out, globals->filter_res.current);
				CDsp::InitFilter(&vcf_data2, globals->filter_freq.current + vcf_out, globals->filter_res.current);
				//
				//	Slide
				if (osc1_speed != osc1_target_speed) {
					float dest = (osc1_target_speed - osc1_speed) * slide_speed;
					osc1_speed += dest;
					dest = (osc2_target_speed - osc2_speed) * slide_speed;
					osc2_speed += dest;
				}
			}
			//
			//	Compute samples to render this iteration
			amount = numsamples;
			if (amount > ticks_remaining)
				amount = ticks_remaining;
			numsamples -= amount;
			ticks_remaining -= amount;
			//
			//	Oscilators
			osc1_spd = osc1_speed + vibrato_osc1_speed;
			osc2_spd = osc2_speed + vibrato_osc2_speed;
			//	Phase
			osc2_tme = osc2_time + globals->phase.current;
			//
			pleft = out1 - 1;
			pright = out2 - 1;
			nsamples = amount;
			do
			{
				*++pleft = GetSample(pwaveform, globals->overtype, osc1_out, globals->feedback.current, osc1_time);
				*++pright = GetSample(pwaveform, globals->overtype, osc2_out, globals->feedback.current, osc2_tme);
				osc1_time += osc1_spd;
				osc2_tme += osc2_spd;
			}
			while (--nsamples);
			//
			osc2_time = osc2_tme - globals->phase.current;
			//
			//	Distort
			dist = globals->distortion.current;
			dist2 = dist * WAVESIZE;
			ndis = 1.0f - dist;
			//
			pleft = out1 - 1;
			pright = out2 - 1;
			nsamples = amount;
			do {
				++pleft;
				++pright;
				*pleft = *pleft * ndis + GetWTSample(pdistform, *pleft * dist2) * dist;
				*pright = *pright * ndis + GetWTSample(pdistform, *pright * dist2) * dist;
			} while (--nsamples);
			//
			//	Filter
			switch (globals->filter_type)
			{
				case 0:
					CDsp::LPFilter12(&vcf_data1, out1, amount);
					CDsp::LPFilter12(&vcf_data2, out2, amount);
					break;
				case 1:
					CDsp::LPFilter24(&vcf_data1, out1, amount);
					CDsp::LPFilter24(&vcf_data2, out2, amount);
					break;
				case 4:
					CDsp::LPFilter36(&vcf_data1, out1, amount);
					CDsp::LPFilter36(&vcf_data2, out2, amount);
					break;
				case 2:
					CDsp::HPFilter12(&vcf_data1, out1, amount);
					CDsp::HPFilter12(&vcf_data2, out2, amount);
					break;
				case 3:
					CDsp::HPFilter24(&vcf_data1, out1, amount);
					CDsp::HPFilter24(&vcf_data2, out2, amount);
					break;
			}
			//
			//	Amplify & Output
			pleft = out1 - 1;
			pright = out2 - 1;
			nsamples = amount;
			do
			{
				vca_out = vca_env.Next() * amplitude;
				*++psamplesleft += *++pleft * vca_out;
				*++psamplesright += *++pright * vca_out;
			}
			while (--nsamples);
		}
		while (numsamples);
		//
		//	Limit OSC times
		while (osc1_time >= WAVESIZE)
			osc1_time -= WAVESIZE;
		while (osc2_time >= WAVESIZE)
			osc2_time -= WAVESIZE;
	}
};
