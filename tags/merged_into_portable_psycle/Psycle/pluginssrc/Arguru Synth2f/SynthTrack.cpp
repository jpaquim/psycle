// SynthTrack.cpp: implementation of the CSynthTrack class.
//
//////////////////////////////////////////////////////////////////////
#include <math.h>
#include "SynthTrack.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSynthTrack::CSynthTrack()
{
	output=0;
	NoteCutTime=0;
	NoteCut=false;

	timetocompute=FILTER_CALC_TIME;

	lfo_phase=0;
	synthglide = 256;

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
	vibrato=false;

	InitArpeggio();
	m_filter.init(44100);
}

CSynthTrack::~CSynthTrack()
{

}

CSynthTrack::NoteOn(int note, SYNPAR *tspar,int spd)
{
	syntp=tspar;

	synthglide = 256-syntp->synthglide;

	InitLfo(syntp->vcf_lfo_speed,syntp->vcf_lfo_amplitude);

	note+=6;

	float nnote=(float)note+
				(float)tspar->globalfinetune*0.0038962f+
				(float)tspar->globaldetune;

	OSC1Speed=(float)pow(2.0, (float)nnote/12.0);

	float note2=nnote+
				(float)tspar->osc2finetune*0.0038962f+
				(float)tspar->osc2detune;

	OSC2Speed=(float)pow(2.0, (float)note2/12.0);

	if ( AmpEnvStage == 0 ) {
		ROSC1Speed = OSC1Speed;
		ROSC2Speed = OSC2Speed;
	}

	OSC2Vol=(float)syntp->osc_mix*0.0039062f;
	OSC1Vol=1.0f-OSC2Vol;
	
	float spdcoef;
	
	if(spd<65)	spdcoef=(float)spd*0.015625f;
	else		spdcoef=1.0f;

	OSC1Vol*=(float)syntp->out_vol*0.0039062f*spdcoef;
	OSC2Vol*=(float)syntp->out_vol*0.0039062f*spdcoef;

	ArpLimit=tspar->arp_cnt;
	ArpMode=syntp->arp_mod;
	Arp_tickcounter=0;
	Arp_basenote=nnote;
	Arp_samplespertick=2646000/(syntp->arp_bpm*4);
	ArpCounter=1;

	InitEnvelopes(true);

}

CSynthTrack::InitEnvelopes(bool force)
{
	// Init Amplitude Envelope
	VcfEnvMod=(float)syntp->vcf_envmod;
	VcfCutoff=(float)syntp->vcf_cutoff;

	VcfResonance=(float)syntp->vcf_resonance/256.0f;

	AmpEnvSustainLevel=(float)syntp->amp_env_sustain*0.0039062f;
	VcfEnvSustainLevel=(float)syntp->vcf_env_sustain*0.0039062f;
	
	if(AmpEnvStage==0 )
	{
		AmpEnvStage=1;
		AmpEnvCoef=1.0f/(float)syntp->amp_env_attack;
	}
	else if ( AmpEnvStage < 4 || force)
	{
		AmpEnvStage=5;
		AmpEnvCoef=AmpEnvValue/32.0f;
	
		if(AmpEnvCoef<=0.0f) AmpEnvCoef=0.03125f;
	}

	// Init Filter Envelope
	if(VcfEnvStage==0)
	{
		VcfEnvStage=1;
		VcfEnvCoef=1.0f/(float)syntp->vcf_env_attack;
	}
	else if ( VcfEnvStage < 4 || force)
	{
		VcfEnvStage=5;
		VcfEnvCoef=VcfEnvValue/32.0f;

		if(VcfEnvCoef<=0.0f) VcfEnvCoef=0.03125f;
	}
}

CSynthTrack::NoteOff(bool fast)
{
	float const unde = 0.00001f;

	if(AmpEnvStage)
	{
		AmpEnvStage=4;
		VcfEnvStage=4;

		AmpEnvCoef=AmpEnvValue/(float)syntp->amp_env_release;
		VcfEnvCoef=VcfEnvValue/(float)syntp->vcf_env_release;

		if(AmpEnvCoef<unde)AmpEnvCoef=unde;
		if(VcfEnvCoef<unde)VcfEnvCoef=unde;

//		if(!syntp->amp_env_sustain) AmpEnvStage=0;
	}
}

CSynthTrack::Vibrate()
{
	if(vibrato)
	{
		OSCvib=(float)sin(VibratoGr)*VibratoDepth;
		VibratoGr+=VibratoSpeed;

		if (VibratoGr>6.28318530717958647692528676655901f)
			VibratoGr-=6.28318530717958647692528676655901f;
	}
}

CSynthTrack::ActiveVibrato(int speed,int depth)
{
	VibratoSpeed=(float)depth/16.0f;
	VibratoDepth=(float)speed/16.0f;
	vibrato=true;
}

CSynthTrack::DisableVibrato()
{
	vibrato=false;
}

CSynthTrack::InitArpeggio()
{
	ArpNote[0][0]	=0;
	ArpNote[0][1]	=3;
	ArpNote[0][2]	=7;
	ArpNote[0][3]	=12;
	ArpNote[0][4]	=15;
	ArpNote[0][5]	=19;
	ArpNote[0][6]	=24;
	ArpNote[0][7]	=27;
	ArpNote[0][8]	=31;
	ArpNote[0][9]	=36;
	ArpNote[0][10]	=39;
	ArpNote[0][11]	=43;
	ArpNote[0][12]	=48;
	ArpNote[0][13]	=51;
	ArpNote[0][14]	=55;
	ArpNote[0][15]	=60;

	ArpNote[1][0]	=0;
	ArpNote[1][1]	=4;
	ArpNote[1][2]	=7;
	ArpNote[1][3]	=12;
	ArpNote[1][4]	=16;
	ArpNote[1][5]	=19;
	ArpNote[1][6]	=24;
	ArpNote[1][7]	=28;
	ArpNote[1][8]	=31;
	ArpNote[1][9]	=36;
	ArpNote[1][10]	=40;
	ArpNote[1][11]	=43;
	ArpNote[1][12]	=48;
	ArpNote[1][13]	=52;
	ArpNote[1][14]	=55;
	ArpNote[1][15]	=60;

	ArpNote[2][0]	=0;
	ArpNote[2][1]	=3;
	ArpNote[2][2]	=7;
	ArpNote[2][3]	=10;
	ArpNote[2][4]	=12;
	ArpNote[2][5]	=15;
	ArpNote[2][6]	=19;
	ArpNote[2][7]	=22;
	ArpNote[2][8]	=24;
	ArpNote[2][9]	=27;
	ArpNote[2][10]	=31;
	ArpNote[2][11]	=34;
	ArpNote[2][12]	=36;
	ArpNote[2][13]	=39;
	ArpNote[2][14]	=43;
	ArpNote[2][15]	=46;

	ArpNote[3][0]	=0;
	ArpNote[3][1]	=4;
	ArpNote[3][2]	=7;
	ArpNote[3][3]	=10;
	ArpNote[3][4]	=12;
	ArpNote[3][5]	=16;
	ArpNote[3][6]	=19;
	ArpNote[3][7]	=22;
	ArpNote[3][8]	=24;
	ArpNote[3][9]	=28;
	ArpNote[3][10]	=31;
	ArpNote[3][11]	=34;
	ArpNote[3][12]	=36;
	ArpNote[3][13]	=40;
	ArpNote[3][14]	=43;
	ArpNote[3][15]	=46;

	ArpNote[4][0]	=0;
	ArpNote[4][1]	=12;
	ArpNote[4][2]	=0;
	ArpNote[4][3]	=-12;
	ArpNote[4][4]	=12;
	ArpNote[4][5]	=0;
	ArpNote[4][6]	=12;
	ArpNote[4][7]	=0;
	ArpNote[4][8]	=0;
	ArpNote[4][9]	=12;
	ArpNote[4][10]	=-12;
	ArpNote[4][11]	=0;
	ArpNote[4][12]	=12;
	ArpNote[4][13]	=0;
	ArpNote[4][14]	=12;
	ArpNote[4][15]	=-12;

	ArpNote[5][0]	=0;
	ArpNote[5][1]	=12;
	ArpNote[5][2]	=24;
	ArpNote[5][3]	=0;
	ArpNote[5][4]	=12;
	ArpNote[5][5]	=24;
	ArpNote[5][6]	=12;
	ArpNote[5][7]	=0;
	ArpNote[5][8]	=24;
	ArpNote[5][9]	=12;
	ArpNote[5][10]	=0;
	ArpNote[5][11]	=0;
	ArpNote[5][12]	=12;
	ArpNote[5][13]	=0;
	ArpNote[5][14]	=0;
	ArpNote[5][15]	=24;

	ArpNote[6][0]	=0;
	ArpNote[6][1]	=12;
	ArpNote[6][2]	=19;
	ArpNote[6][3]	=0;
	ArpNote[6][4]	=0;
	ArpNote[6][5]	=7;
	ArpNote[6][6]	=0;
	ArpNote[6][7]	=7;
	ArpNote[6][8]	=0;
	ArpNote[6][9]	=12;
	ArpNote[6][10]	=19;
	ArpNote[6][11]	=0;
	ArpNote[6][12]	=12;
	ArpNote[6][13]	=19;
	ArpNote[6][14]	=0;
	ArpNote[6][15]	=-12;

	ArpNote[7][0]	=0;
	ArpNote[7][1]	=3;
	ArpNote[7][2]	=7;
	ArpNote[7][3]	=12;
	ArpNote[7][4]	=15;
	ArpNote[7][5]	=19;
	ArpNote[7][6]	=24;
	ArpNote[7][7]	=27;
	ArpNote[7][8]	=31;
	ArpNote[7][9]	=27;
	ArpNote[7][10]	=24;
	ArpNote[7][11]	=19;
	ArpNote[7][12]	=15;
	ArpNote[7][13]	=12;
	ArpNote[7][14]	=7;
	ArpNote[7][15]	=3;

	ArpNote[8][0]	=0;
	ArpNote[8][1]	=4;
	ArpNote[8][2]	=7;
	ArpNote[8][3]	=12;
	ArpNote[8][4]	=16;
	ArpNote[8][5]	=19;
	ArpNote[8][6]	=24;
	ArpNote[8][7]	=28;
	ArpNote[8][8]	=31;
	ArpNote[8][9]	=28;
	ArpNote[8][10]	=24;
	ArpNote[8][11]	=19;
	ArpNote[8][12]	=16;
	ArpNote[8][13]	=12;
	ArpNote[8][14]	=7;
	ArpNote[8][15]	=4;

}

CSynthTrack::DoGlide()
{
	// Glide Handler
	if(ROSC1Speed<OSC1Speed)
	{
		ROSC1Speed+=oscglide;

		if(ROSC1Speed>OSC1Speed) ROSC1Speed=OSC1Speed;
	}
	else
	{
		ROSC1Speed-=oscglide;

		if(ROSC1Speed<OSC1Speed) ROSC1Speed=OSC1Speed;
	}

	if(ROSC2Speed<OSC2Speed)
	{
		ROSC2Speed+=oscglide;

		if(ROSC2Speed>OSC2Speed) ROSC2Speed=OSC2Speed;
	}
	else
	{
		ROSC2Speed-=oscglide;

		if(ROSC2Speed<OSC2Speed) ROSC2Speed=OSC2Speed;
	}
}

CSynthTrack::PerformFx()
{
	Vibrate();

	float shift;

	// Perform tone glide
	DoGlide();

	switch(sp_cmd)
	{
		/* 0x01 : Pitch Up */
		case 0x01:
			shift=(float)sp_val*0.001f;
			OSC1Speed+=shift;
			OSC2Speed+=shift;
			break;

		/* 0x02 : Pitch Down */
		case 0x02:
			shift=(float)sp_val*0.001f;
			OSC1Speed-=shift;
			OSC2Speed-=shift;
			if(OSC1Speed<0)OSC1Speed=0;
			if(OSC2Speed<0)OSC2Speed=0;
			break;

		/* 0x11 : Note Cut */
		case 0x0E:
			if(NoteCutTime<=0)
			{
				NoteCut=false;
				NoteOff();
			}
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
}

CSynthTrack::InitEffect(int cmd, int val)
{
	sp_cmd=cmd;
	sp_val=val;

	// Init glide
	if (cmd==3)				oscglide= (float)(val*val)*0.001f;
	else if(synthglide<256) oscglide= (synthglide*synthglide)*0.0000625f; //  1/(1000*16)
	else					oscglide= 256.0f;

	// Init vibrato
	if (cmd==4)	ActiveVibrato(val>>4,val&0xf);
	else		DisableVibrato();

	// Note CUT
	if (cmd==0x0E && val>0)
	{
		NoteCutTime=val*32;
		NoteCut=true;
	}
	else NoteCut=false;

}

CSynthTrack::InitLfo(int freq,int amp)
{
	lfo_freq=(float)freq*0.000005f;
}