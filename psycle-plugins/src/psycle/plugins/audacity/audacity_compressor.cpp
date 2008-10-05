// -*- mode:c++; indent-tabs-mode:t -*-
//////////////////////////////////////////////////////////////////////
// AudaCity Compressor plugin for PSYCLE by Sartorius
//
//   Original
/**********************************************************************

	Audacity: A Digital Audio Editor

	Compressor.cpp

	Dominic Mazzoni

	Steve Jolly made it inherit from EffectSimpleMono.
	GUI added and implementation improved by Dominic Mazzoni, 5/11/2003.

**********************************************************************/

#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "Compressor.h"

#define MAX_SAMPLES 3000
#define M_PI 3.14159265359f
#define NUMPARAMETERS 5

CMachineParameter const paraThreshold = 
{ 
	"Threshold",
	"Threshold",		// description
	-36,			// MinValue
	-1,			// MaxValue
	MPF_STATE,		// Flags
	-12,
};

CMachineParameter const paraRatio = 
{ 
	"Ratio",
	"Ratio",		// description
	100,			// MinValue
	10000,			// MaxValue
	MPF_STATE,		// Flags
	200,
};

CMachineParameter const paraAttackTime = 
{ 
	"Attack",
	"Attack",		// description
	100,			// MinValue
	10000,			// MaxValue
	MPF_STATE,		// Flags
	200,
};

CMachineParameter const paraDecayTime = 
{ 
	"Decay",
	"Decay",		// description
	100,			// MinValue
	10000,			// MaxValue
	MPF_STATE,		// Flags
	1000,
};


CMachineParameter const paraUseGain = 
{ 
	"Use gain",
	"Use gain",		// description
	0,			// MinValue
	1,			// MaxValue
	MPF_STATE,		// Flags
	0,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraThreshold,
	&paraRatio,
	&paraAttackTime,
	&paraDecayTime,
	&paraUseGain
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	EFFECT,			// flags
	NUMPARAMETERS,		// numParameters
	pParameters,		// Pointer to parameters
#ifdef _DEBUG
	"Audacity Compressor (Debug build)",		// name
#else
	"Audacity Compressor",				// name
#endif
	"ACompressor",					// short name
	"Dominic Mazzoni/Sartorius",			// author
	"About",					// A command, that could be use for open an editor, etc...
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

	EffectCompressor sl,sr;
	int samplerate;
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
	samplerate = pCB->GetSamplingRate();
	sl.setSampleRate(samplerate);
	sr.setSampleRate(samplerate);
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
	if(samplerate!=pCB->GetSamplingRate()) Init();
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
	pCB->MessBox("Audacity Compressor","ACompressor",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
	switch(par)
	{
	case 0:
		sl.setThreshold(val);
		sl.setGainDB();
		sr.setThreshold(val);
		sr.setGainDB();
		break;
	case 1: 
		sl.setRatio(val*.01);
		sr.setRatio(val*.01);
		break;
	case 2: 
		sl.setAttack(val*.001);
		sr.setAttack(val*.001);
		break;
	case 3: 
		sl.setDecay(val*.001);
		sr.setDecay(val*.001);
		break;
	case 4:
		sl.setGain(val==1);
		sr.setGain(val==1);
		break;
	default:
		break;
	}
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	if (sl.BufferIn(psamplesleft,numsamples)!=0)
		sl.Process(numsamples);
	if (sr.BufferIn(psamplesright,numsamples)!=0)
		sr.Process(numsamples);
	sl.BufferOut(psamplesleft,numsamples);
	sr.BufferOut(psamplesright,numsamples);
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{

	switch(param)
	{
		case 0:
			std::sprintf(txt,"%i dB",value);
			return true;
		case 1:
			std::sprintf(txt,"%.1f:1",(float)value*.01f);
			return true;
		case 2:
		case 3:
			std::sprintf(txt,"%.01f s",(float)value*.001f);
			return true;
		case 4:
			std::sprintf(txt,value?"yes":"no");
			return true;

		default:
			return false;
	}
}
