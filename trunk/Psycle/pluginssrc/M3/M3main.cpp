// M3 Buzz plugin by MAKK makk@gmx.de
// released on 04-21-99
// Thanks must go to Robert Bristow-Johnson pbjrbj@viconet.com
// a.k.a. robert@audioheads.com for his excellent
// Cookbook formulas for the filters.
// The code is not really speed optimized
// and compiles with many warnings - i'm to lazy to correct
// them all (mostly typecasts).
// Use the source for your own plugins if you want, but don't
// rip the hole machine please.
// Thanks in advance. MAKK

// This Source is Modified by [JAZ] ( jaz@pastnotecut.org ) to
// accomodate the Plugin to Psycle.
// Original Author sources are under:
// http://www.fortunecity.com/skyscraper/rsi/76/plugins.htm

#include "..\..\machineinterface.h"


float freqTab[120];
float coefsTab[4*128*128*8];
float LFOOscTab[0x10000];
signed short WaveTable[5][2100];



#define MAX_PSYCLE_TRACKS	32   // This value defines the MAX_TRACKS of PSYCLE, not of the Plugin.
						 // Leave it like it is. Your Plugin NEEDS TO support it.
						 // (Or dinamically allocate them. Check JMDrum's Source for an example)

#define NUMPARAMETERS 34 // Change this number to the number of parameters of your machine
						 // Remember that Psycle ONLY have GLOBAL parameters.




CMachineParameter const paraWave1 =
{
	"Osc1Wav", "Oscillator 1 Waveform", 0, 5, MPF_STATE, 0
};
CMachineParameter const paraPulseWidth1 =
{
	"PulseWidth1", "Oscillator 1 Pulse Width", 0, 127, MPF_STATE, 0x40
};


CMachineParameter const paraWave2 =
{
	"Osc2Wav", "Oscillator 2 Waveform", 0, 5, MPF_STATE, 0
};
CMachineParameter const paraPulseWidth2 =
{
	"PulseWidth2", "Oscillator 2 Pulse Width", 0, 127, MPF_STATE, 0x40
};


CMachineParameter const paraDetuneSemi=
{
	"Semi Detune", "Semi Detune in Halfnotes", 0, 127, MPF_STATE, 0x40
};
CMachineParameter const paraDetuneFine=
{
	"Fine Detune", "Fine Detune", 0, 127, MPF_STATE, 0x40
};


CMachineParameter const paraSync =
{
	"Oscs Synced", "Sync: Osc2 synced by Osc1", 0, 1, MPF_STATE, 0
};


CMachineParameter const paraMixType =
{
	"MixType", "MixType", 0, 8, MPF_STATE, 0
};
CMachineParameter const paraMix =
{
	"Osc Mix", "Mix Osc1 <-> Osc2", 0, 127, MPF_STATE, 0x40
};


CMachineParameter const paraSubOscWave =
{
	"SubOscWav", "Sub Oscillator Waveform", 0, 4, MPF_STATE, 0
};
CMachineParameter const paraSubOscVol =
{
	"SubOscVol", "Sub Oscillator Volume", 0, 127, MPF_STATE, 0x40
};


CMachineParameter const paraPEGAttackTime =
{
	"Pitch Env Attack", "Pitch Envelope Attack Time", 0, 127, MPF_STATE, 0
};
CMachineParameter const paraPEGDecayTime =
{
	"Pitch Env Release", "Pitch Envelope Release Time", 0, 127, MPF_STATE, 0
};
CMachineParameter const paraPEnvMod =
{
	"Pitch Env Mod", "Pitch Envelope Modulation", 0, 127, MPF_STATE, 0x40
};


CMachineParameter const paraGlide =
{
	"Glide", "Glide", 0, 127, MPF_STATE, 0
};


CMachineParameter const paraVolume =
{
	"Volume", "Volume (Sustain-Level)", 0, 127, MPF_STATE, 0x40
};

CMachineParameter const paraAEGAttackTime =
{
	"Amp Env Attack", "Amplitude Envelope Attack Time (ms)", 0, 127, MPF_STATE, 10
};

CMachineParameter const paraAEGSustainTime =
{
	"Amp Env Sustain", "Amplitude Envelope Sustain Time (ms)", 0, 127, MPF_STATE, 50
};

CMachineParameter const paraAEGReleaseTime =
{
	"Amp Env Release", "Amplitude Envelope Release Time (ms)", 0, 127, MPF_STATE, 30
};

CMachineParameter const paraFilterType =
{
	"FilterType", "Filter Type ... 0=LP 1=HP 2=BP 3=BR", 0, 3, MPF_STATE, 0
};
CMachineParameter const paraCutoff =
{
	"Cutoff", "Filter Cutoff Frequency", 0, 127, MPF_STATE, 127
};
CMachineParameter const paraResonance =
{
	"Res./Bandw.", "Filter Resonance/Bandwidth", 0, 127, MPF_STATE, 32
};


CMachineParameter const paraFEGAttackTime =
{
	"Filter Env Attack", "Filter Envelope Attack Time", 0, 127, MPF_STATE, 0
};
CMachineParameter const paraFEGSustainTime =
{
	"Filter Env Sustain", "Filter Envelope Sustain Time", 0, 127, MPF_STATE, 0
};
CMachineParameter const paraFEGReleaseTime =
{
	"Filter Env Release", "Filter Envelope Release Time", 0, 127, MPF_STATE, 0
};
CMachineParameter const paraFEnvMod =
{
	"Filter Env Mod", "Filter Envelope Modulation", 0, 127, MPF_STATE, 0x40
};

// LFOs
CMachineParameter const paraLFO1Dest =
{
	"LFO1 Dest", "LFO1 Destination", 0, 15, MPF_STATE, 0
};
CMachineParameter const paraLFO1Wave =
{
	"LFO1 Wav", "LFO1 Waveform", 0, 4, MPF_STATE, 0
};
CMachineParameter const paraLFO1Freq =
{
	"LFO1 Freq", "LFO1 Frequency", 0, 127, MPF_STATE, 0
};
CMachineParameter const paraLFO1Amount =
{
	"LFO1 Amount", "LFO1 Amount", 0, 127, MPF_STATE, 0
};

// lfo2
CMachineParameter const paraLFO2Dest =
{
	"LFO2 Dest", "LFO2 Destination", 0, 15, MPF_STATE, 0
};
CMachineParameter const paraLFO2Wave =
{
	"LFO2 Wav", "LFO2 Waveform", 0, 4, MPF_STATE, 0
};
CMachineParameter const paraLFO2Freq =
{
	"LFO2 Freq", "LFO2 Frequency", 0, 127, MPF_STATE, 0
};
CMachineParameter const paraLFO2Amount =
{
	"LFO2 Amount", "LFO2 Amount", 0, 127, MPF_STATE, 0
};

CMachineParameter const *pParameters[] =
{
        &paraWave1,
        &paraPulseWidth1,
        &paraWave2,
        &paraPulseWidth2,
        &paraDetuneSemi,
        &paraDetuneFine,
        &paraSync,
        &paraMixType,
        &paraMix,
        &paraSubOscWave,
        &paraSubOscVol,
        &paraPEGAttackTime,
        &paraPEGDecayTime,
        &paraPEnvMod,
        &paraGlide,

        &paraVolume,
        &paraAEGAttackTime,
        &paraAEGSustainTime,
        &paraAEGReleaseTime,

        &paraFilterType,
        &paraCutoff,
        &paraResonance,
        &paraFEGAttackTime,
        &paraFEGSustainTime,
        &paraFEGReleaseTime,
        &paraFEnvMod,

        // LFO 1
        &paraLFO1Dest,
        &paraLFO1Wave,
        &paraLFO1Freq,
        &paraLFO1Amount,
        // LFO 2
        &paraLFO2Dest,
        &paraLFO2Wave,
        &paraLFO2Freq,
        &paraLFO2Amount,
};


CMachineInfo const MacInfo =
{
        MI_VERSION,
        GENERATOR,                                                                              // flags
        NUMPARAMETERS,                                                                              // min tracks
        pParameters,
#ifdef _DEBUG
        "M3 by Makk (Debug build)",                     // name
#else
        "M3 by Makk",
#endif
        "M3",                                                                   // short name
        "Makk",                                                                 // author
        "About",
		5
};

//
// Here It goes the "mi" declaration. It has been moved to M3Track.h due to some compiling 
// requirements.
//
#include "M3Track.h"
#include <string.h>



DLL_EXPORTS		// To export DLL functions to host

mi::mi()
{
	Vals=new int[NUMPARAMETERS];
}

mi::~mi()
{
	delete Vals;

// Destroy dinamically allocated objects/memory here
}

void mi::Init()
{
        TabSizeDivSampleFreq = (float)(2048.0/pCB->GetSamplingRate());

        for( int i=0; i<MAX_SIMUL_TRACKS; i++)
        {
                Tracks[i].pmi = this;
                Tracks[i].Init();
        }

		// Generate Oscillator tables
		for(int c=0;c<2100;c++)
		{
			double sval=(double)c*0.00306796157577128245943617517898389;

						WaveTable[0][c]=int(sin(sval)*16384.0f);

			if (c<2048) WaveTable[1][c]=(c*16)-16384;
			else		WaveTable[1][c]=((c-2048)*16)-16384;

			if (c<1024)	WaveTable[2][c]=-16384;
			else		WaveTable[2][c]=16384;

			if (c<1024)	WaveTable[3][c]=(c*32)-16384;
			else		WaveTable[3][c]=16384-((c-1024)*32);
						WaveTable[4][c]=rand();

		}

        // generate frequencyTab
        double freq = 16.35; //c0 bis b9
        for( int j=0; j<10; j++)
        {
                for( int i=0; i<12; i++)
                {
                        freqTab[j*12+i] = (float)freq;
                        freq *= 1.05946309435929526; // *2^(1/12)
                }
        }
        // generate coefsTab
        for( int t=0; t<4; t++)
                for( int f=0; f<128; f++)
                        for( int r=0; r<128; r++)
                                ComputeCoefs( coefsTab+(t*128*128+f*128+r)*8, f, r, t);
        // generate LFOOscTab
        for( int p=0; p<0x10000; p++)
                LFOOscTab[p] = pow( 1.00004230724139582, p-0x8000);
}

void mi::Stop()
{
	for( int i=0; i<MAX_SIMUL_TRACKS; i++) Tracks[i].Stop();
}

// Called when a parameter is changed by the host app / user gui
void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;	// Don't remove this line. Psycle reads this variable when Showing the
					// Parameters' Dialog box.
	tvals tmp;

	SetNoValue(tmp);

	switch(par)
	{
	case 0:	tmp.Wave1 = val;		break;
	case 1: tmp.PulseWidth1 = val;	break;
	case 2:	tmp.Wave2 = val;		break;
	case 3: tmp.PulseWidth2 = val;	break;
	case 4: tmp.DetuneSemi = val;	break;
	case 5: tmp.DetuneFine = val;	break;
	case 6: tmp.Sync = val;			break;
	case 7: tmp.MixType = val;		break;
	case 8: tmp.Mix = val;			break;
	case 9: tmp.SubOscWave = val;	break;
	case 10: tmp.SubOscVol = val;	break;
	case 11: tmp.PEGAttackTime = val;	break;
	case 12: tmp.PEGDecayTime = val;	break;
	case 13: tmp.PEnvMod = val;		break;
	case 14: tmp.Glide = val;		break;
	case 15: tmp.Volume = val;		break;
    case 16: tmp.AEGAttackTime = val;	break;
	case 17: tmp.AEGSustainTime = val;	break;
	case 18: tmp.AEGReleaseTime = val;	break;
	case 19: tmp.FilterType = val;	break;
	case 20: tmp.Cutoff = val;		break;
	case 21: tmp.Resonance = val;	break;
	case 22: tmp.FEGAttackTime = val;	break;
	case 23: tmp.FEGSustainTime = val;	break;
	case 24: tmp.FEGReleaseTime = val;	break;
	case 25: tmp.FEnvMod = val;		break;
	case 26: tmp.LFO1Dest = val;	break;
	case 27: tmp.LFO1Wave = val;	break;
	case 28: tmp.LFO1Freq = val;	break;
	case 29: tmp.LFO1Amount = val;	break;
	case 30: tmp.LFO2Dest = val;	break;
	case 31: tmp.LFO2Wave = val;	break;
	case 32: tmp.LFO2Freq = val;	break;
	case 33: tmp.LFO2Amount = val;	break;
	}

	for (int i=0; i<MAX_SIMUL_TRACKS;i++)	// It is MUCH better to change the parameter to all
	{										// tracks than _reset_ the selected trach each note.
		Tracks[i].Tick(tmp);
	}

}

// Called each tick (i.e.when playing). Note: it goes after ParameterTweak and before SeqTick
void mi::SequencerTick()
{
	//	If you need this, add a command handler. You cannot use this one like you did in buzz
}

////////////////////////////////////////////////////////////////////////
// The SeqTick function is where your notes and pattern command handlers
// should be processed. Called each tick.
// It is called by the host sequencer
	
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	// Note Off			== 120
	// Empty Note Row	== 255
	// Less than note off == NoteON

	int useVoice=-1;
	tvals tmp;

	SetNoValue(tmp);

	if(note<120) // New Note entering.
	{
		for (int voice=0; voice<MAX_SIMUL_TRACKS; voice++)	// Find a voice to apply the new note
		{
			switch(Tracks[voice].AEGState)
			{
				case EGS_NONE: 
					if ( useVoice == -1 ) useVoice=voice;
					else if ( Tracks[useVoice]._channel != channel || !Tracks[useVoice].Glide )
					{
						useVoice=voice;
					}
					break;

				case EGS_RELEASE:
					if ( useVoice == -1 ) useVoice = voice;
					break;
			}
			if ( Tracks[voice]._channel == channel ) // Does exist a Previous note?
			{
				if ( !Tracks[voice].Glide )	// If no Glide , Note Off
				{	
					tmp.Note=120;
					Tracks[voice].Tick(tmp);
					Tracks[voice]._channel = -1;
					if ( useVoice == -1 ) { useVoice = voice; }
				}
				else useVoice = voice;	// Else, use it.
			}
		}
		if ( useVoice == -1 )	// No free voices. Assign one!
		{
			useVoice=0;
		}
		tmp.Note=note;
		Tracks[useVoice]._channel=channel;
		Tracks[useVoice].Tick(tmp);
	}
	else if ( note == 120 )
	{
		for (int voice=0;voice<MAX_SIMUL_TRACKS; voice++)  // Find the...
		{
			if (( Tracks[voice]._channel == channel ) && // ...playing voice on current channel.
				( Tracks[voice].AEGState != EGS_NONE ) &&
				( Tracks[voice].AEGState != EGS_RELEASE ))
			{
				tmp.Note=note;
				Tracks[voice].Tick(tmp);//  Handle Note Off
				Tracks[voice]._channel=-1;
				return;
			}
		}
	}
}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{
	bool gotSomething=false;
	for ( int i=0; i<MAX_SIMUL_TRACKS; i++)
	{
		if ( Tracks[i].AEGState)
		{
			Tracks[i].Work( psamplesleft, numsamples);
			gotSomething=true;
		}
    }
	if (gotSomething) for (int i=0;i<numsamples;i++) psamplesright[i]=psamplesleft[i];
}
void mi::Command()
{
	char txt[32];
	sprintf(txt,"Compiled on %s",__DATE__);
	pCB->MessBox(txt,"M3 Psycle Plugin",0);
}

// Function that describes the value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value) // Param is 0 based!
{
    switch(param){
        case 1: // PW1
        case 3: // PW2
                sprintf(txt, "%u : %u", (int)(value*100.0/127),
									100-(int)(value*100.0/127));
				return true;
                break;
        case 4: // semi detune
                if( value == 0x40)	 strcpy(txt,"�0 halfnotes");
                else if( value > 0x40) sprintf( txt, "+%i halfnotes", value-0x40);
                else			     sprintf( txt, "%i halfnotes", value-0x40);
				return true;
                break;
        case 5: // fine detune
                if( value == 0x40)	 strcpy(txt,"�0 cents");
                else if( value > 0x40) sprintf( txt, "+%i cents", (int)((value-0x40)*100.0/63));
                else				 sprintf( txt, "%i cents", (int)((value-0x40)*100.0/63));
				return true;
                break;

        case 6: // Sync
                if( value == 1) strcpy(txt,"on");
                else strcpy(txt,"off");
                return true;
                break;

        case 7: // MixType
                switch( value)
				{
					case 0: strcpy(txt,"add");break;
					case 1: strcpy(txt,"difference");break;
					case 2: strcpy(txt,"mul");break;
					case 3: strcpy(txt,"highest amp");break;
					case 4: strcpy(txt,"lowest amp");break;
					case 5: strcpy(txt,"and");break;
					case 6: strcpy(txt,"or");break;
					case 7: strcpy(txt,"xor");break;
					case 8: strcpy(txt,"random");break;
					default: strcpy(txt,"Invalid!");
                }
				return true;
                break;
        case 8: // Mix
                switch( value) {
	                case 0: strcpy(txt,"Osc1");break;
		            case 127:strcpy(txt,"Osc2");break;
			        default: sprintf(txt, "%u%% : %u%%", 100-(int)(value*100.0/127),
						                                 (int)(value*100.0/127));
                }
				return true;
                break;

        case 11: // Pitch Env
        case 12: // Pitch Env
        case 16: // Amp Env
        case 17: // Amp Env
        case 18: // Amp Env
        case 22: // Filter Env
        case 23: // Filter Env
        case 24: // Filter Env
                sprintf( txt, "%.4f sec", EnvTime( value)/1000);
				return true;
                break;

        case 13: // PitchEnvMod
        case 25: // Filt ENvMod
                sprintf( txt, "%i", value-0x40);
				return true;
                break;
        case 19:
                switch( value) {
					case 0: strcpy(txt,"lowpass");break;
					case 1: strcpy(txt,"highpass");break;
					case 2: strcpy(txt,"bandpass");break;
					case 3: strcpy(txt,"bandreject");break;
					default: strcpy(txt,"Invalid!");
                }
				return true;
                break;
        case 26: // LFO1Dest
                switch( value) {
				case 0: strcpy(txt,"none");break;
					case 1: strcpy(txt,"osc1");break;
					case 2: strcpy(txt,"p.width1");break;
					case 3: strcpy(txt,"volume");break;
					case 4: strcpy(txt,"cutoff");break;
					case 5: strcpy(txt,"osc1+pw1");break; // 12
					case 6: strcpy(txt,"osc1+volume");break; // 13
					case 7: strcpy(txt,"osc1+cutoff");break; // 14
					case 8: strcpy(txt,"pw1+volume");break; // 23
					case 9: strcpy(txt,"pw1+cutoff");break; // 24
					case 10: strcpy(txt,"vol+cutoff");break; // 34
					case 11: strcpy(txt,"o1+pw1+vol");break;// 123
					case 12: strcpy(txt,"o1+pw1+cut");break;// 124
					case 13: strcpy(txt,"o1+vol+cut");break;// 134
					case 14: strcpy(txt,"pw1+vol+cut");break;// 234
					case 15: strcpy(txt,"all");break;// 1234
					default: strcpy(txt,"Invalid!");
                }
				return true;
                break;
        case 30: // LFO2Dest
                switch( value) {
					case 0: strcpy(txt,"none");break;
					case 1: strcpy(txt,"osc2");break;
					case 2: strcpy(txt,"p.width2");break;
					case 3: strcpy(txt,"mix");break;
					case 4: strcpy(txt,"resonance");break;

					case 5: strcpy(txt,"osc2+pw2");break; // 12
					case 6: strcpy(txt,"osc2+mix");break; // 13
					case 7: strcpy(txt,"osc2+res");break; // 14
					case 8: strcpy(txt,"pw2+mix");break; // 23
					case 9: strcpy(txt,"pw2+res");break; // 24
					case 10: strcpy(txt,"mix+res");break; // 34

					case 11: strcpy(txt,"o2+pw2+mix");break; // 123
					case 12: strcpy(txt,"o2+pw2+res");break; // 124
					case 13: strcpy(txt,"o2+mix+res");break; // 134
					case 14: strcpy(txt,"pw2+mix+res");break; // 234
					case 15: strcpy(txt,"all");break; // 1234
					default: strcpy(txt,"Invalid!");
                }
				return true;
                break;
        case 9: // SubOscWave
				if( value == 4) { strcpy(txt,"random"); return true; break; }
        case 0: // OSC1Wave
        case 2: // OSC2Wave
        case 27: // LFO1Wave
        case 31: // LFO2Wave
                switch( value) {
					case 0: strcpy(txt,"sine");break;
					case 1: strcpy(txt,"saw");break;
					case 2: strcpy(txt,"square");break;
					case 3: strcpy(txt,"triangle");break;
					case 4: strcpy(txt,"noise");break;
					case 5: strcpy(txt,"random");break;
					case 6: strcpy(txt,"Invalid!");break;
                }
				return true;
                break;
        case 28: // LFO1Freq
        case 32: // LFO2Freq
                if( value <= 116)
                        sprintf( txt, "%.4f HZ", LFOFreq( value));
                else
                        sprintf( txt, "%u ticks", 1<<(value-117));
				return true;
                break;
    }
	return false;
}

void mi::SetNoValue(tvals &tv)
{
        tv.Note=0xff;
        tv.Wave1=0xff;
        tv.PulseWidth1=0xff;
        tv.Wave2=0xff;
        tv.PulseWidth2=0xff;
        tv.DetuneSemi=0xff;
        tv.DetuneFine=0xff;
        tv.Sync=0xff;
        tv.MixType=0xff;
        tv.Mix=0xff;
        tv.SubOscWave=0xff;
        tv.SubOscVol=0xff;
        tv.PEGAttackTime=0xff;
        tv.PEGDecayTime=0xff;
        tv.PEnvMod=0xff;
        tv.Glide=0xff;

        tv.Volume=0xff;
        tv.AEGAttackTime=0xff;
        tv.AEGSustainTime=0xff;
        tv.AEGReleaseTime=0xff;

        tv.FilterType=0xff;
        tv.Cutoff=0xff;
        tv.Resonance=0xff;
        tv.FEGAttackTime=0xff;
        tv.FEGSustainTime=0xff;
        tv.FEGReleaseTime=0xff;
        tv.FEnvMod=0xff;

        tv.LFO1Dest=0xff;
        tv.LFO1Wave=0xff;
        tv.LFO1Freq=0xff;
        tv.LFO1Amount=0xff;
        tv.LFO2Dest=0xff;
        tv.LFO2Wave=0xff;
        tv.LFO2Freq=0xff;
        tv.LFO2Amount=0xff;

}



void mi::ComputeCoefs( float *coefs, int freq, int r, int t)
{

    float omega = 2*PI*Cutoff(freq)/pCB->GetSamplingRate();
    float sn = sin( omega);
    float cs = cos( omega);
    float alpha;
        if( t<2)
                alpha = sn / Resonance( r *(freq+70)/(127.0+70));
        else
                alpha = sn * sinh( Bandwidth( r) * omega/sn);

        float a0, a1, a2, b0, b1, b2;

        switch( t) {
        case 0: // LP
                b0 =  (1 - cs)/2;
                b1 =   1 - cs;
                b2 =  (1 - cs)/2;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 1: // HP
                b0 =  (1 + cs)/2;
                b1 = -(1 + cs);
                b2 =  (1 + cs)/2;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 2: // BP
                b0 =   alpha;
                b1 =   0;
                b2 =  -alpha;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 3: // BR
                b0 =   1;
                b1 =  -2*cs;
                b2 =   1;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        }

        coefs[0] = b0/a0;
        coefs[1] = b1/a0;
        coefs[2] = b2/a0;
        coefs[3] = -a1/a0;
        coefs[4] = -a2/a0;
}
