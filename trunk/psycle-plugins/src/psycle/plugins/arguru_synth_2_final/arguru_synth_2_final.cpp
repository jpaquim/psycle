#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include "SynthTrack.hpp"
#include <cstdlib>

int const MAX_ENV_TIME = 250000;
int const NUMPARAMETERS = 28;

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
	19,												// MaxValue
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

CMachineParameter const paraGlobalDetune = 
{
	"Glb. Detune",
	"Global Detune",									// description
	-36,											// MinValue	
	36,											// MaxValue
	MPF_STATE,										// Flags
	1 
};
// Why is the default tuning +1 +60? answer:
// the wavetable contains one period of 2048 samples,
// at 44100Hz (which Asynth assumes), this is a ~21.5Hz signal.
// A standard tune is A-5 (i.e, note 69) to be 440Hz, which is
// ~ 20.4335 times bigger. log2 of this value is ~ 4.3528.
// Multiply this value by 12 (notes/octave) to get 52.2344,
// which stands for note 52 and finetune 0.2344.
// The value of note in SeqTick() is note-18 ( 69-18 = 51 ),
// So we add one to Tune, and fine is 0.2344 * 256 ~ 60.01

CMachineParameter const paraGlobalFinetune = 
{
	"Gbl. Finetune",
	"Global Finetune",									// description
	-256,											// MinValue	
	256,											// MaxValue
	MPF_STATE,										// Flags
	60
};

CMachineParameter const paraGlide = 
{
	"Glide Depth",
	"Glide Depth",									// description
	0,											// MinValue	
	256,											// MaxValue
	MPF_STATE,										// Flags
	0
};

CMachineParameter const paraInterpolation = 
{
	"Interpolation",
	"I",									// description
	0,											// MinValue	
	1,											// MaxValue
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
	&paraGlobalDetune,
	&paraGlobalFinetune,
	&paraGlide,
	&paraInterpolation
};


CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	GENERATOR,								// flags
	NUMPARAMETERS,							// numParameters
	pParameters,							// Pointer to parameters
#if !defined NDEBUG
	"Arguru Synth 2 final (debug build)",		// name
#else
	"Arguru Synth 2 final",						// name
#endif
	"Arguru Synth",							// short name
	"J. Arguelles (arguru)",							// author
	"help",									// A command, that could be use for open an editor, etc...
	4
};

class mi : public CMachineInterface
{
public:
	void InitWaveTable();
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

	signed short WaveTable[5][2050];
	CSynthTrack track[MAX_TRACKS];
	
	SYNPAR globalpar;

};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	Vals=new int[NUMPARAMETERS];
	InitWaveTable();
}

mi::~mi()
{
	delete[] Vals;

// Destroy dinamically allocated objects/memory here
}

void mi::Init()
{
// Initialize your stuff here
}

void mi::Stop()
{
	for(int c=0;c<MAX_TRACKS;c++)
	track[c].NoteOff(true);
}

void mi::SequencerTick()
{
	for (int i=0; i < MAX_TRACKS; i++)
	{
		track[i].InitEffect(0,0);
	}
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
	globalpar.globaldetune=Vals[24];
	globalpar.globalfinetune=Vals[25];
	globalpar.synthglide=Vals[26];
	globalpar.interpolate=Vals[27];
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
		"\n01xx : Pitch slide-up",
		"\n02xx : Pitch slide-down",
		"\n03xx : Pitch glide",
		"\n04xy : Vibrato [x=depth, y=speed]",
		"\n07xx : Change vcf env modulation [$00=-128, $80=0, $FF=+128]",
		"\n08xx : Change vcf cutoff frequency",
		"\n09xx : Change vcf resonance amount",
		"\n11xx : Vcf cutoff slide-up",
		"\n12xx : Vcf cutoff slide-down\0"
		);

pCB->MessBox(buffer,"�-=<([aRgUrU's SYNTH 2 (Final)])>=-�",0);

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{
	float sl=0;
	// not used: float sr=0;

	for(int c=0;c<tracks;c++)
	{
		if(track[c].AmpEnvStage)
		{

			float *xpsamplesleft=psamplesleft;
			float *xpsamplesright=psamplesright;
			--xpsamplesleft;
			--xpsamplesright;
			
			int xnumsamples=numsamples;
		
			CSynthTrack *ptrack=&track[c];

			if(ptrack->NoteCut) ptrack->NoteCutTime-=numsamples;
		
			ptrack->PerformFx();

			if ( globalpar.osc_mix == 0 )
			{	do
				{
					sl=ptrack->GetSampleOsc1();
					*++xpsamplesleft+=sl;
					*++xpsamplesright+=sl;
				} while(--xnumsamples);
			}
			else if ( globalpar.osc_mix == 256 )
			{	do
				{
					sl=ptrack->GetSampleOsc2();
					*++xpsamplesleft+=sl;
					*++xpsamplesright+=sl;
				} while(--xnumsamples);
			}
			else
			{	do
				{
					sl=ptrack->GetSample();
					*++xpsamplesleft+=sl;
					*++xpsamplesright+=sl;
				} while(--xnumsamples);
			}
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
		case 0:sprintf(txt,"Lowpass A");return true;break;
		case 1:sprintf(txt,"Hipass A");return true;break;
		case 2:sprintf(txt,"Bandpass A");return true;break;
		case 3:sprintf(txt,"Bandreject A");return true;break;
		case 4:sprintf(txt,"ParaEQ1 A");return true;break;
		case 5:sprintf(txt,"InvParaEQ1 A");return true;break;
		case 6:sprintf(txt,"ParaEQ2 A");return true;break;
		case 7:sprintf(txt,"InvParaEQ2 A");return true;break;
		case 8:sprintf(txt,"ParaEQ3 A");return true;break;
		case 9:sprintf(txt,"InvParaEQ3 A");return true;break;
		case 10:sprintf(txt,"Lowpass B");return true;break;
		case 11:sprintf(txt,"Hipass B");return true;break;
		case 12:sprintf(txt,"Bandpass B");return true;break;
		case 13:sprintf(txt,"Bandreject B");return true;break;
		case 14:sprintf(txt,"ParaEQ1 B");return true;break;
		case 15:sprintf(txt,"InvParaEQ1 B");return true;break;
		case 16:sprintf(txt,"ParaEQ2 B");return true;break;
		case 17:sprintf(txt,"InvParaEQ2 B");return true;break;
		case 18:sprintf(txt,"ParaEQ3 B");return true;break;
		case 19:sprintf(txt,"InvParaEQ3 B");return true;break;
		}
	}

	if(param==19)
	{
		float fv=(float)value*0.390625f;

		if ( value == 0 ) sprintf(txt,"Osc1");
		else if ( value == 256 ) sprintf(txt,"Osc2");
		else sprintf(txt,"%.1f%% : %.1f%%",100-fv,fv);

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
	if ((param==26) && ( value == 0 ))
	{
		sprintf(txt,"Off");
		return true;
	}
	if (param==27)
	{
		sprintf(txt,value?"On":"Off");
		return true;
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
	case 7: // Change envmod
		globalpar.vcf_envmod=val-128;
	break;
	
	case 8: // Change cutoff
		globalpar.vcf_cutoff=val/2;
	break;
	
	case 9: // Change reso
		globalpar.vcf_resonance=val/2;
	break;
	
	}

	// Note Off			== 120
	// Empty Note Row	== 255
	// Less than note off value??? == NoteON!
	if(note<120)
	track[channel].NoteOn(note-18,&globalpar,60);

	// Note off
	if(note==120)
	track[channel].NoteOff();
}

void mi::InitWaveTable()
{
	for(int c=0;c<2050;c++)
	{
		double sval=(double)c*0.00306796157577128245943617517898389;

					WaveTable[0][c]=int(sin(sval)*16384.0f);

		if (c<2048) WaveTable[1][c]=(c*16)-16384;
		else		WaveTable[1][c]=((c-2048)*16)-16384;

		if (c<1024 || c>=2048)	WaveTable[2][c]=-16384;
		else		WaveTable[2][c]=16384;

		if (c<1024)	WaveTable[3][c]=(c*32)-16384;
		else if (c<2048) WaveTable[3][c]=16384-((c-1024)*32);
		else		WaveTable[3][c]=((c-2048)*32)-16384;

					WaveTable[4][c]=rand()-16384;
	}
}
