#include <project.h>
#include <psycle/plugin/MachineInterface.h>
#include <psycle/plugin/surround/biquad.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////
// KarLKoX "Surround" plugin for PSYCLE

#define VERNUM "v1.2"

CMachineParameter const paraLength = 
{ 
	"Cutoff Frequency",
	"Cutoff for HighPass Filter",					// description
	0,												// MinValue	
	1000,											// MaxValue
	MPF_STATE,										// Flags
	400,
};
CMachineParameter const paraMode = 
{ 
	"Work Mode",
	"Working model for the surround",				// description
	0,												// MinValue	
	1,												// MaxValue
	MPF_STATE,										// Flags
	0,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraLength,
	&paraMode
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	EFFECT,									// flags
	2,										// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"KarLKoX Surround (Debug build)" VERNUM,		// name
#else
	"KarLKoX Surround" VERNUM,						// name
#endif
	"Surround",								// short name
	"Saïd Bougribate",						// author
	"About",								// A command, that could be use for open an editor, etc...
	2										// must be 2 else we can't see the knob (??)
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
	void Setup(bool clearfiltermemory);
	
private:
	biquad bqleft, bqright;
	int smprate;

};

DLL_EXPORTS

mi::mi()
{
	// The constructor zone
	Vals = new int[2];
}

mi::~mi()
{
	delete Vals;
// Destroy dinamically allocated objects/memory here
}

void mi::Init()
{
	Setup(true);
}

void mi::Setup(bool clearfiltermemory)
{
	smprate=pCB->GetSamplingRate();
	switch( Vals[1]  )
	{
	case 0:
		BiQuad_new(LPF, 0.0f, (float)Vals[0], (float)smprate, 1, &bqleft,clearfiltermemory);
		break;
	case 1:
		BiQuad_new(HPF, 0.0f, (float)Vals[0], (float)smprate, 1, &bqleft,clearfiltermemory);
		BiQuad_new(HPF, 0.0f, (float)Vals[0], (float)smprate, 1, &bqright,clearfiltermemory);
		break;
	}
}
void mi::SequencerTick()
{
	if (pCB->GetSamplingRate() != smprate ) Setup(false);
}

void mi::Command()
{
// Called when user presses editor button
// Probably you to show your custom window here
// or an about button
	pCB->MessBox("Made 14/12/2001 by Saïd Bougribate for Psycl3!\n\n Some modifications made by [JAZ] on Dec 2002\n Version" VERNUM,"-=KarLKoX=- [Surround]",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par] = val;
	Setup(false);
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	//float xlb, xrb, xlt, xrt;

	// over all samples 
	switch(Vals[1])
	{
	case 0:
		do
		{
			float xl = *psamplesleft;

			*psamplesleft  = -xl + 2*(float)BiQuad(xl, &bqleft); //BQ is a Lowpass

			++psamplesleft;

		} while(--numsamples);
		break;
	case 1:
		do
		{
			float xl = *psamplesleft;
			float xr = *psamplesleft;
			
			float xtl = (float)BiQuad(xl, &bqleft); // BQ is a HighPass
			float xtr = (float)BiQuad(xl, &bqright); // BQ is a HighPass

			*psamplesleft  = xl+xtl*0.5-xtr*0.5;
			*psamplesright  = xr-xtl*0.5+xtr*0.5;
			
			++psamplesleft;
			++psamplesright;
			
		} while(--numsamples);
		break;
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch (param)
	{
	case 0:
		sprintf(txt,"%i Hz",value);
		return true;
	case 1:
		switch(value)
		{
		case 0:
			strcpy(txt,"Old Mode");
			return true;
		case 1:
			strcpy(txt,"New Mode");
			return true;
		}
		break;
	}
	return false;
}

