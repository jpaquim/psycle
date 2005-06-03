/*		Blitz (C)2005 by Jan-Marco Edelmann [voskomo], voskomo_at_voskomo_dot_com
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
#include "voice.h"
#include <cmath>
#define FILTER_CALC_TIME	64
#define TWOPI				6.28318530717958647692528676655901f

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSynthTrack::CSynthTrack()
{
	updateCount=1;
	timetocompute=FILTER_CALC_TIME;

	masterVolume=0;
	osc1Vol=0;
	osc2Vol=0;
	osc3Vol=0;
	osc4Vol=0;
	DCOglide=0.0f;
	semiglide=0.0f;
	DefGlide=256.0f;
	semitone=0.0f;
	rsemitone=0.0f;

	sp_cmd=0;
	sp_val=0;

	DCO1Pitch=0.0f;
	RDCO1Pitch=0.0f;
	DCO1Position=0.0f;
	DCO1Last=0.0f;

	DCO2Pitch=0.0f;
	RDCO2Pitch=0.0f;
	DCO2Position=0.0f;
	DCO2Last=0.0f;

	DCO3Pitch=0.0f;
	RDCO3Pitch=0.0f;
	DCO3Position=0.0f;
	DCO3Last=0.0f;

	DCO4Pitch=0.0f;
	RDCO4Pitch=0.0f;
	DCO4Position=0.0f;
	DCO4Last=0.0f;

	modEnvStage=0;
	modEnvValue=0.0f;
	ampEnvStage=0;
	ampEnvValue=0.0f;
	fltEnvStage=0;
	fltEnvValue=0.0f;
	pwmcount=0;
	fxcount=0;
	m_filter.init(44100);
	voiceVol=1.0f;
	rcVolCutoff=0.2f;
	rcCutCutoff=0.2f;
	rcVol=0.0f;
	rcCut=0.0f;

	arpInput[0]=0;

	nextNote=1;
	nextSpd=0;
}

CSynthTrack::~CSynthTrack(){
}

void CSynthTrack::InitVoice(VOICEPAR *voicePar){
	vpar=voicePar;
}

void CSynthTrack::ResetSym(){
	synbase[0]=vpar->initposition[0];
	synfx[0].reset();
	synbase[1]=vpar->initposition[1];
	synfx[1].reset();
	synbase[2]=vpar->initposition[2];
	synfx[2].reset();
	synbase[3]=vpar->initposition[3];
	synfx[3].reset();
	pwmcount=0;
}

void CSynthTrack::NoteTie(int note){
	note+=6;
	basenote=note+1.235f;
	rsemitone=0.0f;
	semiglide=0.0f;
	Bend=0;
}

void CSynthTrack::NoteOn(int note, VOICEPAR *voicePar, int spd){
	vpar=voicePar;
	nextNote=note;
	nextSpd=spd;
	ampEnvSustainLevel=(float)vpar->ampS*0.0039062f;
	if(ampEnvStage==0){
		RealNoteOn();
		ampEnvStage=1;
		ampEnvCoef=(1.0f/(float)vpar->ampA)*speedup;
	}else{
		ampEnvStage=5;
		ampEnvCoef=ampEnvValue/128.0f;
		if(ampEnvCoef<=0.0f) ampEnvCoef=0.03125f;
	}
	fltEnvSustainLevel=(float)vpar->fltS*0.0039062f;
	if(fltEnvStage==0){
		fltEnvStage=1;
		fltEnvCoef=(1.0f/(float)vpar->fltA)*speedup2;
	}else{
		fltEnvStage=5;
		fltEnvCoef=fltEnvValue/128.0f;
		if(fltEnvCoef<=0.0f)fltEnvCoef=0.03125f;
	}
}

void CSynthTrack::RealNoteOn(){
	updateCount=1;
	int note=nextNote;
	int spd=nextSpd;
	vpar->stereoPos=1-vpar->stereoPos;
	currentStereoPos=vpar->stereoPos;
	
	fltResonance=(float)vpar->fltResonance/256.0f;
	modEnvStage=1;
	modEnvCoef=(1.0f/(float)vpar->modA);
	
	ResetSym();
	lfoViberSample=0;
	lfoViber.setLevel(vpar->lfoDepth);
	lfoViber.setSpeed(vpar->lfoSpeed);
	lfoViber.setDelay(vpar->lfoDelay);
	lfoViber.reset();

	softenHighNotes=1.0f - ((float)pow(2.0,note/12.0)*vpar->ampTrack*0.015625);
	if (softenHighNotes < 0.0f) softenHighNotes = 0.0f;

	fxcount=0;
	if (vpar->globalGlide == 0) { DefGlide=256.0f; }
	else DefGlide=float((vpar->globalGlide*vpar->globalGlide)*0.0000625f);
	note+=6;
	basenote=note+1.235f;
	rsemitone=0.0f;
	semiglide=0.0f;
	Bend=0;

	modEnvValue=0.0f;
	modEnvLast=0.0f;
	stopRetrig=false;

	arpShuffle=0;
	arpCount=0;
	arpLen=2;
	arpIndex=0;
	curArp=0;
	arpInput[1]=0;arpInput[2]=0;arpInput[3]=0;
	arpList[0]=0;arpList[1]=0;arpList[2]=0;
	arpList[3]=0;arpList[4]=0;arpList[5]=0;arpList[6]=0;arpList[7]=0;arpList[8]=0;arpList[9]=0;arpList[10]=0;arpList[11]=0;arpList[12]=0;
	arpList[13]=0;arpList[14]=0;arpList[15]=0;arpList[16]=0;arpList[17]=0;arpList[18]=0;arpList[19]=0;arpList[20]=0;arpList[21]=0;
	oscArpTranspose[0]=0;
	oscArpTranspose[1]=0;
	oscArpTranspose[2]=0;
	oscArpTranspose[3]=0;
	float spdcoef;

	if(spd<65) spdcoef=(float)spd*0.015625f;
	else spdcoef=1.0f;

	volMulti = 0.0039062f*spdcoef;

	synposLast[0]=9999;
	synposLast[1]=9999;
	synposLast[2]=9999;
	synposLast[3]=9999;
	synbase[0]=vpar->initposition[0];
	synbase[1]=vpar->initposition[1];
	synbase[2]=vpar->initposition[2];
	synbase[3]=vpar->initposition[3];

	speedup=((float)vpar->ampScaling*basenote*0.0015f)+1.0f;
	speedup2=((float)vpar->fltScaling*basenote*0.0015f)+1.0f;
	if (speedup<1.0f) speedup=1.0f;
	if (speedup2<1.0f) speedup2=1.0f;
}

void CSynthTrack::UpdateTuning() {
	tuningChange=false;
	float modEnv = modEnvValue*vpar->modEnvAmount;
	float vibadd;
	if (vpar->lfoDelay == 0) vibadd=0.125f*(float)vpar->syncvibe;
	else vibadd = 0.125f*(float)lfoViberSample;
	for (int c = 0; c < 4; c++){
		switch (vpar->oscOptions[c]){
		case 0: oscArpTranspose[c] = curArp; break; // ---
		case 1: oscArpTranspose[c] = curArp; break; // sync
		case 2: oscArpTranspose[c] = 0; break;      // arpless sync
		case 3: oscArpTranspose[c] = 0; break;      // arp note 1
		case 4: oscArpTranspose[c] = arpInput[1]; break; // 2
		case 5: oscArpTranspose[c] = arpInput[2]; break; // 3
		case 6: oscArpTranspose[c] = arpInput[3]; break; // 4
		}
	}
	switch (vpar->lfoDestination) {
	case 0:	//all osc
		DCO1Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[0]+vpar->globalCourse+vpar->oscCourse[0]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[0])*0.0039062f)))/12.0);
		DCO2Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[1]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO3Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[2]+vpar->globalCourse+vpar->oscCourse[2]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[2])*0.0039062f)))/12.0);
		DCO4Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[3]+vpar->globalCourse+vpar->oscCourse[3]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[3])*0.0039062f)))/12.0);
		break;
	case 1: // osc 2+3+4
		DCO1Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[0]+vpar->globalCourse+vpar->oscCourse[0]+(((modEnv+vpar->globalFine+vpar->oscFine[0])*0.0039062f)))/12.0);
		DCO2Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[1]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO3Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[2]+vpar->globalCourse+vpar->oscCourse[2]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[2])*0.0039062f)))/12.0);
		DCO4Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[3]+vpar->globalCourse+vpar->oscCourse[3]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[3])*0.0039062f)))/12.0);
		break;
	case 2: // osc 2+3
		DCO1Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[0]+vpar->globalCourse+vpar->oscCourse[0]+(((modEnv+vpar->globalFine+vpar->oscFine[0])*0.0039062f)))/12.0);
		DCO2Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[1]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO3Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[2]+vpar->globalCourse+vpar->oscCourse[2]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[2])*0.0039062f)))/12.0);
		DCO4Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[3]+vpar->globalCourse+vpar->oscCourse[3]+(((modEnv+vpar->globalFine+vpar->oscFine[3])*0.0039062f)))/12.0);
		break;
	case 3: // osc 2+4
		DCO1Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[0]+vpar->globalCourse+vpar->oscCourse[0]+(((modEnv+vpar->globalFine+vpar->oscFine[0])*0.0039062f)))/12.0);
		DCO2Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[1]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO3Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[2]+vpar->globalCourse+vpar->oscCourse[2]+(((modEnv+vpar->globalFine+vpar->oscFine[2])*0.0039062f)))/12.0);
		DCO4Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[3]+vpar->globalCourse+vpar->oscCourse[3]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[3])*0.0039062f)))/12.0);
		break;
	case 4: // osc 3+4
		DCO1Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[0]+vpar->globalCourse+vpar->oscCourse[0]+(((modEnv+vpar->globalFine+vpar->oscFine[0])*0.0039062f)))/12.0);
		DCO2Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[1]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO3Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[2]+vpar->globalCourse+vpar->oscCourse[2]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[2])*0.0039062f)))/12.0);
		DCO4Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[3]+vpar->globalCourse+vpar->oscCourse[3]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[3])*0.0039062f)))/12.0);
		break;
	case 5: // osc 2
		DCO1Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[0]+vpar->globalCourse+vpar->oscCourse[0]+(((modEnv+vpar->globalFine+vpar->oscFine[0])*0.0039062f)))/12.0);
		DCO2Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[1]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO3Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[2]+vpar->globalCourse+vpar->oscCourse[2]+(((modEnv+vpar->globalFine+vpar->oscFine[2])*0.0039062f)))/12.0);
		DCO4Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[3]+vpar->globalCourse+vpar->oscCourse[3]+(((modEnv+vpar->globalFine+vpar->oscFine[3])*0.0039062f)))/12.0);
		break;
	case 6: // osc 3
		DCO1Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[0]+vpar->globalCourse+vpar->oscCourse[0]+(((modEnv+vpar->globalFine+vpar->oscFine[0])*0.0039062f)))/12.0);
		DCO2Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[1]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO3Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[2]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO4Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[3]+vpar->globalCourse+vpar->oscCourse[3]+(((modEnv+vpar->globalFine+vpar->oscFine[3])*0.0039062f)))/12.0);
		break;
	case 7: // osc 4
		DCO1Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[0]+vpar->globalCourse+vpar->oscCourse[0]+(((modEnv+vpar->globalFine+vpar->oscFine[0])*0.0039062f)))/12.0);
		DCO2Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[1]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		DCO3Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[2]+vpar->globalCourse+vpar->oscCourse[2]+(((modEnv+vpar->globalFine+vpar->oscFine[2])*0.0039062f)))/12.0);
		DCO4Pitch=(float)pow(2.0,(Bend+rbasenote+rsemitone+oscArpTranspose[3]+vpar->globalCourse+vpar->oscCourse[1]+(((modEnv+vibadd+vpar->globalFine+vpar->oscFine[1])*0.0039062f)))/12.0);
		break;
	}
	RDCO1Pitch=freqChange(DCO1Pitch);
	RDCO2Pitch=freqChange(DCO2Pitch);
	RDCO3Pitch=freqChange(DCO3Pitch);
	RDCO4Pitch=freqChange(DCO4Pitch);
}

void CSynthTrack::InitEnvelopes()
{
	ampEnvSustainLevel=(float)vpar->ampS*0.0039062f;
	if(ampEnvStage==0){
		ampEnvStage=1;
		ampEnvCoef=(1.0f/(float)vpar->ampA)*speedup;
	}else{
		ampEnvStage=5;
		ampEnvCoef=ampEnvValue/512.0f;
		if(ampEnvCoef<=0.0f) ampEnvCoef=0.03125f;
	}
	fltEnvSustainLevel=(float)vpar->fltS*0.0039062f;
	if(fltEnvStage==0){
		fltEnvStage=1;
		fltEnvCoef=(1.0f/(float)vpar->fltA)*speedup2;
	}else{
		fltEnvStage=5;
		fltEnvCoef=fltEnvValue/512.0f;
		if(fltEnvCoef<=0.0f)fltEnvCoef=0.03125f;
	}

}

void CSynthTrack::GetSample(float* slr)
{
	float output=0.0f;
	float output1=0.0f;
	float output2=0.0f;
	float output3=0.0f;
	float output4=0.0f;
	//float outputRM1=0.0f;
	//float outputRM2=0.0f;

	updateCount--;
	if (updateCount < 1){
		updateCount = 128;
		masterVolume=(float)vpar->globalVolume*volMulti*0.005f;
		osc1Vol=vpar->oscVolume[0]*masterVolume;
		osc2Vol=vpar->oscVolume[1]*masterVolume;
		osc3Vol=vpar->oscVolume[2]*masterVolume;
		osc4Vol=vpar->oscVolume[3]*masterVolume;
		rm1Vol=vpar->rm1*masterVolume*0.0001f;
		rm2Vol=vpar->rm2*masterVolume*0.0001f;
		arpSpeed[0]=vpar->arpSpeed;
		if (vpar->arpShuffle) arpSpeed[1]=vpar->arpShuffle; else arpSpeed[1]=arpSpeed[0];
	}


	if(ampEnvStage)	{
		pwmcount--;
		int c;
		if (pwmcount < 0){
			pwmcount = 500;

			synfx[0].setRange(vpar->oscSymLfoRange[0]);
			synfx[0].setSpeed(vpar->oscSymLfoSpeed[0]);
			synfx[1].setRange(vpar->oscSymLfoRange[1]);
			synfx[1].setSpeed(vpar->oscSymLfoSpeed[1]);
			synfx[2].setRange(vpar->oscSymLfoRange[2]);
			synfx[2].setSpeed(vpar->oscSymLfoSpeed[2]);
			synfx[3].setRange(vpar->oscSymLfoRange[3]);
			synfx[3].setSpeed(vpar->oscSymLfoSpeed[3]);

			float float1, float2, float3, float4, float5 = 0;
			float size1, size2, step1, step2, phase;
			int work1, work2 = 0;
			for (int i=0; i<4; i++) {
				int pos = (synbase[i]+synfx[i].getPosition()+vpar->oscFuncSym[i])&2047;
				if (pos != synposLast[i]) {
					synposLast[i]=pos;
					switch (vpar->oscFuncType[i]) {
					case 0: break; // no function
					case 1: // stretch&squash
						size1 = float(pos); size2 = float(2047-pos);
						if (size1!=0) { step1 = 1024.0f/(size1+1.0f); } else { step1 = 1024.0f; }
						if (size2!=0) { step2 = 1024.0f/(size2+1.0f); } else { step2 = 0.0f; }
						phase = 0.0f; if (size1 == 0.0f) phase+=1024.0f;
						for(c=0;c<2050;c++){
							WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][int(phase)];
							if (phase < 1024.0f) phase+=step1; else phase+=step2;
						}
						break;
					case 2: // stretch&squash 2
						size1 = float(pos); size2 = float(2047-pos);
						if (size1!=0) { step1 = 1024.0f/(size1+1.0f); } else { step1 = 1024.0f; }
						if (size2!=0) { step2 = 1024.0f/(size2+1.0f); } else { step2 = 0.0f; }
						phase = 0.0f; if (size1 == 0.0f) phase+=1024.0f;
						for(c=0;c<2050;c++){
							WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][int(phase+phase)];
							if (phase < 1024.0f) phase+=step1; else phase+=step2;
						}
						break;
					case 3: // pw am
						for(c=0;c<2048;c++){
							if (c < pos) WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][c];
							else WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][c]>>1;
						}
						break;
					case 4: // squash&squash
						float1 = 0;
						float2 = 0;
						float4 = (2047-pos)/2.0f;   // 1st halve size in byte
						float5 = 1024-float4;    // 2nd halve size in byte
						if (float4!=0) { float1 = 1024/float4; // 1st halve step
						} else { float1 = 1024; }
						if (float5!=0) { float2 = 1024/float5; // 2nd halve step
						} else { float2 = 0; }
						float3 = 0;			  //source phase
						for(c=0;c<2100;c++){
							if (c<1024) {
								if (float3<1024) {
										if (float4) {
											WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][(int)float3];
											float3+=float1;
										} else {
											WaveBuffer[i][c]=0;
											float3=1024;
										}
								} else {
									if (float5) {
										WaveBuffer[i][c]=0;
										float3=1024;
									}
								}
							} else {
								if (float3<2048) {
									if (float4 != 0) {
										WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][(int)float3];
										float3+=float1;
									} else WaveBuffer[i][c]=0;
								} else {
									if (float5 != 0) WaveBuffer[i][c]=0;
								}
							}
						}
						break;
					case 5: // Muted Synce
						float1 = 0;
						float2 = float(pos*6)/2047+1;
						for(c=0;c<2048;c++){
							if (float1<2047){
								WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][(int)float1];
								float1+=float2;
								if (float1 > 2047) float1=2048;
							} else WaveBuffer[i][c]=0;
						}
						break;
					case 6: // Syncfake
						float1 = 0;
						float2 = float(pos*6)/2047+1;
						for(c=0;c<2048;c++){
							WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][(int)float1];
							float1+=float2;
							if (float1 > 2047) float1-=2048;
						}
						break;
					case 7: // Restart
						work1 = 0;
						work2 = 2047-pos;
						for(c=0;c<2048;c++){
							WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][work1];
							work1++; if (work1 > work2) work1 = 0;
						}
						break;
					case 8: // Negator
						for(c=0;c<2048;c++){
							if (c<pos)WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][c];
							else WaveBuffer[i][c]=0-vpar->WaveTable[vpar->oscWaveform[i]][c];
						}
						break;
					case 9: // Double Negator
						if (pos > 1023) pos = 2047-pos;
						for(c=0;c<1024;c++){
							if (c<pos){
								WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][c];
								WaveBuffer[i][2047-c]=vpar->WaveTable[vpar->oscWaveform[i]][2047-c];
							} else {
								WaveBuffer[i][c]=0-vpar->WaveTable[vpar->oscWaveform[i]][c];
								WaveBuffer[i][2047-c]=0-vpar->WaveTable[vpar->oscWaveform[i]][2047-c];
							}
						}
						break;
					case 10: // Rect Negator
						for(c=0;c<2048;c++){
							if (((pos+c)&2047)<1024)WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][c];
							else WaveBuffer[i][c]=0-vpar->WaveTable[vpar->oscWaveform[i]][c];
						}
						break;
					case 11: // Octaving
						float1 = (2047-float(pos))/2047;
						float2 = float(pos)/2047;
						for (c=0;c<2048;c++){
							WaveBuffer[i][c]=(signed short)((vpar->WaveTable[vpar->oscWaveform[i]][c]*float1)+(vpar->WaveTable[vpar->oscWaveform[i]][(c+c)&2047]*float2));
						}
						break;
					case 12: // FSK
						work1=0;
						work2=2047-pos;
						for (c=0;c<2048;c++){
							if (c<work2){
								WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][c];
								work1=c+1;
							}
							else {
								WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][work1];
								work1+=2; if (work1>2047) work1 -= 2048;
							}
						}
						break;
					case 13: // Mixer
						for (c=0;c<2048;c++){
							WaveBuffer[i][c]=(vpar->WaveTable[vpar->oscWaveform[i]][c]>>1)+(vpar->WaveTable[vpar->oscWaveform[i]][(c+pos)&2047]>>1);
						}
						break;
					case 14: // Dual Mixer
						for (c=0;c<2048;c++){
							WaveBuffer[i][c]=(signed short)((vpar->WaveTable[vpar->oscWaveform[i]][c]*0.3333333)+(vpar->WaveTable[vpar->oscWaveform[i]][(c+pos)&2047]*0.3333333)+(vpar->WaveTable[vpar->oscWaveform[i]][(c-pos)&2047]*0.3333333));
						}
						break;
					case 15: // Fbk.Mixer
						for (c=0;c<2048;c++){
							WaveBuffer[i][c]=(WaveBuffer[i][c]>>1)+(vpar->WaveTable[vpar->oscWaveform[i]][(c+pos)&2047]>>1);
						}
						break;
					case 16: // Inv.Mixer
						for (c=0;c<2048;c++){
							WaveBuffer[i][c]=(vpar->WaveTable[vpar->oscWaveform[i]][c]>>1)-(vpar->WaveTable[vpar->oscWaveform[i]][(c+pos)&2047]>>1);
						}
						break;
					case 17: // TriMix
						float1 = (2047-float(pos))/2047;
						float2 = float(pos)/2047;
						work1 = 0;
						for(c=0;c<2048;c++){
							WaveBuffer[i][c]=(signed short)((vpar->WaveTable[vpar->oscWaveform[i]][c]*float1)+(vpar->WaveTable[WAVE_TRIANGLE][c]*float2));
						}
						break;
					case 18: // SawMix
						float1 = (2047-float(pos))/2047;
						float2 = float(pos)/2047;
						work1 = 0;
						for(c=0;c<2048;c++){
							WaveBuffer[i][c]=(signed short)((vpar->WaveTable[vpar->oscWaveform[i]][c]*float1)+((work1-16384)*float2));
							work1+=16;
						}
						break;
					case 19: // SqrMix
						float1 = (2047-float(pos))/2047;
						float2 = float(pos)/2047;
						work1 = 0;
						for(c=0;c<2048;c++){
							WaveBuffer[i][c]=(signed short)((vpar->WaveTable[vpar->oscWaveform[i]][c]*float1)+(vpar->WaveTable[WAVE_SQUARE][c]*float2));
							work1+=16;
						}
						break;
					case 20: // Tremelo
						float1 = (2047-float(pos))/2047;
						for(c=0;c<2048;c++){
							WaveBuffer[i][c]=(signed short)((float)vpar->WaveTable[vpar->oscWaveform[i]][c]*float1);
						}
						break;
					case 21: // Phase Mod
						float2 = float(pos)/2047;
						work1 = 0;
						for(c=0;c<2048;c++){
							WaveBuffer[i][c]=vpar->WaveTable[vpar->oscWaveform[i]][(short int)(c+(vpar->WaveTable[vpar->oscWaveform[i]][c]*float2*0.5))&2047];
							work1+=16;
						}
						break;
					}
				}
			}
			synfx[0].next();
			synfx[1].next();
			synfx[2].next();
			synfx[3].next();
		}

		//Arpeggio
		fxcount--;
		if (fxcount < 0){
			fxcount = 50;
			GetEnvMod();
			if (modEnvValue != modEnvLast) tuningChange=true;
			if (vpar->lfoDelay){
				lfoViber.next();
				lfoViberLast=lfoViberSample;
				lfoViberSample=(float)lfoViber.getPosition();
				if (lfoViberSample != lfoViberSample) tuningChange=true;
			}
			arpCount--;
			if (arpCount < 0){
				arpCount = arpSpeed[arpShuffle];
				if ((stopRetrig == false)&((vpar->arpRetrig == 1)||((vpar->arpRetrig == 2)&(arpShuffle == 0)))) InitEnvelopes();
				arpShuffle=1-arpShuffle;
				arpIndex++;
				if (arpIndex >= arpLen) arpIndex = 0;
				curArp=arpList[arpIndex];
				tuningChange=true;
			}
		}

		if (tuningChange) UpdateTuning();

		if ( vpar->oscVolume[0] || vpar->rm1 || vpar->oscOptions[1]==1 || vpar->oscOptions[1]==2 ){
			if (vpar->oscFuncType[0]){
				for (c=0; c<4; c++){
					output1 += WaveBuffer[0][f2i(DCO1Position+DCO1Last)];
					DCO1Last=(float)output1*vpar->oscFeedback[0];
					DCO1Position+=RDCO1Pitch;
					if(DCO1Position>=2048.0f){
						DCO1Position-=2048.0f;
						if ((vpar->oscOptions[1] == 1) || (vpar->oscOptions[1] == 2)){
							DCO2Position=DCO1Position;
							if ((vpar->oscOptions[2] == 1) || (vpar->oscOptions[2] == 2)){
								DCO3Position=DCO1Position;
								if ((vpar->oscOptions[3] == 1) || (vpar->oscOptions[3] == 2)) DCO4Position=DCO1Position;
							}
						}			
					}
				}
			} else {
				for (c=0; c<4; c++){
					output1 += vpar->WaveTable[vpar->oscWaveform[0]][f2i(DCO1Position+DCO1Last)];
					DCO1Last=(float)output1*vpar->oscFeedback[0];
					DCO1Position+=RDCO1Pitch;
					if(DCO1Position>=2048.0f){
						DCO1Position-=2048.0f;
						if ((vpar->oscOptions[1] == 1) || (vpar->oscOptions[1] == 2)){
							DCO2Position=DCO1Position;
							if ((vpar->oscOptions[2] == 1) || (vpar->oscOptions[2] == 2)){
								DCO3Position=DCO1Position;
								if ((vpar->oscOptions[3] == 1) || (vpar->oscOptions[3] == 2)) DCO4Position=DCO1Position;
							}
						}
					}
				}
			}
			output1 *= 0.25f;
		}

		if ( vpar->oscVolume[1] || vpar->rm1 || vpar->oscOptions[2]==1 || vpar->oscOptions[2]==2){
			if (vpar->oscFuncType[1]){
				for (c=0; c<4; c++){
					output2 += WaveBuffer[1][f2i(DCO2Position+DCO2Last)];
					DCO2Last=(float)output2*vpar->oscFeedback[1];
					DCO2Position+=RDCO2Pitch;
					if(DCO2Position>=2048.0f){
						DCO2Position-=2048.0f;
						if ((vpar->oscOptions[2] == 1) || (vpar->oscOptions[2] == 2)){
							DCO3Position=DCO2Position;
							if ((vpar->oscOptions[3] == 1) || (vpar->oscOptions[3] == 2)){
								DCO4Position=DCO2Position;
								if ((vpar->oscOptions[0] == 1) || (vpar->oscOptions[0] == 2)) DCO1Position=DCO2Position;
							}
						}
					}
				}
			} else {
				for (c=0; c<4; c++){
					output2 += vpar->WaveTable[vpar->oscWaveform[1]][f2i(DCO2Position+DCO2Last)];
					DCO2Last=(float)output2*vpar->oscFeedback[1];
					DCO2Position+=RDCO2Pitch;
					if(DCO2Position>=2048.0f){
						DCO2Position-=2048.0f;
						if ((vpar->oscOptions[2] == 1) || (vpar->oscOptions[2] == 2)){
							DCO3Position=DCO2Position;
							if ((vpar->oscOptions[3] == 1) || (vpar->oscOptions[3] == 2)){
								DCO4Position=DCO2Position;
								if ((vpar->oscOptions[0] == 1) || (vpar->oscOptions[0] == 2)) DCO1Position=DCO2Position;
							}
						}
					}
				}
			}
			output2 *= 0.25f;
		}

		if ( vpar->oscVolume[2] || vpar->rm2 || vpar->oscOptions[3]==1 || vpar->oscOptions[3]==2 ){
			if (vpar->oscFuncType[2]){
				for (c=0; c<4; c++){
					output3 += WaveBuffer[2][f2i(DCO3Position+DCO3Last)];
					DCO3Last=(float)output3*vpar->oscFeedback[2];
					DCO3Position+=RDCO3Pitch;
					if(DCO3Position>=2048.0f){
						DCO3Position-=2048.0f;
						if ((vpar->oscOptions[3] == 1) || (vpar->oscOptions[3] == 2)){
							DCO4Position=DCO3Position;
							if ((vpar->oscOptions[0] == 1) || (vpar->oscOptions[0] == 2)){
								DCO1Position=DCO3Position;
								if ((vpar->oscOptions[1] == 1) || (vpar->oscOptions[1] == 2)) DCO2Position=DCO3Position;
							}
						}
					}
				}
			} else {
				for (c=0; c<4; c++){
					output3 += vpar->WaveTable[vpar->oscWaveform[2]][f2i(DCO3Position+DCO3Last)];	
					DCO3Last=(float)output3*vpar->oscFeedback[2];
					DCO3Position+=RDCO3Pitch;
					if(DCO3Position>=2048.0f){
						DCO3Position-=2048.0f;
						if ((vpar->oscOptions[3] == 1) || (vpar->oscOptions[3] == 2)){
							DCO4Position=DCO3Position;
							if ((vpar->oscOptions[0] == 1) || (vpar->oscOptions[0] == 2)){
								DCO1Position=DCO3Position;
								if ((vpar->oscOptions[1] == 1) || (vpar->oscOptions[1] == 2)) DCO2Position=DCO3Position;
							}
						}
					}
				}
			}
			output3 *= 0.25f;
		}

		if ( vpar->oscVolume[3] || vpar->rm2 || vpar->oscOptions[0]==1 || vpar->oscOptions[0]==2 ){
			if (vpar->oscFuncType[3]){
				for (c=0; c<4; c++){
					output4 += WaveBuffer[3][f2i(DCO4Position+DCO4Last)];
					DCO4Last=(float)output4*vpar->oscFeedback[3];
					DCO4Position+=RDCO4Pitch;
					if(DCO4Position>=2048.0f){
						DCO4Position-=2048.0f;
						if ((vpar->oscOptions[0] == 1) || (vpar->oscOptions[0] == 2)){
							DCO1Position=DCO4Position;
							if ((vpar->oscOptions[1] == 1) || (vpar->oscOptions[1] == 2)){
								DCO2Position=DCO4Position;
								if ((vpar->oscOptions[2] == 1) || (vpar->oscOptions[2] == 2)) DCO3Position=DCO4Position;
							}
						}
					}
				}
			} else {
				for (c=0; c<4; c++){
					output4 += vpar->WaveTable[vpar->oscWaveform[3]][f2i(DCO4Position+DCO4Last)];
					DCO4Last=(float)output4*vpar->oscFeedback[3];
					DCO4Position+=RDCO4Pitch;
					if(DCO4Position>=2048.0f){
						DCO4Position-=2048.0f;
						if ((vpar->oscOptions[0] == 1) || (vpar->oscOptions[0] == 2)){
							DCO1Position=DCO4Position;
							if ((vpar->oscOptions[1] == 1) || (vpar->oscOptions[1] == 2)){
								DCO2Position=DCO4Position;
								if ((vpar->oscOptions[2] == 1) || (vpar->oscOptions[2] == 2)) DCO3Position=DCO4Position;
							}
						}
					}
				}
			}
			output4 *= 0.25f;
		}

		output=(output1*osc1Vol)+(output2*osc2Vol)+(output3*osc3Vol)+(output4*osc4Vol)+(output1*output2*rm1Vol)+(output3*output4*rm2Vol);	

		GetEnvFlt();
		if (vpar->fltType){
			if(!timetocompute--) {
				int realcutoff=int(vpar->fltCutoff+(vpar->filtvibe*0.005)+(rbasenote*0.1*vpar->fltTrack)+(fltEnvValue*vpar->fltEnvAmount*((1-vpar->fltVelocity)+(voiceVol*vpar->fltVelocity))));
				if(realcutoff<1)realcutoff=1;
				if(realcutoff>250)realcutoff=250;
				rcCut+=(realcutoff-50-rcCut)*rcCutCutoff;
				m_filter.setfilter(vpar->fltType, rcCut, vpar->fltResonance);
				timetocompute=FILTER_CALC_TIME;
			} output = m_filter.res(output);
		}

		rcVol+=(((GetEnvAmp()*((1-vpar->ampVelocity)+(voiceVol*vpar->ampVelocity))*softenHighNotes)-rcVol)*rcVolCutoff);
		output*=rcVol;
		slr[0]=output*vpar->stereoLR[currentStereoPos];
		slr[1]=output*vpar->stereoLR[1-currentStereoPos];
	}
}

void CSynthTrack::GetEnvMod(){
	switch (modEnvStage){
	case 1:
		modEnvValue+=modEnvCoef;
		if(modEnvValue>1.0f){
			modEnvCoef=1.0f/(float)vpar->modD;
			modEnvStage=2;
		}
		break;
	case 2:
		modEnvValue-=modEnvCoef;
		if(modEnvValue<0){
			modEnvValue=0;
			modEnvStage=0;
		}
		break;
	}
}

float CSynthTrack::GetEnvAmp(){
	switch(ampEnvStage)
	{
	case 1: // Attack
		ampEnvValue+=ampEnvCoef;
		
		if(ampEnvValue>1.0f)
		{
			ampEnvCoef=((1.0f-ampEnvSustainLevel)/(float)vpar->ampD)*speedup;
			ampEnvStage=2;
		}

		return ampEnvValue;
	break;

	case 2: // Decay
		ampEnvValue-=ampEnvCoef;
		
		if(ampEnvValue<ampEnvSustainLevel)
		{
			ampEnvValue=ampEnvSustainLevel;
			ampEnvCoef=((ampEnvSustainLevel)/(float)vpar->ampD2)*speedup;
			ampEnvStage=3;

			if(!vpar->ampS)
			ampEnvStage=0;
		}

		return ampEnvValue;
	break;

	case 3:
		// Decay 2
		ampEnvValue-=ampEnvCoef;
		
		if(ampEnvValue<=0){
			ampEnvValue=0;
			ampEnvStage=0;
		}
		return ampEnvValue;
	break;

	case 4: // Release
		ampEnvValue-=ampEnvCoef;

		if(ampEnvValue<0.0f){
			ampEnvValue=0.0f;
			ampEnvStage=0;
		}
		return ampEnvValue;
	break;
	
	case 5: // FastRelease
		ampEnvValue-=ampEnvCoef;

		if(ampEnvValue<0.0f)
		{
			ampEnvValue=0.0f;
			RealNoteOn();
			ampEnvStage=1;
			ampEnvCoef=(1.0f/(float)vpar->ampA)*speedup;
		}

		return ampEnvValue;
	break;

	}

	return 0;
}

void CSynthTrack::GetEnvFlt()
{
	switch(fltEnvStage)
	{
		case 1: // Attack
		fltEnvValue+=fltEnvCoef;
		
		if(fltEnvValue>1.0f)
		{
			fltEnvCoef=((1.0f-fltEnvSustainLevel)/(float)vpar->fltD)*speedup2;
			fltEnvStage=2;
		}
	break;

	case 2: // Decay
		fltEnvValue-=fltEnvCoef;
		
		if(fltEnvValue<fltEnvSustainLevel)
		{
			fltEnvValue=fltEnvSustainLevel;
			fltEnvCoef=((fltEnvSustainLevel)/(float)vpar->fltD2)*speedup2;
			fltEnvStage=3;
		}
	break;

	case 3:
		// Decay 2
		fltEnvValue-=fltEnvCoef;
		
		if(fltEnvValue<=0)
		{
			fltEnvValue=0;
			fltEnvStage=0;
		}
	break;

	case 4: // Release
		fltEnvValue-=fltEnvCoef;

		if(fltEnvValue<0.0f)
		{
			fltEnvValue=0.0f;
			fltEnvStage=0;
		}
	break;

	case 5: // FastRelease
		fltEnvValue-=fltEnvCoef;

		if(fltEnvValue<0.0f)
		{
			fltEnvValue=0.0f;
			fltEnvStage=1;
			fltEnvCoef=(1.0f/(float)vpar->fltA)*speedup2;
		}
	break;
	}
}

void CSynthTrack::NoteOff()
{
	stopRetrig=true;
	if(ampEnvStage){
		ampEnvStage=4;
		fltEnvStage=4;
		ampEnvCoef=(ampEnvValue/(float)vpar->ampR)*speedup;
		fltEnvCoef=(fltEnvValue/(float)vpar->fltR)*speedup2;
	}
}

void CSynthTrack::NoteStop()
{
	stopRetrig=true;
	if(ampEnvStage){
		ampEnvStage=4;
		fltEnvStage=4;
		ampEnvCoef=ampEnvValue/32.0f;
		fltEnvCoef=fltEnvValue/32.0f;
	}
}

void CSynthTrack::DoGlide() {

	// Glide Handler
	if(rbasenote<basenote){
		rbasenote+=DCOglide;
		if(rbasenote>basenote) rbasenote=basenote;
		tuningChange=true;
	}else{
		if (rbasenote>basenote){
			rbasenote-=DCOglide;
			if(rbasenote<basenote) rbasenote=basenote;
			tuningChange=true;
		}
	}
	// Semi Glide Handler
	if(rsemitone<semitone){
		rsemitone+=semiglide;
		if(rsemitone>semitone) rsemitone=semitone;
		tuningChange=true;
	}else{
		if (rsemitone>semitone){
			rsemitone-=semiglide;
			if(rsemitone<semitone) rsemitone=semitone;
			tuningChange=true;
		}
	}
}

void CSynthTrack::PerformFx()
{
	float shift;

	// Perform tone glide
	DoGlide();

	if ( ((sp_cmd & 0xF0) == 0xD0) || ((sp_cmd & 0xF0) == 0xE0)) semiglide=sp_gl;

	if ( (sp_cmd & 0xF0) < 0xC0) {
		if (sp_cmd != 0) {
			arpInput[1] = sp_cmd>>4;
			arpInput[2] = sp_cmd&0x0f;
			arpInput[3] = sp_val;
			switch (vpar->arpPattern) {
			case 0: // 1oct Up
				arpList[0]=arpInput[0];
				arpList[1]=arpInput[1];
				arpList[2]=arpInput[2];
				if (arpInput[3] == 0) {
					arpLen=3;
				} else {
					arpLen=4;
					arpList[3]=arpInput[3];
				}
				break;
			case 1: // 2oct Up
				arpList[0]=arpInput[0];
				arpList[1]=arpInput[1];
				arpList[2]=arpInput[2];
				if (arpInput[3] == 0) {
					arpLen=3;
					arpList[3]=arpInput[0]+12;
					arpList[4]=arpInput[1]+12;
					arpList[5]=arpInput[2]+12;
				} else {
					arpLen=8;
					arpList[3]=arpInput[3];
					arpList[4]=arpInput[0]+12;
					arpList[5]=arpInput[1]+12;
					arpList[6]=arpInput[2]+12;
					arpList[7]=arpInput[3]+12;
				}
				break;
			case 2: // 3oct Up
				arpList[0]=arpInput[0];
				arpList[1]=arpInput[1];
				arpList[2]=arpInput[2];
				if (arpInput[3] == 0) {
					arpLen=9;
					arpList[3]=arpInput[0]+12;
					arpList[4]=arpInput[1]+12;
					arpList[5]=arpInput[2]+12;
					arpList[6]=arpInput[0]+24;
					arpList[7]=arpInput[1]+24;
					arpList[8]=arpInput[2]+24;
				} else {
					arpLen=12;
					arpList[3]=arpInput[3];
					arpList[4]=arpInput[0]+12;
					arpList[5]=arpInput[1]+12;
					arpList[6]=arpInput[2]+12;
					arpList[7]=arpInput[3]+12;
					arpList[8]=arpInput[0]+24;
					arpList[9]=arpInput[1]+24;
					arpList[10]=arpInput[2]+24;
					arpList[11]=arpInput[3]+24;
				}
				break;
			case 3: // 1oct Down
				if (arpInput[3] == 0) {
					arpLen=3;
					arpList[0]=arpInput[2];
					arpList[1]=arpInput[1];
					arpList[2]=arpInput[0];
				} else {
					arpLen=4;
					arpList[0]=arpInput[3];
					arpList[1]=arpInput[2];
					arpList[2]=arpInput[1];
					arpList[3]=arpInput[0];
				}			
				break;
			case 4: // 2oct Down
				if (arpInput[3] == 0) {
					arpLen=6;
					arpList[0]=arpInput[2]+12;
					arpList[1]=arpInput[1]+12;
					arpList[2]=arpInput[0]+12;
					arpList[3]=arpInput[2];
					arpList[4]=arpInput[1];
					arpList[5]=arpInput[0];
				} else {
					arpLen=8;
					arpList[0]=arpInput[3]+12;
					arpList[1]=arpInput[2]+12;
					arpList[2]=arpInput[1]+12;
					arpList[3]=arpInput[0]+12;
					arpList[4]=arpInput[3];
					arpList[5]=arpInput[2];
					arpList[6]=arpInput[1];
					arpList[7]=arpInput[0];
				}			
				break;
			case 5: // 3oct Down
				if (arpInput[3] == 0) {
					arpLen=9;
					arpList[0]=arpInput[2]+24;
					arpList[1]=arpInput[1]+24;
					arpList[2]=arpInput[0]+24;
					arpList[3]=arpInput[2]+12;
					arpList[4]=arpInput[1]+12;
					arpList[5]=arpInput[0]+12;
					arpList[6]=arpInput[2];
					arpList[7]=arpInput[1];
					arpList[8]=arpInput[0];
				} else {
					arpLen=12;
					arpList[0]=arpInput[3]+24;
					arpList[1]=arpInput[2]+24;
					arpList[2]=arpInput[1]+24;
					arpList[3]=arpInput[0]+24;
					arpList[4]=arpInput[3]+12;
					arpList[5]=arpInput[2]+12;
					arpList[6]=arpInput[1]+12;
					arpList[7]=arpInput[0]+12;
					arpList[8]=arpInput[3];
					arpList[9]=arpInput[2];
					arpList[10]=arpInput[1];
					arpList[11]=arpInput[0];
				}			
				break;
			case 6: // 1oct Up/Down
				if (arpInput[3] == 0) {
					arpLen=4;
					arpList[0]=arpInput[0];
					arpList[1]=arpInput[1];
					arpList[2]=arpInput[2];
					arpList[3]=arpInput[1];
				} else {
					arpLen=6;
					arpList[0]=arpInput[0];
					arpList[1]=arpInput[1];
					arpList[2]=arpInput[2];
					arpList[3]=arpInput[3];
					arpList[4]=arpInput[2];
					arpList[5]=arpInput[1];
				}			
				break;
			case 7: // 2oct Up/Down
				if (arpInput[3] == 0) {
					arpLen=10;
					arpList[0]=arpInput[0];
					arpList[1]=arpInput[1];
					arpList[2]=arpInput[2];
					arpList[3]=arpInput[0]+12;
					arpList[4]=arpInput[1]+12;
					arpList[5]=arpInput[2]+12;
					arpList[6]=arpInput[1]+12;
					arpList[7]=arpInput[0]+12;
					arpList[8]=arpInput[2];
					arpList[9]=arpInput[1];
				} else {
					arpLen=14;
					arpList[0]=arpInput[0];
					arpList[1]=arpInput[1];
					arpList[2]=arpInput[2];
					arpList[3]=arpInput[3];
					arpList[4]=arpInput[0]+12;
					arpList[5]=arpInput[1]+12;
					arpList[6]=arpInput[2]+12;
					arpList[7]=arpInput[3]+12;
					arpList[8]=arpInput[2]+12;
					arpList[9]=arpInput[1]+12;
					arpList[10]=arpInput[0]+12;
					arpList[11]=arpInput[3];
					arpList[12]=arpInput[2];
					arpList[13]=arpInput[1];
				}			
				break;
			case 8: // 3oct Up/Down
				if (arpInput[3] == 0) {
					arpLen=16;
					arpList[0]=arpInput[0];
					arpList[1]=arpInput[1];
					arpList[2]=arpInput[2];
					arpList[3]=arpInput[0]+12;
					arpList[4]=arpInput[1]+12;
					arpList[5]=arpInput[2]+12;
					arpList[6]=arpInput[0]+24;
					arpList[7]=arpInput[1]+24;
					arpList[8]=arpInput[2]+24;
					arpList[9]=arpInput[1]+24;
					arpList[10]=arpInput[0]+24;
					arpList[11]=arpInput[2]+12;
					arpList[12]=arpInput[1]+12;
					arpList[13]=arpInput[0]+12;
					arpList[14]=arpInput[2];
					arpList[15]=arpInput[1];
				} else {
					arpLen=22;
					arpList[0]=arpInput[0];
					arpList[1]=arpInput[1];
					arpList[2]=arpInput[2];
					arpList[3]=arpInput[3];
					arpList[4]=arpInput[0]+12;
					arpList[5]=arpInput[1]+12;
					arpList[6]=arpInput[2]+12;
					arpList[7]=arpInput[3]+12;
					arpList[8]=arpInput[0]+24;
					arpList[9]=arpInput[1]+24;
					arpList[10]=arpInput[2]+24;
					arpList[11]=arpInput[3]+24;
					arpList[12]=arpInput[2]+24;
					arpList[13]=arpInput[1]+24;
					arpList[14]=arpInput[0]+24;
					arpList[15]=arpInput[3]+12;
					arpList[16]=arpInput[2]+12;
					arpList[17]=arpInput[1]+12;
					arpList[18]=arpInput[0]+12;
					arpList[19]=arpInput[3];
					arpList[20]=arpInput[2];
					arpList[21]=arpInput[1];
				}			
				break;
			case 9: //1oct Down/Up
				if (arpInput[3] == 0) {
					arpLen=4;
					arpList[0]=arpInput[2];
					arpList[1]=arpInput[1];
					arpList[2]=arpInput[0];
					arpList[3]=arpInput[1];
				} else {
					arpLen=6;
					arpList[0]=arpInput[3];
					arpList[1]=arpInput[2];
					arpList[2]=arpInput[1];
					arpList[3]=arpInput[0];
					arpList[4]=arpInput[1];
					arpList[5]=arpInput[2];
				}			
				break;
			case 10: //2oct Down/Up
				if (arpInput[3] == 0) {
					arpLen=10;
					arpList[0]=arpInput[2]+12;
					arpList[1]=arpInput[1]+12;
					arpList[2]=arpInput[0]+12;
					arpList[3]=arpInput[2];
					arpList[4]=arpInput[1];
					arpList[5]=arpInput[0];
					arpList[6]=arpInput[1];
					arpList[7]=arpInput[2];
					arpList[8]=arpInput[0]+12;
					arpList[9]=arpInput[1]+12;
				} else {
					arpLen=14;
					arpList[0]=arpInput[3]+12;
					arpList[1]=arpInput[2]+12;
					arpList[2]=arpInput[1]+12;
					arpList[3]=arpInput[0]+12;
					arpList[4]=arpInput[3];
					arpList[5]=arpInput[2];
					arpList[6]=arpInput[1];
					arpList[7]=arpInput[0];
					arpList[8]=arpInput[1];
					arpList[9]=arpInput[2];
					arpList[10]=arpInput[3];
					arpList[11]=arpInput[0]+12;
					arpList[12]=arpInput[1]+12;
					arpList[13]=arpInput[2]+12;
				}			
				break;
			case 11: //3oct Down/Up
				if (arpInput[3] == 0) {
					arpLen=16;
					arpList[0]=arpInput[2]+24;
					arpList[1]=arpInput[1]+24;
					arpList[2]=arpInput[0]+24;
					arpList[3]=arpInput[2]+12;
					arpList[4]=arpInput[1]+12;
					arpList[5]=arpInput[0]+12;
					arpList[6]=arpInput[2];
					arpList[7]=arpInput[1];
					arpList[8]=arpInput[0];
					arpList[9]=arpInput[1];
					arpList[10]=arpInput[2];
					arpList[11]=arpInput[0]+12;
					arpList[12]=arpInput[1]+12;
					arpList[13]=arpInput[2]+12;
					arpList[14]=arpInput[0]+24;
					arpList[15]=arpInput[1]+24;
				} else {
					arpLen=6;
					arpList[0]=arpInput[3]+24;
					arpList[1]=arpInput[2]+24;
					arpList[2]=arpInput[1]+24;
					arpList[3]=arpInput[0]+24;
					arpList[4]=arpInput[3]+12;
					arpList[5]=arpInput[2]+12;
					arpList[6]=arpInput[1]+12;
					arpList[7]=arpInput[0]+12;
					arpList[8]=arpInput[3];
					arpList[9]=arpInput[2];
					arpList[10]=arpInput[1];
					arpList[11]=arpInput[0];
					arpList[12]=arpInput[1];
					arpList[13]=arpInput[2];
					arpList[14]=arpInput[3];
					arpList[15]=arpInput[0]+12;
					arpList[16]=arpInput[1]+12;
					arpList[17]=arpInput[2]+12;
					arpList[18]=arpInput[3]+12;
					arpList[19]=arpInput[0]+24;
					arpList[20]=arpInput[1]+25;
					arpList[21]=arpInput[2]+24;
				}			
				break;
			}
		}
	} else {
		switch (sp_cmd) {
			/* 0xC1 : Pitch Up */
			case 0xC1:
				shift=(float)sp_val*0.001f;
				Bend+=shift;
			break;
			/* 0xC2 : Pitch Down */
			case 0xC2:
				shift=(float)sp_val*0.001f;
				Bend-=shift;
			break;
			/* 0xC5 Intervall */
			case 0xC5:
				arpList[0]=0;
				if (sp_val > 127) arpList[1]=0-(sp_val & 0x7F);
				else arpList[1]=sp_val;
				arpLen=2;
			break;
			/* 0xC6 Touchtaping */
			case 0xC6:
				if (sp_val > 127) arpList[0]=0-(sp_val & 0x7F);
				else arpList[0]=sp_val;
				arpLen=1;
			break;
			/* 0xC7 Touchtaping with Retrig */
			case 0xC7:
				if (sp_val > 127) arpList[0]=0-(sp_val & 0x7F);
				else arpList[0]=sp_val;
				arpLen=1;
			break;

			/* 0xCC Volume */
			case 0xCC:
				masterVolume=(float)sp_val*volMulti;
			break;
		}
	}
}

void CSynthTrack::InitEffect(int cmd, int val)
{
	sp_cmd=cmd;
	sp_val=val;

	// Init glide
	if (cmd == 0xC3 || cmd == 0xC4) {
		DCOglide=(float)(val*val)*0.0000625f;
	} else 	DCOglide=DefGlide;

	//SemiGlide
	if ( ((cmd & 0xF0) == 0xD0) || ((cmd & 0xF0) == 0xE0)){
		sp_gl=(val*val)*0.0000625f;
		if ((cmd & 0xF0) == 0xD0) semitone=0.0f-(float)(cmd&15);
		else {
			if ((cmd & 0xF0) == 0xE0) semitone=(float)(cmd&15);
		else {semitone=0.0f;
				semiglide=DefGlide;
			}
		}
	}

	// Touchtaping
	if (cmd == 0xC6) { arpLen=1; arpCount=-1; }
	// Touchtaping with Retrig
	if (cmd == 0xC7) { InitEnvelopes(); arpLen=1; arpCount=-1; }
	if (cmd == 0xCC) voiceVol=(float)val/255.0f;
}
