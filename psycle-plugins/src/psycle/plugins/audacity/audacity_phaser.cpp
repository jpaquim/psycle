// -*- mode:c++; indent-tabs-mode:t -*-
//////////////////////////////////////////////////////////////////////
// AudaCity Phaser effect plugin for PSYCLE by Sartorius
//
//   Original
/**********************************************************************

	Audacity: A Digital Audio Editor

	Phaser.cpp

	Effect programming:
	Nasca Octavian Paul

	UI programming:
	Dominic Mazzoni (with the help of wxDesigner)
	Vaughan Johnson (Preview)

**********************************************************************/
#include <psycle/plugin_interface.hpp>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define MAX_STAGES 24
#define phaserlfoshape 4.0f
// How many samples are processed before compute the lfo value again
#define lfoskipsamples 20
#define M_PI 3.14159265359f
#define NUMPARAMETERS 6

CMachineParameter const paraLFOFreq = 
{ 
	"LFO Freq",
	"LFOFreq",																								// description
	1,																																																// MinValue				
	40,																																												// MaxValue
	MPF_STATE,																																								// Flags
	4,
};

CMachineParameter const paraLFOStartPhase = 
{ 
	"LFO start phase",
	"LFOStartPhase",																								// description
	0,																																																// MinValue				
	359,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0,
};

CMachineParameter const paraDepth = 
{ 
	"Depth",
	"Depth",																								// description
	0,																																																// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	70,
};

CMachineParameter const paraStages = 
{ 
	"Stages",
	"Stages",																																												// description
	2,																																																// MinValue				
	24,																																												// MaxValue
	MPF_STATE,																																								// Flags
	2,
};

CMachineParameter const paraDryWet = 
{ 
	"Dry/Wet",
	"Dry/Wet",																																												// description
	0,																																																// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	50,
};

CMachineParameter const paraFB = 
{ 
	"Feedback",
	"Feedback",																																												// description
	-100,																																																// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraLFOFreq,
	&paraLFOStartPhase,
	&paraDepth,
	&paraStages,
	&paraDryWet,
	&paraFB
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	EFFECT,																																								// flags
	NUMPARAMETERS,																																// numParameters
	pParameters,																																// Pointer to parameters
#ifdef _DEBUG
	"Audacity Phaser (Debug build)",								// name
#else
	"Audacity Phaser",																								// name
#endif
	"APhaser",																												// short name
	"Nasca Octavian Paul/Sartorius",																												// author
	"About",																																// A command, that could be use for open an editor, etc...
	1
};


class mi : public CMachineInterface
{
public:

	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);

private:
	float freq;
	float startphase;
	float fb;
	float depth;
	int stages;
	float drywet;

	// state variables
	unsigned long skipcount;
	float old_l[MAX_STAGES],old_r[MAX_STAGES]; // must be as large as MAX_STAGES
	float gain_l,gain_r;
	float fbout_l,fbout_r;
	float lfoskip;
	float phase_l,phase_r;

};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[NUMPARAMETERS];
}

mi::~mi()
{
	delete Vals;
}

void mi::Init()
{
// Initialize your stuff here
	freq = 0.4f;
	startphase = 0;
	depth = .7f;

	lfoskip = freq * 2 * M_PI / pCB->GetSamplingRate();
	skipcount = 0;
	gain_l = gain_r = 0;
	fbout_l = fbout_r = 0;

	phase_l = startphase; phase_r = phase_l + M_PI;

	for (int j = 0; j < MAX_STAGES; j++){
		old_l[j] = 0;
		old_r[j] = 0;
	}
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
pCB->MessBox("Audacity Phaser","APhaser",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
	switch(par)
	{
		case 0: freq = (float)val * .1f; lfoskip = freq * 2 * M_PI / pCB->GetSamplingRate(); break;
		case 1: startphase = float(val * M_PI / 180); phase_l = startphase; phase_r = phase_l + M_PI; break;
		case 2: depth = float(val * .01f);  break;
		case 3: stages = (val%2==1)?val-1:val; break;
		case 4: drywet = val * .01f; break;
		case 5: fb = val * .01f; break;
		default:
				break;
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param)
	{
		case 0:
			std::sprintf(txt,"%.1f Hz",(float)value*.1f);
			return true;
		case 1:
			std::sprintf(txt,"%i°",value);
			return true;
		case 2:
			std::sprintf(txt,"%i%%",value);
			return true;
		case 3:
			std::sprintf(txt,"%i",(value%2==1)?value-1:value);
			return true;
		case 4:
			std::sprintf(txt,"%i%%:%i%%",(100-value),value);
			return true;
		case 5:
			std::sprintf(txt,"%.2f",(float)value*0.01f);
			return true;
		default:
			return false;
	}
	//return false;
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	static float anti_denormal = 1.0e-20f;
	float m_l, m_r, tmp;

		do
			{
			float in_l = *psamplesleft;
			float in_r = *psamplesright;
			m_l = in_l + fbout_l * fb;
			m_r = in_r + fbout_r * fb;
			if ((skipcount++) % lfoskipsamples == 0) {
				//compute sine between 0 and 1
				gain_l = (1 + std::cos(skipcount * lfoskip + phase_l)) / 2;
				// change lfo shape
				gain_l =(std::exp(gain_l * phaserlfoshape) - 1) / (std::exp(phaserlfoshape)-1);
				gain_l = 1 - gain_l * depth;      // attenuate the lfo

				gain_r = (1 + std::cos(skipcount * lfoskip + phase_r)) / 2;
				// change lfo shape
				gain_r =(std::exp(gain_r * phaserlfoshape) - 1) / (std::exp(phaserlfoshape)-1);
				gain_r = 1 - gain_r * depth;      // attenuate the lfo
			}
			// phasing routine
			for (int j = 0; j < stages; j++) {
				tmp = old_l[j];
				old_l[j] = gain_l * tmp + m_l+anti_denormal;
				m_l = tmp - gain_l * old_l[j];
				tmp = old_r[j];
				old_r[j] = gain_r * tmp + m_r+anti_denormal;
				m_r = tmp - gain_r * old_r[j];
				anti_denormal=-anti_denormal;
			}
			fbout_l = m_l; fbout_r = m_r;
			*psamplesleft = m_l * drywet + in_l * (1 - drywet);
			*psamplesright = m_r * drywet + in_r * (1 - drywet);
		
			//if (out_l < -1.0) 
			//				out_l = float(-1.0);
			//else if (out_l > 1.0)
			//				out_l = float(1.0);                   
			//
			//
			//if (out_r < -1.0) 
			//				out_r = float(-1.0);
			//else if (out_r > 1.0)
			//				out_r = float(1.0); 

			++psamplesleft;
			++psamplesright;

		} while(--numsamples);
	
}
