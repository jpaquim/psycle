#include <psycle/plugin_interface.hpp>
#include "AllPass.hpp"
#include "CombFilter.hpp"
#include "LowPass.hpp"
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

/////////////////////////////////////////////////////////////////////
// Arguru reverb plugin for PSYCLE

#define NCOMBS				1
#define NALLS				12

CMachineParameter const paraCombdelay = 
{ 
	"Pre Delay",
	"Pre Delay time",																																// description
	1,																																																// MinValue				
	32768,																																												// MaxValue
	MPF_STATE,																																								// Flags
	929,
};

CMachineParameter const paraCombseparator = 
{ 
	"Comb Spread",
	"Comb Filter separation",																								// description
	16,																																																// MinValue				
	512,																																												// MaxValue
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

CMachineParameter const paraAPg = 
{ 
	"Feedback",
	"Feedback",																																								// description
	1,																																																// MinValue				
	1024,																																												// MaxValue
	MPF_STATE,																																								// Flags
	1001,
};

CMachineParameter const paraCutoff = 
{ 
	"Absortion",
	"Absortion",																																				// description
	1,																																																// MinValue				
	22050,																																												// MaxValue
	MPF_STATE,																																								// Flags
	7059,
};

CMachineParameter const paraDry = 
{ 
	"Dry",
	"Dry",																																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256,
};


CMachineParameter const paraWet = 
{ 
	"Wet",
	"Wet",																																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	128,
};


CMachineParameter const paraNC = 
{ 
	"Filters",
	"Number of allpass filters",																				// description
	0,																																																// MinValue				
	NALLS,																																												// MaxValue
	MPF_STATE,																																								// Flags
	NALLS,
};

CMachineParameter const *pParameters[] = 
{ 
	&paraCombdelay,
	&paraCombseparator,
	&paraAPdelay,
	&paraAPg,
	&paraCutoff,
	&paraDry,
	&paraWet,
	&paraNC

};

//#pragma pack(1)								
//
//#pragma pack()

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	0,																																								// flags
	8,																																								// numParameters
	pParameters,																												// Pointer to parameters
#ifdef _DEBUG
	"Arguru Reverb (Debug build)",												// name
#else
	"Arguru Reverb",																								// name
#endif
	"Reverb",																																// short name
	"J. Arguelles",																												// author
	"About",																																				// A command, that could be use for open an editor, etc...
	2
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
	
	// Reverb members
	virtual void SetAll(int delay);

private:
	

	//CCombFilter comb[NCOMBS];
	CCombFilter comb;
	CAllPass				all[NALLS];
	CLowpass				fl;
	CLowpass				fr;
	int								prevfilters;

};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	Vals=new int[8];
	prevfilters=0;
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

	if(par<2)
	{
		comb.Initialize(Vals[0],Vals[1]);
	}
	else if ( par == 2)
	{
		SetAll(Vals[2]);
	}
	else if ( par == 7)
	{
		for(int c=prevfilters;c<Vals[par];c++)
		{
			all[c].Clear();
		}
		prevfilters=Vals[par]-1;
	}
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
pCB->MessBox("Made 31/5/2000 by Juan Antonio Arguelles Rius for Psycl3!","·-=<([aRgUrU's R3V3RB])>=-·",0);
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	// Compute intermediate variables
	// This should be computed on ParameterTweak for optimization,
	// using global intermediate variables, but,
	// anyway, a few calcs doesnt take too CPU.

	float const cf=(float)Vals[4]*0.0000453514739229024943310657596371882f;
	float const dry_amount				=(float)Vals[5]*0.00390625f;
	float const wet_amount				=(float)Vals[6]*0.00390625f;
	
	float g=(float)Vals[3]*0.0009765f;
	--psamplesleft;
	--psamplesright;

	int const na=Vals[7];

	do
	{
		float const sl = *++psamplesleft;
		float const sr = *++psamplesright;
		
		comb.Work(sl+2,sr+2);
		float l_revresult=comb.left_output;
		float r_revresult=comb.right_output;
		
		for(int c=0;c<na;c++)
		{
			all[c].Work(l_revresult,r_revresult,g);
			l_revresult=all[c].left_output;
			r_revresult=all[c].right_output;
		}

		*psamplesleft=sl*dry_amount+fl.Process(l_revresult,cf)*wet_amount;
		*psamplesright=sr*dry_amount+fr.Process(r_revresult,cf)*wet_amount;

	} while(--numsamples);
	
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	if(param==0)
	{
		// Meter
		sprintf(txt,"%.1f ms.",(float)value*0.0226757f);
		return true;
	}

	if(param==2)
	{
		// Meter
		sprintf(txt,"%.1f mtrs.",(float)value*0.17f);
		return true;
	
	}

	if(param==3)
	{
		// Meter
		sprintf(txt,"%.1f%%",(float)value*0.0976562f);
		return true;
	
	}

	if(param==4)
	{
		// Frequency of the lowpass
		sprintf(txt,"%d Hzs.",value);
		return true;
	}

	if(param==5 || param==6)
	{
		// Dry and Wet parameters shows value measured in %
		sprintf(txt,"%.1f%%",float(value)*0.390625f);
		return true;
	}
	return false;
}

void mi::SetAll(int delay)
{
	for(int c=0;c<NALLS;c++)
	{
		all[c].Initialize(delay*(c+1)+(c*c),int(float(c)*1.3f));
	}
}
