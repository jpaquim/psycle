// -*- mode:c++; indent-tabs-mode:t -*-
#include <psycle/plugin_interface.hpp>
#include "envelope.hpp"
#include <math.h> // should be <cmath>

//////////////////////////////////////////////////////////////////////
// Psycle
//////////////////////////////////////////////////////////////////////
//
// FM Synthesizer plugin v1.0
//
//////////////////////////////////////////////////////////////////////
//
// Original code by Zephod (Buzz SuperFM)
//
// Code ReMixed by Arguru
//
// History
// -------
//
// V1.0 (2-Jun-2000) Code: Arguru
// ------------------------------
// * General code cleanup
// * 'Dsplib' not used anymore
// * 'smooth' class not used anymore
// * Adapted for psycle machine interface
// * Removed that ugly GetAuxBuffer stuff agjhhh!
// * 'Filter' removed. [Filters + FM, mmm...]
// * Fixed noteoff handling [performs a smooth release]
// * Optimized "envelope" class
// * Two envelopes, one for AMP (VCA) and ENV for FM modulators
// * Bugfixes, speedups
// * Envelope class: fixed, faster and accurate and clickfixed.
// * Smooth eliminated, useless on FM.New smooth envelope class used.
// * FM R000lz now! This sounds like my favourite console [Genesis]!
//
// Vxxx (x-xxx-1999) Code: Zephod
// ------------------------------
// * Original Buzz SuperFM release
//
//////////////////////////////////////////////////////////////////////

#define WF_SET								0
#define WF_ADD								1

#define ENV_ATT 1
#define ENV_DEC 2
#define ENV_SUS 3
#define ENV_REL 4
#define ENV_ANTICLICK 5
#define ENV_NONE 99

CMachineParameter const paraWave =
{
	"OSC Wave",   
	"Oscillator waveform",   
	0,                    
	3,                  
	MPF_STATE,            
	0                      
};

CMachineParameter const paraModWave =
{
	"MOD Wave",   
	"Modulator waveform",   
	0,                    
	3,                  
	MPF_STATE,            
	0                      
};

CMachineParameter const paraAttack =
{
	"VCA Attack",            
	"VCA Attack in smp",   
	16,                 
	65535,             
	MPF_STATE,         
	29               
};

CMachineParameter const paraDecay =
{
	"VCA Decay",          
	"VCA Decay in smp",  
	16,        
	65535,    
	MPF_STATE,
	46974      
};

CMachineParameter const paraSustain =
{
	"VCA S Length",         
	"VCA Sustain lenght in smp",   
	16,                     
	65536,                 
	MPF_STATE,             
	944                      
};

CMachineParameter const paraSustainv =
{
	"VCA S Level",
	"VCA Sustain level",  
	1,               
	256,             
	MPF_STATE,       
	13            
};

CMachineParameter const paraRelease =
{
	"VCA Release",          
	"VCA Release in smp",   
	1,                     
	65536,                 
	MPF_STATE,             
	2414                      
};

//////////////////////////////////////////////////////////////////////
// Modulators Enveloper

CMachineParameter const paraMAttack =
{
	"MOD ENV A",            
	"MOD Envelope attack in smp",   
	16,                 
	65535,             
	MPF_STATE,         
	16               
};

CMachineParameter const paraMDecay =
{
	"MOD ENV D",          
	"MOD Envelope decay in smp",  
	16,        
	65535,    
	MPF_STATE,
	15717      
};

CMachineParameter const paraMSustain =
{
	"MOD ENV SL",         
	"MOD Envelope sustain lenght in smp",   
	16,                     
	65536,                 
	MPF_STATE,             
	10967                      
};

CMachineParameter const paraMSustainv =
{
	"MOD ENV S",
	"MOD Envelope sustain level",  
	1,               
	256,             
	MPF_STATE,       
	8          
};

CMachineParameter const paraMRelease =
{
	"MOD ENV R",          
	"MOD Envelope release in smp",   
	1,                     
	65536,                 
	MPF_STATE,             
	3714                    
};

CMachineParameter const paraModNote1D = 
{ 
	"MOD1 Depth",                
	"Modulator 1 depth",                
	0,              
	1000,              
	MPF_STATE,             
	150                   
};
CMachineParameter const paraModNote2D = 
{ 
	"MOD2 Depth",                
	"Modulator 2 depth",                
	0,              
	1000,              
	MPF_STATE,             
	0                      
};
CMachineParameter const paraModNote3D = 
{ 
	"MOD3 Depth",                
	"Modulator 3 depth",                
	0,              
	1000,              
	MPF_STATE,             
	51                     
};

CMachineParameter const paraModEnv1 = 
{ 
	"MOD1 ENV",                
	"Modulator 1 envelope amount",                
	-256,              
	256,              
	MPF_STATE,             
	256                      
};

CMachineParameter const paraModEnv2 = 
{ 
	"MOD2 ENV",                
	"Modulator 2 envelope amount",                
	-256,              
	256,              
	MPF_STATE,           
	94                    
};


CMachineParameter const paraModEnv3 = 
{ 
	"MOD3 ENV",                
	"Modulator 3 envelope amount",                
	-256,              
	256,              
	MPF_STATE,           
	110                     
};

CMachineParameter const paraRoute =
{
	"FM Rout Mode",
	"Modulator Routing Mode",                  // description
	0,                                                                                              // Min
	3,                                                                              // Max
	MPF_STATE,                                                                              // Flags
	0                                                                                               // default
};

CMachineParameter const paraTune =
{
	"Finetune",
	"Pitch finetune",                  // description
	-128,                                                                                              // Min
	128,                                                                              // Max
	MPF_STATE,                                                                              // Flags
	0                                                                                               // default
};

CMachineParameter const *pParameters[] =
{
	&paraAttack,
	&paraDecay,
	&paraSustain,
	&paraSustainv,
	&paraRelease,
	&paraMAttack,
	&paraMDecay,
	&paraMSustain,
	&paraMSustainv,
	&paraMRelease,
	&paraModNote1D,
	&paraModNote2D,
	&paraModNote3D,
	&paraModEnv1,
	&paraModEnv2,
	&paraModEnv3,
	&paraWave,
	&paraModWave,
	&paraRoute,
	&paraTune
};

class mi;

class CTrack
{
	public:
			CTrack();
		void Tick(int note, int volume);
		void Stop();
		void Init();
		void clean();
		void Work(float *psamplesleft, float *psamplesright , int numsamples);

		void Generate(float *psamplesleft, float *psamplesright,int numsamples);
		inline float Filter( float x);
		float difference, target_vol,advance,current_vol;
		
		inline float Osc(float phi);
		inline float ModOsc(float phi);

			mi *pmi; 

		float Volume;
		int Note;
		float freq,phase;
		float oldout;

		envelope VCA;
		envelope ENV;

		float Mot1dv,Mot2dv,Mot3dv;
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	GENERATOR,																																// flags
	20,																																								// numParameters
	pParameters,																												// Pointer to parameters
#ifdef _DEBUG
	"Zephod SuperFM (Debug build)",												// name
#else
	"Zephod SuperFM (Arguru Remix)",								// name
#endif
	"SuperFM",																																// short name
	"Zephod / Arguru",																								// author
	"Help",																																				// A command, that could be use for open an editor, etc...
	2
};

class mi : public CMachineInterface
{
	public:
		mi();
		virtual ~mi();
		virtual void Init();
		virtual void ParameterTweak(int par, int val);
		virtual void SequencerTick();
		virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
		virtual void Work(float *psamplesleft, float *psamplesright , int numsamples,int numtracks);
		virtual void DSPClear(float *psamplesleft,float *psamplesright, int numsamples);
		virtual void Stop();

		uint8 wave,ModWave;
		CTrack Tracks[MAX_TRACKS];
		int medBreakNote[MAX_TRACKS];
		uint8 route;
		int mod1_env,mod2_env,mod3_env;
		int tickCounter;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

void CTrack::Stop()
{
	VCA.stop();
	ENV.stop();
}

CTrack::CTrack()
{
		difference=0.0f;
		target_vol=0.0f;
		advance=0.0f;
		current_vol=0.0f;
		
		Volume=0.0f;
		Note=0;
		freq=0.0f;
		phase=0.0f;
		oldout=0.0f;

		Mot1dv=0.0f;
		Mot2dv=0.0f;
		Mot3dv=0.0f;

}

void CTrack::Init()
{
	Mot1dv=0;
	Mot2dv=0;
	Mot3dv=0;

	Volume=0.0f;
	VCA.reset();
	VCA.attack(1000);
	VCA.decay(1000);
	VCA.sustain(1000);
	VCA.sustainv(0.5);
	VCA.release(1000);

	ENV.reset();
	ENV.attack(1000);
	ENV.decay(1000);
	ENV.sustain(1000);
	ENV.sustainv(0.5);
	ENV.release(1000);
	
	phase=0;
	freq=0;
}


void CTrack::clean(void)
{
}

void CTrack::Tick(int note, int volume)
{
		int SamplesPerSec=44100;

		if( note <120)
		{
			Note = note - 70;
			float Detune = (float)pmi->Vals[19]*0.0078125f;
			freq = (float) (440.0f*pow(2.0f, ((float)Note+Detune)/12.0f))/float(SamplesPerSec);
			VCA.reset();
			ENV.reset();
	
		}
		else if(note == 120)
		{
			ENV.noteoff();
			VCA.noteoff();
		}

		// Volume [note speed handler]

		if(volume!=255)
		Volume=float(volume)*0.000976562f;
}

float CTrack::Osc(float phi)
{
	switch(pmi->wave)
	{
		case 0:				return (float) sin(phi*2*psycle::plugin_interface::pi)*32768 ;break;
		case 1:				if (sin(phi*2*psycle::plugin_interface::pi)>0) return 32768 ; else return -32768 ;break;
		case 2: return (float) (phi-0.5)* 32768 ;break;
		case 3: return (float) (0.5-phi)* 32768 ;break;
		default:return 0;break;
	}
}

float CTrack::ModOsc(float phi)
{
	switch(pmi->ModWave)
	{
		case 0: return (float) sin(phi*2*psycle::plugin_interface::pi);
		case 1:				if (sin(phi*2*psycle::plugin_interface::pi)>0) return 1; else return -1;break;
		case 2: return (float) (phi-0.5);break;
		case 3: return (float) (0.5-phi);break;
		default:return 0;
	}
}

void CTrack::Generate(float *psamplesleft, float *psamplesright, int numsamples)
{
	float cphase,bphase,dphase,temp;
	float Mot1D,Mot2D,Mot3D;

	float const Mod1ea=(float)pmi->mod1_env*0.0019531f;
	float const Mod2ea=(float)pmi->mod2_env*0.0019531f;
	float const Mod3ea=(float)pmi->mod3_env*0.0019531f;
	
	
	--psamplesleft;
	--psamplesright;
			
	for(int i=0; i<numsamples; i++) 
		{
			Mot1D=Mot1dv+ENV.res()*Mod1ea;
			Mot2D=Mot2dv+ENV.envvol*Mod2ea;
			Mot3D=Mot3dv+ENV.envvol*Mod3ea;

			switch (pmi->route)
			{
			case 0:
					if (Mot3D>0) dphase=phase+ModOsc(phase)*Mot3D;else dphase=phase;
					if (Mot2D>0) cphase=phase+ModOsc(dphase)*Mot2D;else cphase=phase;
					if (Mot1D>0) bphase=phase+ModOsc(cphase)*Mot1D;else bphase=phase;
			break;

			case 1:
					if (Mot3D>0) cphase=phase+ModOsc(phase)*Mot3D;else cphase=phase;
					if (Mot2D>0) cphase=cphase+ModOsc(phase)*Mot2D;
					if (Mot1D>0) bphase=phase+ModOsc(cphase)*Mot1D;else bphase=phase;
			break;
			case 2:
					if (Mot3D>0) bphase=phase+ModOsc(phase)*Mot3D;else bphase=phase;
					if (Mot2D>0) bphase=bphase+ModOsc(phase)*Mot2D;
					if (Mot1D>0) bphase=bphase+ModOsc(phase)*Mot1D;
			break;
			case 3:
					if (Mot3D>0) cphase=phase+ModOsc(phase)*Mot3D;else cphase=phase;
					if (Mot2D>0) bphase=phase+ModOsc(cphase)*Mot2D;else bphase=phase;
					if (Mot1D>0) bphase=bphase+ModOsc(phase)*Mot1D;
			break;
			
			default:
				bphase=phase;
			break;
			}

			if (VCA.envstate!=ENV_NONE)
			temp = Osc(bphase) * VCA.res()*Volume;
			else
			temp=0;

			// Adding to buffer
			++psamplesleft;
			*psamplesleft = *psamplesleft+temp;
			++psamplesright;
			*psamplesright = *psamplesright+temp;

			// New phases
			phase+=freq;
			while (phase >= 1.0f) phase -= 1.0f;

		}
}				

void CTrack::Work( float *psamplesleft,float *psamplesright, int numsamples)
{
	if (VCA.envstate!=ENV_NONE)
	Generate(psamplesleft, psamplesright, numsamples);

}

//////////////////////////////////////////////////////////////////////
//
// Machine Interface implementation
//
//////////////////////////////////////////////////////////////////////

mi::mi()
{
	Vals = new int[20];
	tickCounter=0;
}

mi::~mi()
{
	delete Vals;

	for (int c = 0; c < MAX_TRACKS; c++)
	{
		Tracks[c].clean();
	}
}

void mi::Init()
{

	for (int c = 0; c < MAX_TRACKS; c++)
	{
		Tracks[c].pmi = this;
		Tracks[c].Stop();
	}
	for (int c = 0; c < MAX_TRACKS; c++)
	{
		medBreakNote[c]=0;
		Tracks[c].Init();
	}

}

void mi::ParameterTweak(int par, int val)
{
		int i;

		Vals[par]=val;

		if(par==0) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].VCA.attack(val);
		if(par==1) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].VCA.decay(val);
		if(par==2) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].VCA.sustain(val);
		if(par==3) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].VCA.sustainv((float)val/255);
		if(par==4) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].VCA.release(val);
		
		if(par==5) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].ENV.attack(val);
		if(par==6) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].ENV.decay(val);
		if(par==7) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].ENV.sustain(val);
		if(par==8) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].ENV.sustainv((float)val/255);
		if(par==9) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].ENV.release(val);
		
		if(par==10) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].Mot1dv=(float)val/500;
		if(par==11) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].Mot2dv=(float)val/500;
		if(par==12) for (i=0; i<MAX_TRACKS ; i++) Tracks[i].Mot3dv=(float)val/500;
		
		if(par==13)				mod1_env=val;
		if(par==14)				mod2_env=val;
		if(par==15)				mod3_env=val;
		
		if(par==16)				wave=val;
		if(par==17) ModWave=val;

		if(par==18)
		route = val;

}

// Called once when Tick
void mi::SequencerTick() 
{
tickCounter=0;
}

void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
		int vol=254;
		medBreakNote[channel]=0;

		if(cmd==0x0C)
		{
			vol=val-2;
			if(vol<0)vol=0;
		}

		if(cmd==0x0E)
		medBreakNote[channel]=val*32;
		
		Tracks[channel].Tick(note,vol);
	

}

void mi::DSPClear(float *psamplesleft,float *psamplesright, int numsamples)
{
	do
	{
	*psamplesleft=0;
	*psamplesright=0;
	psamplesleft++;
	psamplesright++;
	}while(--numsamples);
}

void mi::Work(float *psamplesleft,float *psamplesright, int numsamples,int numtracks)
{
	//bool gotsomething = false;
	
	DSPClear(psamplesleft,psamplesright,numsamples);

	tickCounter+=numsamples;
	
	for ( int i=0; i<numtracks; i++) 
	{
		if ( Tracks[i].VCA.envstate!=ENV_NONE)
		{
		Tracks[i].Work( psamplesleft,psamplesright, numsamples);

			// Note off check [0x0E] command
			if(medBreakNote[i])
			{
				if(tickCounter>medBreakNote[i])
				{
					Tracks[i].VCA.noteoff();
					Tracks[i].ENV.noteoff();
				}
			}
		
		}
	}
}

void mi::Stop()
{
		for( int i=0; i<MAX_TRACKS; i++) Tracks[i].Stop();
}

/*
char const *mi::DescribeValue(int const param, int const value)
{
	static char txt[16];

	switch(param)
	{
	case 9:				
		switch(value)
		{
			case 0:sprintf(txt, "Sine");break;
			case 1:sprintf(txt, "Pulse");break;
			case 2:sprintf(txt, "Saw");break;
			case 3:sprintf(txt, "-Saw");break;
		}

		break;
	case 10:				
		switch(value)
		{
			case 0:sprintf(txt, "Sine");break;
			case 1:sprintf(txt, "Pulse");break;
			case 2:sprintf(txt, "Saw");break;
			case 3:sprintf(txt, "-Saw");break;
		}
		break;
	case 13:				
		switch(value)
		{
			case 0:sprintf(txt, "Lowpass");break;
			case 1:sprintf(txt, "Highpass");break;
			case 2:sprintf(txt, "Bandpass");break;
			case 3:sprintf(txt, "Bandreject");break;
		}
		break;
	case 14:				
		switch(value)
		{
			case 0:sprintf(txt, "O(M(M(M)))");break;
			case 1:sprintf(txt, "O(M(M+M))");break;
			case 2:sprintf(txt, "O(M+M+M)");break;
			case 3:sprintf(txt, "O(M+M(M))");break;
		}

		break;
	default:
		return NULL;
	}

	return txt;
}
*/
