/*
	GameFX (C)2005 by Jan-Marco Edelmann [voskomo], voskomo_at_yahoo_dot_de
	Programm is based on Arguru Bass. Filter seems to be Public Domain.

	This plugin is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.\n"\

	This plugin is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <project.private.hpp>
#include <cmath>

#include "voice.h"

#define FILTER_CALC_TIME 64
#define TWOPI 6.28318530717958647692528676655901f

CSynthTrack::CSynthTrack()
{
	replaycount=1;
	perf_count=1;
	perf_index=0;
	perf_index=0;
	timetocompute=FILTER_CALC_TIME;

	cur_basenote=0;
	cur_realnote=0;
	cur_volume=0;
	voicevol=1.0f;
	volmulti=0.0f;
	cur_waveform=0;
	cur_transpose=0;
	add_to_pitch=0.0f;
	cur_option=0;
	cur_command=0;
	cur_parameter=0;
	cur_speed=16;
	cur_pw=512;
	keyrelease=true;

	OSCVol=0;
	oscglide=0;

	sp_cmd=0;
	sp_val=0;

	OSCSpeed=0.0f;
	ROSCSpeed=0.0f;
	OSCPosition=0.0f;

	AmpEnvStage=0;
	AmpEnvValue=0.0f;
	VcfEnvStage=0;
	VcfEnvValue=0.0f;

	m_filter.init(44100);
}

CSynthTrack::~CSynthTrack()
{

}

void CSynthTrack::NoteOn(int note, PERFORMANCE *perf, int spd)
{
	vpar=perf;
	replaycount = 1;
	perf_count = 1;
	perf_index = vpar->StartPos;
	note+=6;
	speed=spd;
	if(spd<65) spdcoef=(float)spd*0.015625f; else spdcoef=1.0f;
	volmulti=0.0039062f*spdcoef;
	if (sp_cmd != 12) voicevol=1.0f;
	cur_basenote=note;
	keyrelease=false;
	InitEnvelopes();
}

void CSynthTrack::InitEnvelopes()
{
	// Init Amplitude Envelope
	VcfEnvMod=(float)vpar->EnvMod;
	VcfCutoff=(float)vpar->Cutoff;

	VcfResonance=(float)vpar->Resonance/256.0f;

	AmpEnvSustainLevel=(float)vpar->AEGSustain*0.0039062f;
	VcfEnvSustainLevel=(float)vpar->FEGSustain*0.0039062f;

	if(AmpEnvStage==0)
	{
		AmpEnvStage=1;
		AmpEnvCoef=1.0f/(float)vpar->AEGAttack;
	}
	else
	{
		AmpEnvStage=5;
		AmpEnvCoef=AmpEnvValue/32.0f;
		if(AmpEnvCoef<=0.0f)AmpEnvCoef=0.03125f;
	}

	// Init Filter Envelope
	if(VcfEnvStage==0)
	{
		VcfEnvStage=1;
		VcfEnvCoef=1.0f/(float)vpar->FEGAttack;
	}
	else
	{
		VcfEnvStage=5;
		VcfEnvCoef=VcfEnvValue/32.0f;
		if(VcfEnvCoef<=0.0f)VcfEnvCoef=0.03125f;
	}
}

float CSynthTrack::GetSample()
{
	float output=0;

	replaycount--;
	if (replaycount == 0){
		replaycount=vpar->ReplaySpeed;
		perf_count--;
		if (perf_count == 0){
			perf_count = cur_speed;
			if (vpar->Volume[perf_index]){
				cur_volume=vpar->Volume[perf_index];
				OSCVol=(float)cur_volume*volmulti;
			}

			if (vpar->Waveform[perf_index]) cur_waveform=vpar->Waveform[perf_index]-1;
			if (vpar->Transpose[perf_index]) {
				add_to_pitch=0.0f;
				cur_transpose=vpar->Transpose[perf_index]-1;
				cur_option=vpar->Option[perf_index];
				if (cur_option & 1) cur_realnote = cur_transpose; else cur_realnote = cur_basenote-6+cur_transpose;
				if (cur_option & 2){
					 if (keyrelease==false) InitEnvelopes();
				}
			}
			cur_command=vpar->Command[perf_index];
			cur_parameter=vpar->Parameter[perf_index];
			switch(cur_command){
				case 3: cur_realnote=(cur_realnote+cur_parameter)&127; if (cur_realnote>95) cur_realnote-=32; break;
				case 4: cur_realnote=(cur_realnote-cur_parameter)&127; if (cur_realnote>95) cur_realnote-=32; break;
				case 5: cur_pw=cur_parameter<<2; break;
				case 15: NoteOff(); break;
			}
			if (vpar->Speed[perf_index]) {
				cur_speed=vpar->Speed[perf_index];
				perf_count=cur_speed;
			}
			perf_index++;
			if ((perf_index == vpar->LoopEnd+1) || (perf_index > 15)) perf_index = vpar->LoopStart;
		}
		switch(cur_command){
			case 1: add_to_pitch+=cur_parameter*0.001f; break;
			case 2: add_to_pitch-=cur_parameter*0.001f; break;
		}
		OSCSpeed=(float)pow(2.0, (55.235+cur_realnote+add_to_pitch+vpar->Finetune*0.0039062f)/12.0)*0.03125;
		if (OSCSpeed > 370) OSCSpeed=370; //Limit C-0 to C-7 (8 octaves because sid also has 8)
		if (cur_waveform > 6) OSCSpeed*=0.25;
	}

	if(AmpEnvStage)
	{

		switch(cur_waveform)
		{
			case 5:		output=vpar->Wavetable[cur_waveform][f2i(OSCPosition)+cur_pw]*OSCVol*voicevol; break;
			case 8:		output=vpar->shortnoise*OSCVol*voicevol; break;
			default:	output=vpar->Wavetable[cur_waveform][f2i(OSCPosition)]*OSCVol*voicevol; break;
		}
		if (cur_waveform > 6) vpar->noiseused=true;
		OSCPosition+=ROSCSpeed;
		
		if(OSCPosition>=2048.0f)
		OSCPosition-=2048.0f;

		GetEnvVcf();

		if(!timetocompute--)
		{
			int realcutoff=VcfCutoff+VcfEnvMod*VcfEnvValue;

			if(realcutoff<1)realcutoff=1;
			if(realcutoff>240)realcutoff=240;
			m_filter.setfilter(0,realcutoff,vpar->Resonance); // 0 means lowpass
			timetocompute=FILTER_CALC_TIME;
		}

		return m_filter.res(output)*GetEnvAmp();	
	}
	else
	return 0;
}

float CSynthTrack::GetEnvAmp()
{
	switch(AmpEnvStage)
	{
	case 1: // Attack
		AmpEnvValue+=AmpEnvCoef;
		
		if(AmpEnvValue>1.0f)
		{
			AmpEnvCoef=(1.0f-AmpEnvSustainLevel)/(float)vpar->AEGDecay;
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

			if(!vpar->AEGSustain)
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
			AmpEnvCoef=1.0f/(float)vpar->AEGAttack;
		}

		return AmpEnvValue;
	break;
	
	}

	return 0;
}

void CSynthTrack::GetEnvVcf()
{
	switch(VcfEnvStage)
	{
	case 1: // Attack
		VcfEnvValue+=VcfEnvCoef;
		
		if(VcfEnvValue>1.0f)
		{
			VcfEnvCoef=(1.0f-VcfEnvSustainLevel)/(float)vpar->FEGDecay;
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
			VcfEnvCoef=1.0f/(float)vpar->FEGAttack;
		}

	break;
	}
}

void CSynthTrack::NoteOff()
{
	float const unde = 0.00001f;

	keyrelease=true;
	if(AmpEnvStage)
	{
	AmpEnvStage=4;
	VcfEnvStage=4;
	AmpEnvCoef=AmpEnvValue/(float)vpar->AEGRelease;
	VcfEnvCoef=VcfEnvValue/(float)vpar->FEGRelease;

	if(AmpEnvCoef<unde)AmpEnvCoef=unde;
	if(VcfEnvCoef<unde)VcfEnvCoef=unde;

	if(!vpar->AEGSustain)
	AmpEnvStage=0;
	}
}

void CSynthTrack::DoGlide()
{
	// Glide Handler
	if(ROSCSpeed<OSCSpeed)
	{
		ROSCSpeed+=oscglide;

		if(ROSCSpeed>OSCSpeed)
			ROSCSpeed=OSCSpeed;
	}
	else
	{
		ROSCSpeed-=oscglide;

		if(ROSCSpeed<OSCSpeed)
			ROSCSpeed=OSCSpeed;
	}
}

void CSynthTrack::PerformFx()
{
	// Perform tone glide
	DoGlide();
}

void CSynthTrack::InitEffect(int cmd, int val)
{
	sp_cmd=cmd;
	sp_val=val;

	// Init glide
	if(cmd==3)
	oscglide=(float)(val*val)*0.001f;
	else
	oscglide=256.0f;

	//Vol
	if(cmd==12)voicevol=(float)val/255.0f;
}
