// SynthTrack.h: interface for the CSynthTrack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYNTHTRACK_H__3D2712C1_36AD_11D4_8913_AE42C3A28249__INCLUDED_)
#define AFX_SYNTHTRACK_H__3D2712C1_36AD_11D4_8913_AE42C3A28249__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "filter.h"


#define FILTER_CALC_TIME	64
#define TWOPI				6.28318530717958647692528676655901f

struct SYNPAR
{
	signed short *pWave;
	signed short *pWave2;
	int osc2detune;
	int osc2finetune;
	int osc2sync;
	int amp_env_attack;
	int amp_env_decay;
	int amp_env_sustain;
	int amp_env_release;
	int vcf_env_attack;
	int vcf_env_decay;
	int vcf_env_sustain;
	int vcf_env_release;
	int vcf_lfo_speed;
	int vcf_lfo_amplitude;
	int vcf_cutoff;
	int vcf_resonance;
	int vcf_type;
	int vcf_envmod;
	int osc_mix;
	int out_vol;
	int arp_mod;
	int arp_bpm;
	int arp_cnt;
	int osctune;
	float oscglide;
	int oscfinetune;
	int annn;
};

class CSynthTrack  
{
public:
	void* tpmi;
	InitEffect(int cmd,int val);
	PerformFx();
	DoGlide();
	DisableVibrato();
	ActiveVibrato(int speed,int depth);
	Vibrate();
	float Filter(float x);
	NoteOff();
	float GetEnvAmp();
	void GetEnvVcf();
	float oscglide;
	float GetSample();
	NoteOn(int note, SYNPAR *tspar,int spd);
	InitArpeggio();
	
	CSynthTrack();
	virtual ~CSynthTrack();
	int AmpEnvStage;
	int NoteCutTime;
	bool NoteCut;
	
	filter m_filter;
private:
	float output;
	float lfo_freq;
	float lfo_phase;

	InitLfo(int freq,int amp);

	short timetocompute;
	InitEnvelopes(bool force=false);

	float VcfResonance;
	int sp_cmd;
	int sp_val;

	float OSC1Speed;
	float OSC2Speed;
	float ROSC1Speed;
	float ROSC2Speed;
	
	float OSC1Position;
	float OSC2Position;
	
	float OSCvib;
	float VibratoGr;
	float VibratoSpeed;
	float VibratoDepth;

	// Arpeggiator
	int Arp_tickcounter;
	int Arp_samplespertick;
	int Arp_basenote;
	int ArpMode;
	unsigned char ArpCounter;
	unsigned char ArpLimit;

	signed char ArpNote[16][16];

	bool Arp;

	// Envelope [Amplitude]
	float AmpEnvValue;
	float AmpEnvCoef;
	float AmpEnvSustainLevel;
	bool vibrato;
	float OSC1Vol;
	float OSC2Vol;

	// Envelope [Amplitude]
	float VcfEnvValue;
	float VcfEnvCoef;
	float VcfEnvSustainLevel;
	int VcfEnvStage;
	float VcfEnvMod;
	float VcfCutoff;
	
	SYNPAR *syntp;

	
	inline int f2i(double d)
	{
	const double magic = 6755399441055744.0; // 2^51 + 2^52
	double tmp = (d-0.5) + magic;
	return *(int*) &tmp;
	};

};

inline float CSynthTrack::GetSample()
{
	// Oscillator1

	if(ArpMode>0 && AmpEnvStage)
	{
		if(++Arp_tickcounter>Arp_samplespertick)
		{
			InitEnvelopes();

			Arp_tickcounter=0;

			int note=Arp_basenote+ArpNote[syntp->arp_mod-1][ArpCounter];

			ArpCounter++;
			if(ArpCounter>=syntp->arp_cnt)
			ArpCounter=0;

			float notetemp = (float)note+syntp->osctune;

			OSC1Speed=(float)pow(2.0, notetemp/12.0);

			float note2=(float)note+
			(float)syntp->osc2finetune*0.0038962f+
			(float)syntp->osc2detune+
			(float)syntp->osctune;

			OSC2Speed=(float)pow(2.0, (float)note2/12.0);
		}
	}

	if(AmpEnvStage)
	{
	output=syntp->pWave2[f2i(OSC2Position)]*OSC2Vol+syntp->pWave[f2i(OSC1Position)]*OSC1Vol;

	if(vibrato)
	{
	OSC1Position+=ROSC1Speed+OSCvib;
	OSC2Position+=ROSC2Speed+OSCvib;
	}
	else
	{
	OSC1Position+=ROSC1Speed;
	OSC2Position+=ROSC2Speed;
	}
	
	if(OSC1Position>=2048.0f)
	{
		OSC1Position-=2048.0f;
	
		if(syntp->osc2sync)
		OSC2Position=OSC1Position;
	}

	if(OSC2Position>=2048.0f)
		OSC2Position-=2048.0f;

	GetEnvVcf();

	if(!timetocompute--)
	{
	lfo_phase+=lfo_freq;

	if(lfo_phase>TWOPI)lfo_phase-=TWOPI;
	
	float const VcfLfoVal=sin(lfo_phase)*(float)syntp->vcf_lfo_amplitude;
	int realcutoff=VcfCutoff+VcfLfoVal+VcfEnvMod*VcfEnvValue;

	if(realcutoff<1)realcutoff=1;
	if(realcutoff>240)realcutoff=240;
	m_filter.setfilter(syntp->vcf_type,realcutoff,syntp->vcf_resonance);
	timetocompute=FILTER_CALC_TIME;
	}

	return m_filter.res(output)*GetEnvAmp();	
	}
	else
	return 0;
}

inline float CSynthTrack::GetEnvAmp()
{
	switch(AmpEnvStage)
	{
	case 1: // Attack
		AmpEnvValue+=AmpEnvCoef;
		
		if(AmpEnvValue>1.0f)
		{
			AmpEnvCoef=(1.0f-AmpEnvSustainLevel)/(float)syntp->amp_env_decay;
			AmpEnvStage=2;
		}

		return AmpEnvValue;
	break;

	case 2: // Decay
		AmpEnvValue-=AmpEnvCoef;
		
		if(AmpEnvValue<AmpEnvSustainLevel)
		{
			AmpEnvValue=AmpEnvSustainLevel;
			AmpEnvStage=3;

			if(!syntp->amp_env_sustain)
			AmpEnvStage=0;
		}

		return AmpEnvValue;
	break;

	case 3:
		return AmpEnvValue;
	break;

	case 4: // Release
		AmpEnvValue-=AmpEnvCoef;

		if(AmpEnvValue<0.0f)
		{
			AmpEnvValue=0.0f;
			AmpEnvStage=0;
		}

		return AmpEnvValue;
	break;
	
	case 5: // FastRelease
		AmpEnvValue-=AmpEnvCoef;

		if(AmpEnvValue<0.0f)
		{
			AmpEnvValue=0.0f;
			AmpEnvStage=1;
			AmpEnvCoef=1.0f/(float)syntp->amp_env_attack;
		}

		return AmpEnvValue;
	break;
	
	}

	return 0;
}

inline void CSynthTrack::GetEnvVcf()
{
	switch(VcfEnvStage)
	{
	case 1: // Attack
		VcfEnvValue+=VcfEnvCoef;
		
		if(VcfEnvValue>1.0f)
		{
			VcfEnvCoef=(1.0f-VcfEnvSustainLevel)/(float)syntp->vcf_env_decay;
			VcfEnvStage=2;
		}
	break;

	case 2: // Decay
		VcfEnvValue-=VcfEnvCoef;
		
		if(VcfEnvValue<VcfEnvSustainLevel)
		{
			VcfEnvValue=VcfEnvSustainLevel;
			VcfEnvStage=3;
		}
	break;

	case 4: // Release
		VcfEnvValue-=VcfEnvCoef;

		if(VcfEnvValue<0.0f)
		{
			VcfEnvValue=0.0f;
			VcfEnvStage=0;
		}
	break;

	case 5: // FastRelease
		VcfEnvValue-=VcfEnvCoef;

		if(VcfEnvValue<0.0f)
		{
			VcfEnvValue=0.0f;
			VcfEnvStage=1;
			VcfEnvCoef=1.0f/(float)syntp->vcf_env_attack;
		}

	break;
	}
}

#endif // !defined(AFX_SYNTHTRACK_H__3D2712C1_36AD_11D4_8913_AE42C3A28249__INCLUDED_)
