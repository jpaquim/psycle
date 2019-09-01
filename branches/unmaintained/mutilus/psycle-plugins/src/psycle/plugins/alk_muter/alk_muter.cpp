#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

//////////////////////////////////////////////////////////////////////
// Alk muter plugin for PSYCLE

CMachineParameter const paraType = 
{ 
	"Mute",
	"Mute off/on",																								// description
	0,																												// MinValue				
	1,																												// MaxValue
	MPF_STATE,																								// Flags
	0,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraType
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	0,																												// flags
	1,																												// numParameters
	pParameters,																								// Pointer to parameters
#ifndef NDEBUG
	"Alk Muter (Debug build)",																// name
#else
	"Alk Muter",																								// name
#endif
	"Muter",																								// short name
	"Alk",																												// author
	"About",																								// A command, that could be use for open an editor, etc...
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
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	Vals = new int[sizeof pParameters];
}

mi::~mi()
{
	delete Vals;
}

void mi::Init()
{
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::Command()
{
// Called when user presses editor button
pCB->MessBox("Made by Alkz0r","Alk's Muter",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	if(Vals[0]==1)
	{
		do
		{												
			*psamplesleft = 0.0f;
			*psamplesright = 0.0f;												
			++psamplesleft;
			++psamplesright;												
		} while(--numsamples);
	} 
}

bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(value)
	{
		case 0:sprintf(txt,"off");				break;
		case 1:sprintf(txt,"on");				break;
	}
	return false;
}
