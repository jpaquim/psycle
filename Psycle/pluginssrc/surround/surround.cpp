//////////////////////////////////////////////////////////////////////
// KarLKoX "Surround" plugin for PSYCLE

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "..\..\machineinterface.h"
#include "biquad.h"




CMachineParameter const paraLength = 
{ 
	"Length",
	"Length",										// description
	0,												// MinValue	
	1000,											// MaxValue
	MPF_STATE,										// Flags
	400,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraLength
};


CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	EFFECT,									// flags
	1,										// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"KarLKoX Surround (Debug build)",		// name
#else
	"KarLKoX Surround",						// name
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

private:
	biquad bqleft, bqright;


};

DLL_EXPORTS

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
// Probably you to show your custom window here
// or an about button
pCB->MessBox("Made 14/12/2001 by Saïd Bougribate for Psycl3!","-=KarLKoX=- [Surround]",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;

	if (Vals[par] == 0)
		Vals[par] = 1;
	else if (Vals[par] == 1000)
		Vals[par] = 999;
	BiQuad_new(LPF, 1.0, (double)Vals[par], (double)(pCB->GetSamplingRate()), 1, &bqleft);
	BiQuad_new(LPF, 1.0, (double)Vals[par], (double)(pCB->GetSamplingRate()), 1, &bqright);	


}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
		
	float xlb, xrb, xlt, xrt;

	// over all samples 
	do
		{
		float xl = *psamplesleft;
		float xr = *psamplesright;

		// extract base 
		xlb = (float)BiQuad(xl, &bqleft);
		xrb = (float)BiQuad(xr, &bqright);

		// extract treble 
		xlt = xl - xlb;
		xrt = xr - xrb;
		// phase left treble and recombine 
		xl = -xlt+xlb;
		xr = xrt+xrb;
		// store 
		if (xl < -32767) xl = -32767;
		if (xl > 32767) xl = 32767;
		if (xr < -32767) xr = -32767;
		if (xr > 32767) xr = 32767;
		
		*psamplesleft = xl;
		*psamplesright = xr;

		++psamplesleft;
		++psamplesright;

	} while(--numsamples);

}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	return false;
}