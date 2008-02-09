/* -*- mode:c++, indent-tabs-mode:t -*- */
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include "drum.hpp"
#include <cstdlib>

#define DRUM_VERSION "2.2"
#define MAX_SIMUL_TRACKS 16
#define NUMPARAMETERS 16


CMachineParameter const prStartFreq = {
	"Start Freq",
	"Start Frequency (Hz)",
	80,
	600,
	MPF_STATE,
	200
};

CMachineParameter const prEndFreq = {
	"End Freq",
	"End Frequency (Hz)",
	20,
	400,
	MPF_STATE,
	44
};
CMachineParameter const prFreqDecay = {
	"Freq Decay",
	"Frequency Down Speed (in mscs)",
	10,
	300,
	MPF_STATE,
	65
};
CMachineParameter const prStartAmp = {
	"Start Amp (1)",
	"Starting Amplitude",
	0,
	32767,
	MPF_STATE,
	32767
};
CMachineParameter const prEndAmp = {
	"End Amp (1)",
	"Ending Amplitude",
	0,
	32767,
	MPF_STATE,
	21844
};

CMachineParameter const prLength = {
	"Length",
	"Duration of the note (ms)",
	10,
	500,
	MPF_STATE,
	180
};
CMachineParameter const prOutVol = {
	"Volume",
	"Volume (0-32767)",
	0,
	32767,
	MPF_STATE,
	32767
};
CMachineParameter const prDecMode = {
	"Dec Mode",
	"Decrement Mode",
	0,
	3,
	MPF_STATE,
//				2
	0
};
CMachineParameter const prComp = {
	"Compatible(1/2)",
	"Compatible With version",
	0,
	1,
	MPF_STATE,
//				1
	0
};
CMachineParameter const prNNA = {
	"NNA Command",
	"NNA Command when New Note",
	0,
	1,
	MPF_STATE,
	0
};

CMachineParameter const prAttack = {
	"Attack up to (2)",
	"Attack from 0/100 to x/100",
	0,
	99,
	MPF_STATE,
	2
};
CMachineParameter const prDecay = {
	"Decay up to (2)",
	"Decay from Attack to x/100",
	1,
	100,
	MPF_STATE,
	63
};
CMachineParameter const prSustain = {
	"Sustain Volume (2)",
	"Sustain Volume at Decay-End Pos",
	0,
	100,
	MPF_STATE,
	55
};
CMachineParameter const prMix = {
	"Drum&Thump Mix",
	"Mix of Drum and Thump Signals",
	-100,
	100,
	MPF_STATE,
//				-37
	-100
};
CMachineParameter const prThumpLen = {
	"Thump Length",
	"Thump Length",
	1,
	60,
	MPF_STATE,
	10
};
CMachineParameter const prThumpFreq = {
	"Thump Freq",
	"Thump Frequency",
	220,
	6000,
	MPF_STATE,
	5000
};
CMachineParameter const prNothing = {
	"-",
	"Nothing",
	0,
	1,
	MPF_STATE,
	0
};

CMachineParameter const *pParameters[] = 
{ 
	&prStartFreq,
	&prEndFreq,
	&prFreqDecay,
	&prStartAmp,
	&prEndAmp,
	&prLength,
	&prOutVol,
	&prDecMode,
	&prComp,
	&prNNA,
	&prAttack,
	&prDecay,
	&prSustain,
	&prMix,
	&prThumpLen,
	&prThumpFreq,
};

CMachineInfo const MacInfo =
{
	MI_VERSION,
	GENERATOR,
	NUMPARAMETERS,
	pParameters,
#ifdef _DEBUG
	"Drum Synth v." DRUM_VERSION " (Debug)",
#else
	"Drum Synth v." DRUM_VERSION,
#endif
	"Drum" DRUM_VERSION,
	"[JAZ] on " __DATE__,
	"Command Help",
	4
};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
	virtual void Stop();
	
	int GetVoice(int channel,bool getnew=false);
	void DeallocateVoice(int voice);
private:

	Drum DTrack[MAX_SIMUL_TRACKS];

	int allocatedvoice[MAX_TRACKS];
	int numtracks;

	DrumPars globalpar;
		
};


PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	Vals=new int[NUMPARAMETERS];

	numtracks=0;
	for (int i=0;i<MAX_TRACKS;i++) allocatedvoice[i]=-1;

}

mi::~mi()
{
	delete Vals;

// Destroy dinamically allocated objects/memory here
}

void mi::Init()
{
// Initialize your stuff here

}
void mi::Stop()
{
	for(int c=0;c<numtracks;c++)
		DTrack[c].NoteOff();
	for (int i=0;i<MAX_TRACKS;i++) allocatedvoice[i]=-1;
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	Vals[par]=val;
	switch (par) {
		case 0: 
			globalpar.StartSpeed=val * (float)(MAX_ENVPOS)/pCB->GetSamplingRate();
			globalpar.IncSpeed= (Vals[1] - val) * (float)(MAX_ENVPOS)/ (globalpar.DecLength*pCB->GetSamplingRate());
			break;
		case 1:
			globalpar.IncSpeed= (val - Vals[0]) * (float)(MAX_ENVPOS)/ (globalpar.DecLength*pCB->GetSamplingRate());
			break;
		case 2:
			globalpar.DecLength=val * pCB->GetSamplingRate() / 1000.0;
			globalpar.IncSpeed= (Vals[1] - Vals[0]) * (float)(MAX_ENVPOS)/ (globalpar.DecLength*pCB->GetSamplingRate());
			break;
		case 3:
			if ( Vals[8] == 0 ) {
				globalpar.SinVol=val*globalpar.sinmix/100.0;
				globalpar.DecayDec=((val-Vals[4])*globalpar.sinmix)/(globalpar.SLength*100.0);
			}
			break;
		case 4:
			if ( Vals[8] == 0 ) {
				globalpar.DecayDec=((Vals[3]-val)*globalpar.sinmix)/(globalpar.SLength*100.0);
				//globalpar.SustainDec; // <- Not Used
			}
			break;
		case 5:
			globalpar.SLength=(val* pCB->GetSamplingRate())/1000;
			if ( Vals[8] == 0 ) {
				//globalpar.AttackPos; globalpar.AttackInc; //<- Not Used
				//globalpar.DecayPos=globalpar.SLength;
				globalpar.DecayDec=((Vals[3]-Vals[4])*globalpar.sinmix)/(globalpar.SLength*100.0);
				//globalpar.SustainDec; // <- Not Used
			}
			else {
				if ( Vals[10] != 0 ) { // If it's 0, they don't change.
					globalpar.AttackPos=globalpar.SLength*Vals[10]/100;
					globalpar.AttackInc=(327.67*globalpar.sinmix)/globalpar.AttackPos;
				}
				if ( Vals[10] > Vals[11] ) globalpar.DecayPos=globalpar.AttackPos+1;
				else globalpar.DecayPos=globalpar.SLength*Vals[11]/100;
				globalpar.DecayDec=((327.67-(3.2767*Vals[12]))*globalpar.sinmix)/(globalpar.DecayPos-globalpar.AttackPos);
				globalpar.SustainDec=(3.2767*Vals[12]*globalpar.sinmix)/(globalpar.SLength-globalpar.DecayPos);
			}
			break;
		case 6:				globalpar.OutVol=val/32767.0;
			break;
		case 7:				globalpar.DecMode=val;
			break;
		case 8: 
			if ( val == 0 ) {
				globalpar.AttackPos=0; // globalpar.AttackInc; //<- Not used
				globalpar.SinVol=Vals[3]*globalpar.sinmix/100.0;
				globalpar.DecayPos=globalpar.SLength;
				globalpar.DecayDec=((Vals[3]-Vals[4])*globalpar.sinmix)/(globalpar.SLength*100.0);
				//globalpar.SustainDec; // <- Not Used.
			}
			else {
				if ( Vals[10] != 0 ) {
					globalpar.AttackPos=globalpar.SLength*Vals[10]/100;
					globalpar.AttackInc=(327.67*globalpar.sinmix)/globalpar.AttackPos;
					globalpar.SinVol=0;
				}
				else {  
					globalpar.AttackPos=0; // globalpar.AttackInc; //<- Not used
					globalpar.SinVol=327.67*globalpar.sinmix;
				}
				if ( Vals[10] > Vals[11] ) globalpar.DecayPos=globalpar.AttackPos+1;
				else globalpar.DecayPos=globalpar.SLength*Vals[11]/100;
				globalpar.DecayDec=((327.67-(3.2767*Vals[12]))*globalpar.sinmix)/(globalpar.DecayPos-globalpar.AttackPos);
				globalpar.SustainDec=(3.2767*Vals[12]*globalpar.sinmix)/(globalpar.SLength-globalpar.DecayPos);
			}
			break;
		case 9: break;
		case 10:
			if ( Vals[8] == 1 ) {
				if ( val != 0 ) { 
					globalpar.AttackPos=globalpar.SLength*val/100;
					globalpar.AttackInc=(327.67*globalpar.sinmix)/globalpar.AttackPos;
					globalpar.SinVol=0;
				}
				else {  
					globalpar.AttackPos=0;
					globalpar.SinVol=327.67*globalpar.sinmix;
				}
			}
			break;
		case 11:
			if ( Vals[8] == 1 ) {
				if ( Vals[10] > val ) globalpar.DecayPos=globalpar.AttackPos+1;
				else globalpar.DecayPos=globalpar.SLength*val/100;
				globalpar.DecayDec=((327.67-(3.2767*Vals[12]))*globalpar.sinmix)/(globalpar.DecayPos-globalpar.AttackPos);
				globalpar.SustainDec=(3.2767*Vals[12]*globalpar.sinmix)/(globalpar.SLength-globalpar.DecayPos);
			}
			break;
		case 12:
			if ( Vals[8] == 1 ) {
				globalpar.DecayDec=((327.67-(3.2767*val))*globalpar.sinmix)/(globalpar.DecayPos-globalpar.AttackPos);
				globalpar.SustainDec=(3.2767*val*globalpar.sinmix)/(globalpar.SLength-globalpar.DecayPos);
			}
			break;
		case 13: 
			if ( val > 0 ) { globalpar.sinmix=(-1)*(val-100); globalpar.ThumpVol=32767; }
			else { globalpar.sinmix=100; globalpar.ThumpVol=327.67*(val+100); }
			if ( globalpar.AttackPos == 0 ) {
				if ( Vals[8] == 0 ) globalpar.SinVol=Vals[3]*globalpar.sinmix/100.0;
				else globalpar.SinVol=327.67*globalpar.sinmix;
			}
			else {
				globalpar.SinVol=0;
				globalpar.AttackInc=(327.67*globalpar.sinmix)/globalpar.AttackPos;
			}
			if ( Vals[8] == 0 ) {
				globalpar.DecayDec=((Vals[3]-Vals[4])*globalpar.sinmix)/(globalpar.SLength*100.0);
			}
			else {
				globalpar.DecayDec=((327.67-(3.2767*Vals[12]))*globalpar.sinmix)/(globalpar.DecayPos-globalpar.AttackPos);
				globalpar.SustainDec=(3.2767*Vals[12]*globalpar.sinmix)/(globalpar.SLength-globalpar.DecayPos);
			}
			globalpar.ThumpDec=globalpar.ThumpVol/globalpar.ThumpLength;
			break;
		case 14: 
			globalpar.ThumpLength=(val* pCB->GetSamplingRate())/10000;
			globalpar.ThumpDec=globalpar.ThumpVol/globalpar.ThumpLength;
			break;
		case 15:
			globalpar.ThumpFreq=val;
		default:break;
	}
}
void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
char buffer[256];

sprintf(

		buffer,"%s%s%s%s",
		"Pattern commands\n\n",
		"0Cxx : Set Volume\n\n",
		"  Compatible 1.x : $00->0 $FF->32767\n",
		"  Compatible 2.x : $00->0 $FF->OutVol\n\0"
		);

pCB->MessBox(buffer,"·-=<[JAZ]> JMDrum Synth v." DRUM_VERSION "=-·",0);

}
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	float sl=0;

	for(int c=0;c<MAX_SIMUL_TRACKS;c++)
	{
		if(DTrack[c].AmpEnvStage)
		{
		
			float *xpsamplesleft=psamplesleft;
			float *xpsamplesright=psamplesright;
			int xnumsamples=numsamples;
		
			--xpsamplesleft;
			--xpsamplesright;

			do
			{
				sl=DTrack[c].GetSample();

				*++xpsamplesleft+=sl;
				*++xpsamplesright+=sl;
			
			} while(--xnumsamples);
			
			if(DTrack[c].AmpEnvStage == ST_NONOTE)
			{
				DeallocateVoice(c);
			}
			else
				DTrack[c].Started = true;
		}
	}
}

bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param) {
		case 0:
		case 1:
		case 15:
			sprintf(txt,"%1dHz",value); return true;
		case 2:
		case 5:
			sprintf(txt,"%1dms",value); return true;
		case 7:
			switch(value) {
				case 0: sprintf(txt,"Linear");break;
				case 1: sprintf(txt,"Mode 1.1");break;
				case 2: sprintf(txt,"Mode 2.0");break;
				default: sprintf(txt,"No Decrease");break;
			}
			return true;
		case 8:
			sprintf(txt,"%s",value==0?"Version 1.x(1)":"Version 2.x(2)");
			return true;
		case 9:
			sprintf(txt,"%s",value==0?"NoteOff":"NoteContinue");return true;
		case 10:
		case 11:
		case 12:
			sprintf(txt,"%d%%",value); return true;
		case 13:
			if ( value< 0 ) sprintf(txt,"100%%:%d%%",(value+100));
			else sprintf(txt,"%d%%:100%%",(value-100)*(-1));
			return true;
		case 14:
			sprintf(txt,"%0.1fms",value/10.0f); return true;
		default: return false;
	}
}

void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	int vidx = GetVoice(channel);
	if(note<120) // Note on
	{
		if (vidx != -1 )
		{
			if ( DTrack[vidx].AmpEnvStage && !DTrack[vidx].Started)
				return;

			if (Vals[9]==0) // If NNA is Noteoff, then, do so.
			{
				DTrack[vidx].NoteOff();
			}
		}
		vidx = GetVoice(channel,true);

		DTrack[vidx].Started=false;
		globalpar.samplerate=pCB->GetSamplingRate();

		float tmp=globalpar.OutVol;

		if ( Vals[8] == 0 )				// If Mode 1.x
		{
			if ( cmd == 0x0C ) globalpar.OutVol=(val*Vals[3]/8388352.0);  // (val*(Vals[3]/32767))/256

			DTrack[vidx].NoteOn(48,&globalpar); // C-4 always
		}
		else
		{
			if ( cmd == 0x0C ) globalpar.OutVol=(val*tmp/256.0);

			DTrack[vidx].NoteOn(note,&globalpar);
		}

		globalpar.OutVol=tmp;				//restore outvol
	}
	else if (vidx != -1)
	{
		if (note==120)
		{
			DTrack[vidx].NoteOff(); // Note off
		}
		else if ( cmd == 0x0C )
		{
			if ( Vals[8]==0 ) DTrack[vidx].OutVol=(val/256.0); // Comp 1.x
			else DTrack[vidx].OutVol=(val*globalpar.OutVol/256.0); // Comp 2.x
		}
	}
}

int mi::GetVoice(int channel,bool getnew)
{
	// If this channel already has an allocated voice, use it.
	if ( !getnew ) 
	{
		return allocatedvoice[channel];
	}
	// If not, search an available voice
	int j=channel%MAX_SIMUL_TRACKS;
	while (j<MAX_SIMUL_TRACKS && DTrack[j].Chan != -1) j++;
	if ( j == MAX_SIMUL_TRACKS)
	{
		j=0;
		while (j<MAX_SIMUL_TRACKS && DTrack[j].Chan != -1) j++;
		if (j == MAX_SIMUL_TRACKS)
		{
			j = (unsigned int) (  (double)rand() * MAX_SIMUL_TRACKS /(((double)RAND_MAX) + 1.0 ) );
		}
	}
	allocatedvoice[channel]=j;
	DTrack[j].Chan = channel;
	return j;
}
void mi::DeallocateVoice(int voice)
{
	if ( DTrack[voice].Chan == -1)
		return;

	if (allocatedvoice[DTrack[voice].Chan] == voice )
	{
		allocatedvoice[DTrack[voice].Chan] = -1;
	}
	DTrack[voice].Chan = -1;
}

