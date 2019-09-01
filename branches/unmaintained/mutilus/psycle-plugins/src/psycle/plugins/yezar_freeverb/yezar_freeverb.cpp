#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include "RevModel.hpp"
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

/////////////////////////////////////////////////////////////////////
// Arguru's psycle port of Yezar freeverb

#define NCOMBS				1
#define NALLS				12

CMachineParameter const paraCombdelay = 
{ 
	"Absortion",
	"Damp",																																												// description
	1,																																																// MinValue				
	640,																																												// MaxValue
	MPF_STATE,																																								// Flags
	320,
};

CMachineParameter const paraCombseparator = 
{ 
	"Stereo Width",
	"Width",																																								// description
	1,																																																// MinValue				
	640,																																												// MaxValue
	MPF_STATE,																																								// Flags
	126,
};


CMachineParameter const paraAPdelay = 
{ 
	"Room size",
	"Room size",																																				// description
	1,																																																// MinValue				
	640,																																												// MaxValue
	MPF_STATE,																																								// Flags
	175,
};

CMachineParameter const paraDry = 
{ 
	"Dry Amount",
	"Dry",																																												// description
	0,																																																// MinValue				
	640,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256,
};


CMachineParameter const paraWet = 
{ 
	"Wet Amount",
	"Wet",																																												// description
	0,																																																// MinValue				
	640,																																												// MaxValue
	MPF_STATE,																																								// Flags
	128,
};

CMachineParameter const *pParameters[] = 
{ 
	&paraCombdelay,
	&paraCombseparator,
	&paraAPdelay,
	&paraDry,
	&paraWet,
};

#pragma pack(1)								

#pragma pack()

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	0,																																								// flags
	5,																																								// numParameters
	pParameters,																												// Pointer to parameters
#ifdef _DEBUG
	"Jezar Freeverb (Debug build)",												// name
#else
	"Jezar Freeverb",																								// name
#endif
	"Freeverb",																																// short name
	"Jezar",																																// author
	"About",																																				// A command, that could be use for open an editor, etc...
	5
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
	
private:
	

	//CCombFilter comb[NCOMBS];
	revmodel reverb;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	Vals=new int[5];
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

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	Vals[par]=val;

	// Set damp
	reverb.setdamp(Vals[0]*0.0015625f);
	// Set width
	reverb.setwidth(Vals[1]*0.0015625f);
	// Set room size
	reverb.setroomsize(Vals[2]*0.0015625f);
	// Set reverb wet/dry
	reverb.setdry(Vals[3]*0.0015625f);
	reverb.setwet(Vals[4]*0.0015625f);

}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
pCB->MessBox("Ported in 31/5/2000 by Juan Antonio Arguelles Rius for Psycl3!","·-=<([Freeverb])>=-·",0);
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	// Compute intermediate variables
	// This should be computed on ParameterTweak for optimization,
	// using global intermediate variables, but,
	// anyway, a few calcs doesnt take too CPU.

	//float const cf=(float)Vals[4]*0.0000453514739229024943310657596371882f;
	//float const dry_amount				=(float)Vals[5]*0.00390625f;
	//float const wet_amount				=(float)Vals[6]*0.00390625f;
	
	//float l_revresult=0;
	//float r_revresult=0;

	//float g=(float)Vals[3]*0.0009765f;
	//int na=Vals[7];
	
	reverb.processreplace(psamplesleft,psamplesright,psamplesleft,psamplesright,numsamples,1);
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	if(param==0)
	{
		sprintf(txt,"%.f%%",value*0.15625f);
		return true;
	}
	if(param==1)
	{
		sprintf(txt,"%.0f degrees",value*0.28125f);
		return true;
	}
	if(param==2)
	{
		sprintf(txt,"%.1f mtrs.",(float)value*0.17f);
		return true;
	}
	else if (param==3)
	{
		sprintf(txt,"%.1f%%",float(value)*0.31250f);
		return true;
	}
	else if (param==4)
	{
		sprintf(txt,"%.1f%%",float(value)*0.46875f);
		return true;
	}
	return false;
}
