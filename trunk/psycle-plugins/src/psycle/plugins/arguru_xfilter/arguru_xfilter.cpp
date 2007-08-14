#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

// Arguru xfilter

#define MAX_DELAY_SAMPLES				80000

CMachineParameter const paraDelay = 
{ 
	"Delay time",
	"Delay time",																																				// description
	0,																																																// MinValue				
	44100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	11025,
};

CMachineParameter const paraFeedback = 
{ 
	"Feedback",
	"Feedback",																																								// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	128
};

CMachineParameter const paraDry = 
{ 
	"Dry",
	"Dry",																																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	256
};


CMachineParameter const paraWet = 
{ 
	"Wet",
	"Wet",																																												// description
	0,																																																// MinValue				
	256,																																												// MaxValue
	MPF_STATE,																																								// Flags
	128
};

CMachineParameter const paraTickmode = 
{ 
	"Tick mode",
	"Tick mode",																																				// description
	0,																																																// MinValue				
	1,																																																// MaxValue
	MPF_STATE,																																								// Flags
	0
};

CMachineParameter const paraTicktweak = 
{ 
	"Ticks",
	"Ticks",																																								// description
	0,																																																// MinValue				
	8,																																																// MaxValue
	MPF_STATE,																																								// Flags
	3
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraDelay,
	&paraFeedback,
	&paraDry,
	&paraWet,
	&paraTickmode,
	&paraTicktweak
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	0,																																								// flags
	6,																																								// numParameters
	pParameters,																												// Pointer to parameters
#ifdef _DEBUG
	"Arguru CrossDelay (Debug build)",												// name
#else
	"Arguru CrossDelay",																								// name
#endif
	"CrossDelay",																																// short name
	"J. Arguelles",																												// author
	"About",																																				// A command, that could be use for open an editor, etc...
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
	virtual void ParameterTweak(int par, int val);

	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void SetDelay(int delay,int fdb);

private:

	float dbl[MAX_DELAY_SAMPLES];
	float dbr[MAX_DELAY_SAMPLES];
	int dcl,dcr,ccl,ccr;
	
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[6];
	for(int c=0;c<MAX_DELAY_SAMPLES;c++)
	{
		dbl[c]=0;
		dbr[c]=0;
	}
	ccl=0;
	ccr=0;
	dcl=0;
	dcr=0;
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
pCB->MessBox("Made 18/5/2000 by Juan Antonio Arguelles Rius for Psycl3!","-=<([aRgUrU's Cr0sSdElAy])>=-",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;

	if(par==0 && Vals[4]==0)
	SetDelay(Vals[0],0);

	if(par==5 && Vals[4]==1)
	{
		Vals[0]=Vals[5]*pCB->GetTickLength()*2;
		SetDelay(Vals[0],0);
	}
}

void mi::SetDelay(int delay,int fdb)
{
	ccl=MAX_DELAY_SAMPLES-8;
	ccr=ccl-(delay/2);
	
	dcl=ccl-delay;
	dcr=ccr-delay;

	if(dcl<0)dcl=0;
	if(dcr<0)dcr=0;

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	float fbc=(float)Vals[1]*0.00390625f;
	float const cdry=Vals[2]*0.00390625f;
	float const cwet=Vals[3]*0.00390625f;
	do
	{
		float const il=++*psamplesleft;
		float const ir=++*psamplesright;

		dbl[ccl]=il+dbl[dcl]*fbc;
		dbr[ccr]=ir+dbr[dcr]*fbc;

		*psamplesleft				=il*cdry+dbl[dcl]*cwet;
		*psamplesright				=ir*cdry+dbr[dcl]*cwet;
		
		if(++ccl==MAX_DELAY_SAMPLES)ccl=0;
		if(++ccr==MAX_DELAY_SAMPLES)ccr=0;
		if(++dcl==MAX_DELAY_SAMPLES)dcl=0;
		if(++dcr==MAX_DELAY_SAMPLES)dcr=0;

		++psamplesleft;
		++psamplesright;
	} while(--numsamples);
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	if(param==0)
	{
		float const spt=(float)pCB->GetTickLength()*2;
		sprintf(txt,"%.3f",(float)value/spt);
		return true;
	}
	if(param==1)
	{
		sprintf(txt,"%.1f%%",(float)value*0.390625f);
		return true;
	}
	if(param==2 || param==3)
	{
		float coef=value*0.00390625f;
		if(coef>0.0f)
			sprintf(txt,"%.1f dB",20.0f * log10(coef));
		else
			sprintf(txt,"-Inf. dB");				
		return true;
	}
	if(param==4)
	{
		if(value==0)
			sprintf(txt,"Off");
		else
			sprintf(txt,"On");
		return true;
	}
	return false;
}
