#pragma once
#include "filter.hpp"
#include <psycle/helpers/math/rint.hpp>
#include <psycle/helpers/math/pi.hpp>
///\file SynthTrack.h
///\brief interface for the CSynthTrack class.
#define FILTER_CALC_TIME 64
#define TWOPI            2.0*math::pi

using namespace psycle::helpers;

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
	int globaldetune;
	int globalfinetune;
	int synthglide;
	int interpolate;
};

class CSynthTrack  
{
public:
	CSynthTrack();
	virtual ~CSynthTrack();

	void setSampleRate(int currentSR_, int wavetableSize_, float srCorrection_);
	void setGlobalPar(SYNPAR* globalPar);
	void InitEffect(int cmd,int val);
	void NoteOn(int note, int spd);
	void NoteOff(bool stop=false);
	void PerformFx();
	inline float GetSample();
	inline float GetSampleOsc1();
	inline float GetSampleOsc2();
	
	int AmpEnvStage;
	int NoteCutTime;

private:
	void InitLfo(int freq,int amp);
	void InitEnvelopes(bool force=false);
	void ActiveVibrato(int speed,int depth);
	void DisableVibrato();
	void DoGlide();
	void Vibrate();
	inline float GetEnvAmp();
	inline void GetEnvVcf();
	inline void ArpTick(void);
	inline void FilterTick(void);

	static const signed char ArpNote[9][16];
	filter m_filter;
	SYNPAR *syntp;
	int sampleRate;
	float srCorrection;
	//in float since it is compared with OSCPosition
	float waveTableSize;

	int sp_cmd;
	int sp_val;

	// Envelope [Amplitude]
	float AmpEnvValue;
	float AmpEnvCoef;
	float AmpEnvSustainLevel;
	float Stage5AmpVal;
	bool vibrato;
	float OSC1Vol;
	float OSC2Vol;

	// Envelope [Filter]
	int VcfEnvStage;
	float VcfEnvValue;
	float VcfEnvCoef;
	float VcfEnvSustainLevel;

	float VcfEnvMod;
	float VcfCutoff;
	float VcfResonance;

	//LFO
	float lfo_freq;
	float lfo_phase;
	short timetocompute;


	//Oscillator. Target and current speed.
	float OSC1Speed;
	float OSC2Speed;
	float ROSC1Speed;
	float ROSC2Speed;
	
	float OSC1Position;
	float OSC2Position;
	
	float oscglide;

	//Vibrato parameters and value
	float VibratoGr;
	float VibratoSpeed;
	float VibratoDepth;
	float OSCvib;

	// Arpeggiator
	int ArpMode;
	int Arp_tickcounter;
	int Arp_samplespertick;
	float Arp_basenote;
	unsigned char ArpCounter;
	//unsigned char ArpLimit;


};

inline void CSynthTrack::ArpTick()
{
	Arp_tickcounter=0;

	float note=Arp_basenote+(float)ArpNote[ArpMode-1][ArpCounter];
	OSC1Speed=(float)pow(2.0, note/12.0);

	float note2=note+
	(float)syntp->osc2finetune*0.0039062f+
	(float)syntp->osc2detune;
	OSC2Speed=(float)pow(2.0, note2/12.0);

	if(++ArpCounter>=syntp->arp_cnt)  ArpCounter=0;

	if (AmpEnvStage<4) {
		InitEnvelopes(true);
	}
}

inline void CSynthTrack::FilterTick()
{
	lfo_phase+=lfo_freq;

	if (lfo_phase>TWOPI) lfo_phase-=TWOPI;
	
	float const VcfLfoVal=sin(lfo_phase)*(float)syntp->vcf_lfo_amplitude;
	int realcutoff=VcfCutoff+VcfLfoVal+VcfEnvMod*VcfEnvValue;

	if (realcutoff<1) realcutoff=1;
	if (realcutoff>240) realcutoff=240;

	m_filter.setfilter(syntp->vcf_type%10,realcutoff,syntp->vcf_resonance);
	timetocompute=FILTER_CALC_TIME;

}

inline float CSynthTrack::GetSample()
{
	float output;

	if(AmpEnvStage)
	{
		if ((ArpMode>0) && (++Arp_tickcounter>Arp_samplespertick)) ArpTick();
	
		if ( syntp->interpolate )  // Quite Pronounced CPU usage increase...
		{
			int wtSizeInt = math::lrint<int,float>(waveTableSize);
			int iOsc=math::lrint<int,float>(OSC1Position);
			float fractpart=OSC1Position-iOsc;
			float d0;
			float d1=syntp->pWave[iOsc];
			float d2=syntp->pWave[iOsc+1];
			float d3=syntp->pWave[iOsc+2];
			if (iOsc == 0) {
				d0=syntp->pWave[wtSizeInt - 1];
			}
			else {
				d0=syntp->pWave[iOsc-1];
			}
			output=((((((((3*(d1-d2))-d0)+d3)*0.5f*fractpart)+((2*d2)+d0)-(((5*d1)+d3)*0.5f))*fractpart)+((d2-d0)*0.5f))*fractpart+d1)*OSC1Vol;
			
			iOsc=math::lrint<int,float>(OSC2Position);
			fractpart=OSC2Position-iOsc;
			d1=syntp->pWave2[iOsc];
			d2=syntp->pWave2[iOsc+1];
			d3=syntp->pWave2[iOsc+2];
			if (iOsc == 0) {
				d0=syntp->pWave2[wtSizeInt - 1];
			}
			else {
				d0=syntp->pWave2[iOsc-1];
			}
			output+=((((((((3*(d1-d2))-d0)+d3)*0.5f*fractpart)+((2*d2)+d0)-(((5*d1)+d3)*0.5f))*fractpart)+((d2-d0)*0.5f))*fractpart+d1)*OSC2Vol;

		}
		else
		{
			output=syntp->pWave[math::lrint<int,float>(OSC1Position)]*OSC1Vol+
					syntp->pWave2[math::lrint<int,float>(OSC2Position)]*OSC2Vol;
		}

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
		
		if(OSC1Position>=waveTableSize)
		{
			OSC1Position-=waveTableSize;
		
			if(syntp->osc2sync)
				OSC2Position=OSC1Position;
		}

		if(OSC2Position>=waveTableSize) OSC2Position-=waveTableSize;

		GetEnvVcf();

		if(!timetocompute--) FilterTick();
		
		if ( syntp->vcf_type > 9 ) return m_filter.res2(output)*GetEnvAmp();
		else return m_filter.res(output)*GetEnvAmp();
	}
	else return 0;
}

inline float CSynthTrack::GetSampleOsc1()
{
	float output;

	if(AmpEnvStage)
	{
		if ((ArpMode>0) && (++Arp_tickcounter>Arp_samplespertick)) ArpTick();
	
		if ( syntp->interpolate )  // Quite Pronounced CPU usage increase...
		{
			int wtSizeInt = math::lrint<int,float>(waveTableSize);
			int iOsc=math::lrint<int,float>(OSC1Position);
			float fractpart=OSC1Position-iOsc;
			float d0;
			float d1=syntp->pWave[iOsc];
			float d2=syntp->pWave[iOsc+1];
			float d3=syntp->pWave[iOsc+2];
			if (iOsc == 0) {
				d0=syntp->pWave[wtSizeInt - 1];
			}
			else {
				d0=syntp->pWave[iOsc-1];
			}
			output=((((((((3*(d1-d2))-d0)+d3)*0.5f*fractpart)+((2*d2)+d0)-(((5*d1)+d3)*0.5f))*fractpart)+((d2-d0)*0.5f))*fractpart+d1)*OSC1Vol;
		}
		else
		{
			output=syntp->pWave[math::lrint<int,float>(OSC1Position)]*OSC1Vol;
		}

		if(vibrato) OSC1Position+=ROSC1Speed+OSCvib;
		else        OSC1Position+=ROSC1Speed;
		
		if(OSC1Position>=waveTableSize)  OSC1Position-=waveTableSize;
		
		GetEnvVcf();

		if(!timetocompute--) FilterTick();

		if ( syntp->vcf_type > 9 ) return m_filter.res2(output)*GetEnvAmp();
		else return m_filter.res(output)*GetEnvAmp();
	}
	else return 0;
}
inline float CSynthTrack::GetSampleOsc2()
{
	float output;

	if(AmpEnvStage)
	{
		if ((ArpMode>0) && (++Arp_tickcounter>Arp_samplespertick)) ArpTick();
	
		if ( syntp->interpolate )  // Quite Pronounced CPU usage increase...
		{
			int wtSizeInt = math::lrint<int,float>(waveTableSize);
			int iOsc=math::lrint<int,float>(OSC2Position);
			float fractpart=OSC2Position-iOsc;
			float d0;
			float d1=syntp->pWave2[iOsc];
			float d2=syntp->pWave2[iOsc+1];
			float d3=syntp->pWave2[iOsc+2];
			if (iOsc == 0) {
				d0=syntp->pWave2[wtSizeInt - 1];
			}
			else {
				d0=syntp->pWave2[iOsc-1];
			}
			output=((((((((3*(d1-d2))-d0)+d3)*0.5f*fractpart)+((2*d2)+d0)-(((5*d1)+d3)*0.5f))*fractpart)+((d2-d0)*0.5f))*fractpart+d1)*OSC2Vol;
		}
		else
		{
			output=syntp->pWave2[math::lrint<int,float>(OSC2Position)]*OSC2Vol;
		}

		if(vibrato) OSC2Position+=ROSC2Speed+OSCvib;
		else        OSC2Position+=ROSC2Speed;
		
		if(OSC2Position>=waveTableSize) OSC2Position-=waveTableSize;
		
		GetEnvVcf();

		if(!timetocompute--) FilterTick();

		if ( syntp->vcf_type > 9 ) return m_filter.res2(output)*GetEnvAmp();
		else return m_filter.res(output)*GetEnvAmp();
	}
	else  return 0;
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
	break;

	case 2: // Decay
		AmpEnvValue-=AmpEnvCoef;
		
		if(AmpEnvValue<AmpEnvSustainLevel)
		{
			if(AmpEnvSustainLevel == 0.0f) {
				AmpEnvValue=0.0f;
				AmpEnvStage=0;
				if (oscglide == 0.0f) {
					ROSC1Speed=0.0f;
					ROSC2Speed=0.0f;
				}
			}
			else {
				AmpEnvValue=AmpEnvSustainLevel;
				AmpEnvStage=3;
			}
		}
	break;

	case 4: // Release
		AmpEnvValue-=AmpEnvCoef;

		if(AmpEnvValue<0.0f)
		{
			AmpEnvValue=0.0f;
			AmpEnvStage=0;
			if (oscglide == 0.0f) {
				ROSC1Speed=0.0f;
				ROSC2Speed=0.0f;
			}
		}
	break;
	
	case 5: // FastRelease
		AmpEnvValue-=AmpEnvCoef;
		Stage5AmpVal+=1.0f/(float)syntp->amp_env_attack;

		if(AmpEnvValue<Stage5AmpVal)
		{
			AmpEnvValue=Stage5AmpVal;
			if (oscglide == 0.0f) {
				ROSC1Speed = OSC1Speed;
				ROSC2Speed = OSC2Speed;
			}
			AmpEnvStage=1;
			AmpEnvCoef=1.0f/(float)syntp->amp_env_attack;
			VcfEnvStage=1;
			VcfEnvCoef=1.0f/(float)syntp->vcf_env_attack;
		}
	break;

	default:
		break;
	}

	return AmpEnvValue;
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

	default:
		break;
	}
}
