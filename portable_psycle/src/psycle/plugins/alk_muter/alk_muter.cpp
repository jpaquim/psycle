#include <project.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Arguru simple distortion/saturator plugin for PSYCLE

CMachineParameter const paraType = 
{ 
	"Mute",
	"Mute off/on",						// description
	0,												// MinValue	
	1,											// MaxValue
	MPF_STATE,										// Flags
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
	0,										// flags
	1,										// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"Alk Muter (Debug build)",		// name
#else
	"Alk Muter",					// name
#endif
	"Muter",							// short name
	"Alk",							// author
	"About",								// A command, that could be use for open an editor, etc...
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
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[1];
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

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
pCB->MessBox("Made by Alkz0r","Alk's Muter",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
}

// Work... where all is cooked 
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

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(value)
	{
		case 0:sprintf(txt,"off");	break;
		case 1:sprintf(txt,"on");	break;
	}

	return false;
}