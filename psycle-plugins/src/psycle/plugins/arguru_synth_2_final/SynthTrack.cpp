#include "SynthTrack.hpp"
#include <math.h> ///\todo should be <cmath>
///\file arguru_synth_2_final/SynthTrack.cpp
///\brief implementation of the CSynthTrack class.

const signed char CSynthTrack::ArpNote[9][16] = {
	{0,  3,  7, 12, 15, 19, 24, 27, 31, 36, 39, 43, 48, 51, 55, 60},
	{0,  4,  7, 12, 16, 19, 24, 28, 31, 36, 40, 43, 48, 52, 55, 60},
	{0,  3,  7, 10, 12, 15, 19, 22, 24, 27, 31, 34, 36, 39, 43, 46},
	{0,  4,  7, 10, 12, 16, 19, 22, 24, 28, 31, 34, 36, 40, 43, 46},
	{0, 12,  0,-12, 12,  0, 12,  0,  0, 12,-12,  0, 12,  0, 12,-12},
	{0, 12, 24,  0, 12, 24, 12,  0, 24, 12,  0,  0, 12,  0,  0, 24},
	{0, 12, 19,  0,  0,  7,  0,  7,  0, 12, 19,  0, 12, 19,  0,-12},
	{0,  3,  7, 12, 15, 19, 24, 27, 31, 27, 24, 19, 15, 12,  7,  3},
	{0,  4,  7, 12, 16, 19, 24, 28, 31, 28, 24, 19, 16, 12,  7,  4},
};


CSynthTrack::CSynthTrack()
{
	syntp=0;
	NoteCutTime=0;

	timetocompute=FILTER_CALC_TIME;

	lfo_phase=0;

	VibratoSpeed=0;
	VibratoDepth=0;
	OSC1Vol=0;
	OSC2Vol=0;
	ArpCounter=0;
	oscglide=0;
	Arp_tickcounter=0;
	Arp_samplespertick=0;
	Arp_basenote=0;
	ArpMode=0;

	sp_cmd=0;
	sp_val=0;

	OSC1Speed=0.0f;
	OSC2Speed=0.0f;
	ROSC1Speed=0.0f;
	ROSC2Speed=0.0f;
	OSC1Position=0.0f;
	OSC2Position=0.0f;
	
	VibratoGr=0;
	OSCvib=0;
	AmpEnvStage=0;
	AmpEnvValue=0.0f;
	VcfEnvStage=0;
	VcfEnvValue=0.0f;
	Stage5AmpVal=0.0f;
	vibrato=false;
	resampler.SetQuality(dsp::ResamplerQuality::R_SPLINE);
}

CSynthTrack::~CSynthTrack()
{

}
void CSynthTrack::setSampleRate(int currentSR_, int wavetableSize_, float srCorrection_) {
	m_filter.init(currentSR_);
	sampleRate = currentSR_;
	srCorrection = 44100.0f / (float)sampleRate;
	waveTableSize = wavetableSize_;
	wavetableCorrection = srCorrection_;
	if (AmpEnvStage) {
		AmpEnvStage = 0;
	}

}

void CSynthTrack::setGlobalPar(SYNPAR* globalPar) {
	syntp=globalPar;
}
void CSynthTrack::NoteOn(int note,int spd)
{
	bool forceNew=false;

	InitLfo(syntp->vcf_lfo_speed,syntp->vcf_lfo_amplitude);

	float nnote=(float)note+
		(float)syntp->globalfinetune*0.0038962f+
		(float)syntp->globaldetune;
	OSC1Speed=(float)pow(2.0, (float)nnote*wavetableCorrection/12.0);

	float note2=nnote+
		syntp->osc2finetune+
		syntp->osc2detune;
	OSC2Speed=(float)pow(2.0, (float)note2*wavetableCorrection/12.0);

	//if (oscglide == 0.0f)
	if (sp_cmd != 0x03)
	{
		if (AmpEnvStage == 0)
		{
			ROSC1Speed = OSC1Speed;
			ROSC2Speed = OSC2Speed;
			if (syntp->osc2sync) OSC2Position = 0;
			else
			{
				OSC2Position-= OSC1Position;
				if (OSC2Position<0) OSC2Position+=waveTableSize;
			}
			OSC1Position = 0;
		}
		forceNew =true;
	}
	else if ( AmpEnvStage == 0 || AmpEnvStage > 3) forceNew = true;

	OSC2Vol=(float)syntp->osc_mix*0.0039062f;
	OSC1Vol=1.0f-OSC2Vol;

	float spdcoef;
	
	if(spd<65) spdcoef=(float)spd*0.015625f;
	else       spdcoef=1.0f;

	OSC1Vol*=(float)syntp->out_vol*0.0039062f*spdcoef;
	OSC2Vol*=(float)syntp->out_vol*0.0039062f*spdcoef;

	if (sp_cmd != 0x0E) {
		NoteCutTime = 0;
	}

	//ArpLimit=tspar->arp_cnt;
	ArpMode=syntp->arp_mod;
	Arp_tickcounter=0;
	Arp_basenote=nnote;
	//divides each beat in four arp ticks.
	Arp_samplespertick=(sampleRate * 60)/(syntp->arp_bpm*4);
	ArpCounter=1;

	InitEnvelopes(forceNew);

}

void CSynthTrack::InitEnvelopes(bool forceNew)
{
	VcfEnvMod=(float)syntp->vcf_envmod;
	VcfCutoff=(float)syntp->vcf_cutoff;
	VcfResonance=(float)syntp->vcf_resonance/256.0f;

	// Init Amplitude Envelope
	AmpEnvSustainLevel=(float)syntp->amp_env_sustain*0.0039062f;
	VcfEnvSustainLevel=(float)syntp->vcf_env_sustain*0.0039062f;

	if(AmpEnvStage == 0)
	{
		AmpEnvStage=1;
		AmpEnvCoef=1.0f/syntp->amp_env_attack;
		VcfEnvStage=1;
		VcfEnvCoef=1.0f/syntp->vcf_env_attack;
	}
	else if (forceNew)
	{
		AmpEnvStage=5;
		AmpEnvCoef=AmpEnvValue/syntp->amp_env_release;
		Stage5AmpVal=0.0f;
		VcfEnvStage=4;
		VcfEnvCoef=VcfEnvValue/syntp->vcf_env_release;
	}
}

void CSynthTrack::NoteOff(bool fast)
{
	float const unde = 0.00001f;

	if(AmpEnvStage)
	{
		AmpEnvStage=4;
		VcfEnvStage=4;

		AmpEnvCoef=AmpEnvValue/syntp->amp_env_release;
		VcfEnvCoef=VcfEnvValue/syntp->vcf_env_release;

		if(AmpEnvCoef<unde)AmpEnvCoef=unde;
		if(VcfEnvCoef<unde)VcfEnvCoef=unde;
	}
}

void CSynthTrack::Vibrate()
{
	if(vibrato)
	{
		OSCvib=(float)sin(VibratoGr)*VibratoDepth;
		VibratoGr+=VibratoSpeed;

		if (VibratoGr>TWOPI)
			VibratoGr-=TWOPI;
	}
}

void CSynthTrack::ActiveVibrato(int depth,int speed)
{
	if (depth != 0 ) {
		VibratoSpeed=(float)speed*0.0625*srCorrection;// /16.0f;
		VibratoDepth=(float)depth*0.0625;// /16.0f;
	}
	if (VibratoDepth > 0.0f ) {
		vibrato=true;
	}
}

void CSynthTrack::DisableVibrato()
{
	vibrato=false;
}
void CSynthTrack::DoGlide()
{
	// Glide Handler
	if(ROSC1Speed<OSC1Speed)
	{
		ROSC1Speed+=oscglide*wavetableCorrection;

		if(ROSC1Speed>OSC1Speed) ROSC1Speed=OSC1Speed;
	}
	else if (ROSC1Speed>OSC1Speed)
	{
		ROSC1Speed-=oscglide*wavetableCorrection;

		if(ROSC1Speed<OSC1Speed) ROSC1Speed=OSC1Speed;
	}

	if(ROSC2Speed<OSC2Speed)
	{
		ROSC2Speed+=oscglide*wavetableCorrection;

		if(ROSC2Speed>OSC2Speed) ROSC2Speed=OSC2Speed;
	}
	else if(ROSC2Speed>OSC2Speed)
	{
		ROSC2Speed-=oscglide*wavetableCorrection;

		if(ROSC2Speed<OSC2Speed) ROSC2Speed=OSC2Speed;
	}
}

void CSynthTrack::PerformFx()
{
	/* 0x0E : NoteCut Time */
	if(NoteCutTime<0)
	{
		NoteCutTime=0;
		NoteOff();
	}

	Vibrate();

	float shift;

	switch(sp_cmd)
	{
		/* 0x01 : Pitch Up */
		case 0x01:
			shift=(float)sp_val*0.001f*wavetableCorrection;
			ROSC1Speed+=shift;
			ROSC2Speed+=shift;
			break;

		/* 0x02 : Pitch Down */
		case 0x02:
			shift=(float)sp_val*0.001f*wavetableCorrection;
			ROSC1Speed-=shift;
			ROSC2Speed-=shift;
			if(ROSC1Speed<0.0f)ROSC1Speed=0.0f;
			if(ROSC2Speed<0.0f)ROSC2Speed=0.0f;
			break;

		/* 0x11 : CutOff Up */
		case 0x11:
			VcfCutoff+=sp_val;
			if(VcfCutoff>127)VcfCutoff=127;
			break;

		/* 0x12 : CutOff Down */
		case 0x12:
			VcfCutoff-=sp_val;
			if(VcfCutoff<0)VcfCutoff=0;
			break;
	}
	// Perform tone glide
	DoGlide();
}

void CSynthTrack::InitEffect(int cmd, int val)
{
	sp_cmd=cmd;
	sp_val=val;

	// Init glide
	if (cmd==3) { if ( val != 0 ) oscglide= (float)val*0.001f; }
	else 
	{
		const float synthglide = 256-syntp->synthglide;
		if (synthglide < 256.0f) oscglide = (synthglide*synthglide)*0.0000625f;
		else oscglide= 0.0f;
	}

	// Init vibrato
	if (cmd==4) ActiveVibrato(val>>4,val&0xf);
	else        DisableVibrato();

	// Note CUT
	if (cmd==0x0E && val>0)
	{
		NoteCutTime=val*sampleRate/2000;
	}

}

void CSynthTrack::InitLfo(int freq,int amp)
{
	lfo_freq=(float)freq*0.000005f*wavetableCorrection;
}
