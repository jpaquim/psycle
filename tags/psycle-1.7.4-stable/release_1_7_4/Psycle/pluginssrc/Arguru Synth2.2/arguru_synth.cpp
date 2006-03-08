/////////////////////////////////////////////////////////////////////
// Arguru reverb plugin for PSYCLE

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "..\..\machineinterface.h"

#include "synthtrack.h"
#include "filtercoef.h"

#define MAX_ENV_TIME	250000
#define MAX_TRACKS	32

CMachineParameter const paraOSC1wave = 
{ 
	"OSC1 Wave",
	"OSC1 Wave",									// description
	0,												// MinValue	
	4,												// MaxValue
	MPF_STATE,										// Flags
	1
};

CMachineParameter const paraOSC2wave = 
{ 
	"OSC2 Wave",
	"OSC2 Wave",									// description
	0,												// MinValue	
	4,												// MaxValue
	MPF_STATE,										// Flags
	1
};


CMachineParameter const paraOSC2detune = 
{ 
	"OSC2 Detune",
	"OSC2 Detune",									// description
	-36,											// MinValue	
	36,												// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraOSC2finetune = 
{ 
	"OSC2 Finetune",
	"OSC2 Finetune",								// description
	0,												// MinValue	
	256,											// MaxValue
	MPF_STATE,										// Flags
	27
};

CMachineParameter const paraOSC2sync = 
{ 
	"OSC2 Sync",
	"OSC2 Sync",									// description
	0,												// MinValue	
	1,												// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraVCAattack = 
{ 
	"VCA Attack",
	"VCA Attack",									// description
	32,												// MinValue	
	MAX_ENV_TIME,									// MaxValue
	MPF_STATE,										// Flags
	32
};


CMachineParameter const paraVCAdecay = 
{ 
	"VCA Decay",
	"VCA Decay",									// description
	32,												// MinValue	
	MAX_ENV_TIME,									// MaxValue
	MPF_STATE,										// Flags
	6341
};

CMachineParameter const paraVCAsustain = 
{ 
	"VCA Sustain",
	"VCA Sustain level",							// description
	0,												// MinValue	
	256,											// MaxValue
	MPF_STATE,										// Flags
	0
};


CMachineParameter const paraVCArelease = 
{ 
	"VCA Release",
	"VCA Release",									// description
	32,												// MinValue	
	MAX_ENV_TIME,									// MaxValue
	MPF_STATE,										// Flags
	2630
};


CMachineParameter const paraVCFattack = 
{ 
	"VCF Attack",
	"VCF Attack",									// description
	32,												// MinValue	
	MAX_ENV_TIME,									// MaxValue
	MPF_STATE,										// Flags
	589
};


CMachineParameter const paraVCFdecay = 
{ 
	"VCF Decay",
	"VCF Decay",									// description
	32,												// MinValue	
	MAX_ENV_TIME,									// MaxValue
	MPF_STATE,										// Flags
	2630
};

CMachineParameter const paraVCFsustain = 
{ 
	"VCF Sustain",
	"VCF Sustain level",							// description
	0,												// MinValue	
	256,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraVCFrelease = 
{ 
	"VCF Release",
	"VCF Release",									// description
	32,												// MinValue	
	MAX_ENV_TIME,									// MaxValue
	MPF_STATE,										// Flags
	2630
};

CMachineParameter const paraVCFlfospeed = 
{ 
	"VCF LFO Speed",
	"VCF LFO Speed",								// description
	1,												// MinValue	
	65536,											// MaxValue
	MPF_STATE,										// Flags
	32
};


CMachineParameter const paraVCFlfoamplitude = 
{ 
	"VCF LFO Amplitude",
	"VCF LFO Amplitude",							// description
	0,												// MinValue	
	240,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraVCFcutoff = 
{ 
	"VCF Cutoff",
	"VCF Cutoff",									// description
	0,												// MinValue	
	240,											// MaxValue
	MPF_STATE,										// Flags
	120
};

CMachineParameter const paraVCFresonance = 
{ 
	"VCF Resonance",
	"VCF Resonance",								// description
	1,												// MinValue	
	240,											// MaxValue
	MPF_STATE,										// Flags
	128
};

CMachineParameter const paraVCFtype = 
{ 
	"VCF Type",
	"VCF Type",										// description
	0,												// MinValue	
	9,												// MaxValue
	MPF_STATE,										// Flags
	0
};


CMachineParameter const paraVCFenvmod = 
{ 
	"VCF Envmod",
	"VCF Envmod",									// description
	-240,											// MinValue	
	240,											// MaxValue
	MPF_STATE,										// Flags
	80
};

CMachineParameter const paraOSCmix = 
{ 
	"OSC Mix",
	"OSC Mix",									// description
	0,											// MinValue	
	256,											// MaxValue
	MPF_STATE,										// Flags
	128
};

CMachineParameter const paraOUTvol = 
{ 
	"Volume",
	"Volume",									// description
	0,											// MinValue	
	256,											// MaxValue
	MPF_STATE,										// Flags
	128
};

CMachineParameter const paraARPmode = 
{
	"Arpeggiator",
	"Arpeggiator",								// description
	0,											// MinValue	
	16,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraARPbpm = 
{
	"Arp. BPM",
	"Arp. BPM",									// description
	32,											// MinValue	
	1024,										// MaxValue
	MPF_STATE,									// Flags
	125
};


CMachineParameter const paraARPcount = 
{
	"Arp. Steps",
	"Arp. Steps",									// description
	0,											// MinValue	
	16,											// MaxValue
	MPF_STATE,										// Flags
	4
};

CMachineParameter const paraOSCtune = 
{ 
	"Synth Detune",
	"Synth Detune",									// description
	-36,											// MinValue	
	36,												// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraOSCglide = 
{ 
	"Glide",
	"Glide",									// description
	0,											// MinValue	
	127,												// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraOSCfinetune = 
{ 
	"Finetune",
	"Synth Finetune",									// description
	-256,											// MinValue	
	256,												// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const *pParameters[] = 
{ 
	&paraOSC1wave,
	&paraOSC2wave,
	&paraOSC2detune,
	&paraOSC2finetune,
	&paraOSC2sync,
	&paraVCAattack,
	&paraVCAdecay,
	&paraVCAsustain,
	&paraVCArelease,
	&paraVCFattack,
	&paraVCFdecay,
	&paraVCFsustain,
	&paraVCFrelease,
	&paraVCFlfospeed,
	&paraVCFlfoamplitude,
	&paraVCFcutoff,
	&paraVCFresonance,
	&paraVCFtype,
	&paraVCFenvmod,
	&paraOSCmix,
	&paraOUTvol,
	&paraARPmode,
	&paraARPbpm,
	&paraARPcount,
	&paraOSCtune,
	&paraOSCglide,
	&paraOSCfinetune
};

#pragma pack(1)		

#pragma pack()

CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	GENERATOR,								// flags
	27,										// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"Arguru Synth 2.2(Old)Dbg",			// name
#else
	"Arguru Synth 2.2(Old)",					// name
#endif
	"AS2.2",								// short name
	"Arguru and Gerwin",							// author
	"Help",									// A command, that could be use for open an editor, etc...
	4
};

class mi : public CMachineInterface
{
public:
	InitWaveTable();
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples,int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
	virtual void Stop();

private:

	signed short WaveTable[5][2100];
	CSynthTrack track[MAX_TRACKS];
	
	SYNPAR globalpar;

};

mi* pmi;

DLL_EXPORTS		// To export DLL functions to host

mi::mi()
{
	pmi=this;
	Vals=new int[28];
	InitWaveTable();
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
	for(int c=0;c<MAX_TRACKS;c++)
	track[c].NoteOff();
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	Vals[par]=val;

	globalpar.pWave=&WaveTable[Vals[0]][0];
	globalpar.pWave2=&WaveTable[Vals[1]][0];
	globalpar.osc2detune=Vals[2];
	globalpar.osc2finetune=Vals[3];
	globalpar.osc2sync=Vals[4];
	
	globalpar.amp_env_attack=Vals[5];
	globalpar.amp_env_decay=Vals[6];
	globalpar.amp_env_sustain=Vals[7];
	globalpar.amp_env_release=Vals[8];

	globalpar.vcf_env_attack=Vals[9];
	globalpar.vcf_env_decay=Vals[10];
	globalpar.vcf_env_sustain=Vals[11];
	globalpar.vcf_env_release=Vals[12];
	globalpar.vcf_lfo_speed=Vals[13];
	globalpar.vcf_lfo_amplitude=Vals[14];

	globalpar.vcf_cutoff=Vals[15];
	globalpar.vcf_resonance=Vals[16];
	globalpar.vcf_type=Vals[17];
	globalpar.vcf_envmod=Vals[18];
	globalpar.osc_mix=Vals[19];
	globalpar.out_vol=Vals[20];
	globalpar.arp_mod=Vals[21];
	globalpar.arp_bpm=Vals[22];
	globalpar.arp_cnt=Vals[23];
	globalpar.osctune=Vals[24];
	globalpar.oscglide=Vals[25];
	globalpar.oscfinetune=Vals[26];
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
char buffer[2048];

sprintf(
		buffer,"%s%s%s%s%s%s%s%s%s%s",
		"Pattern commands\n",
		"\n06xx Change sustain",
		"\n07xx Change env modulation",
		"\n08xx Change cuttoff frequency ",
		"\n09xx Change resonance",
		"\n0Axx Change volume",
		"\n0Bxx Change attack",
		"\n0Cxx Detune Osc2",
		"\n0Dxx Detune whole Synth",
		"\n0Exx Change Glide RealTime\0"
		);

pCB->MessBox(buffer,"Arguru Synth 2 Point 2",0);

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{
	float sl=0;
	float sr=0;

	for(int c=0;c<tracks;c++)
	{
		if(track[c].AmpEnvStage)
		{
		track[c].PerformFx();

		float *xpsamplesleft=psamplesleft;
		float *xpsamplesright=psamplesright;
		--xpsamplesleft;
		--xpsamplesright;
		
		int xnumsamples=numsamples;
	
		CSynthTrack *ptrack=&track[c];

		if(ptrack->NoteCut)
		ptrack->NoteCutTime-=numsamples;
		
			do
			{
			sl=ptrack->GetSample();
			*++xpsamplesleft+=sl;
			*++xpsamplesright+=sl;
			} while(--xnumsamples);
		}
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	// Oscillators waveform descriptions
	if(param==0 || param==1)
	{
		switch(value)
		{
		case 0:sprintf(txt,"Sine");return true;break;
		case 1:sprintf(txt,"Sawtooth");return true;break;
		case 2:sprintf(txt,"Square");return true;break;
		case 3:sprintf(txt,"Triangle");return true;break;
		case 4:sprintf(txt,"Random");return true;break;
		}
	}

	if(param==4)
	{
		switch(value)
		{
		case 0:sprintf(txt,"Off");return true;break;
		case 1:sprintf(txt,"On");return true;break;
		}
	}

	if(param==17)
	{
		switch(value)
		{
		case 0:sprintf(txt,"Lowpass");return true;break;
		case 1:sprintf(txt,"Hipass");return true;break;
		case 2:sprintf(txt,"Bandpass");return true;break;
		case 3:sprintf(txt,"Bandreject");return true;break;
		case 4:sprintf(txt,"ParaEQ1");return true;break;
		case 5:sprintf(txt,"InvParaEQ1");return true;break;
		case 6:sprintf(txt,"ParaEQ2");return true;break;
		case 7:sprintf(txt,"InvParaEQ2");return true;break;
		case 8:sprintf(txt,"ParaEQ3");return true;break;
		case 9:sprintf(txt,"InvParaEQ3");return true;break;
		}
	}

	if(param==19)
	{
		float fv=(float)value*0.390625f;
		sprintf(txt,"%.1f%% : %.1f%%",100-fv,fv);
		return true;
	}

	if(param==20)
	{
		sprintf(txt,"%.1f%%",(float)value*0.390625f);
		return true;
	}

	if(param==21)
	{
		switch(value)
		{
		case 0:sprintf(txt,"Off");return true;break;
		case 1:sprintf(txt,"Minor1");return true;break;
		case 2:sprintf(txt,"Major1");return true;break;
		case 3:sprintf(txt,"Minor2");return true;break;
		case 4:sprintf(txt,"Major2");return true;break;
		case 5:sprintf(txt,"Riffer1");return true;break;
		case 6:sprintf(txt,"Riffer2");return true;break;
		case 7:sprintf(txt,"Riffer3");return true;break;
		case 8:sprintf(txt,"Minor Bounce");return true;break;
		case 9:sprintf(txt,"Major Bounce");return true;break;

		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////
// The SeqTick function where your notes and pattern command handlers
// should be processed. Called each tick.
// Is called by the host sequencer
	
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{

	track[channel].InitEffect(cmd,val);
	
	// Global scope synth pattern commands
	switch(cmd)
	{
	
	case 6: // Change sustain
		globalpar.amp_env_sustain=val;
	break;

	case 7: // Change envmod
		globalpar.vcf_envmod=val-128;
	break;
	
	case 8: // Change cutoff
		globalpar.vcf_cutoff=val/2;
	break;
	
	case 9: // Change reso
		globalpar.vcf_resonance=val/2;
	break;

	case 10: // Change volume
		globalpar.out_vol=val/2;
	break;

	case 11: // Change attack
		globalpar.amp_env_attack=(val*975)+32;
	break;

	case 12: // Detune Realtime :)
		globalpar.osc2finetune=val;
	break;

	case 13: // Detune OSC :)
		globalpar.osctune=(val/3.555)-36;
	break;

	case 14: // Change Glide Realtime;
		globalpar.amp_env_sustain = Vals[7];
		globalpar.oscglide=(val/2);
		globalpar.amp_env_sustain = Vals[7];
	break;
	

	}

	// Note Off			== 120
	// Empty Note Row	== 255
	// Less than note off value??? == NoteON!
	if(note<120)
	track[channel].NoteOn(note-24,&globalpar,60);

	// Note off
	if(note==120)
	track[channel].NoteOff();
}

mi::InitWaveTable()
{
	for(int c=0;c<2100;c++)
	{
		double sval=(double)c*0.00306796157577128245943617517898389;

		WaveTable[0][c]=int(sin(sval)*16384.0f);
		WaveTable[1][c]=(c*16)-16384;

		if(c<1024)
		WaveTable[2][c]=-16384;
		else
		WaveTable[2][c]=16384;

		if(c<1024)
		WaveTable[3][c]=(c*32)-16384;
		else
		WaveTable[3][c]=16384-((c-1024)*32);

		if (c<32)	WaveTable[4][c]=16384;
		else		WaveTable[4][c]=0;
		
	}

	int co=0;
	for(c=2048;c<2100;c++)
	{
		WaveTable[1][c]=co-16384;
		co+=16;
	}

}
