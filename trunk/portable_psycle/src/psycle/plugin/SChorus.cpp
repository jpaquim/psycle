//////////////////////////////////////////////////////////////////////
// Sartorius SChorus effect plugin for PSYCLE
//
// Based on Digital Effects Algorithms
// Alexey Smoli, http://st.karelia.ru/~smlalx
// smlalx@yahoo.com

#include <project.h>
#include <psycle/plugin/MachineInterface.h>
#include <string.h>
#include <stdlib.h>

#define MAXIMUM_DELAY   30000

#define NUMPARAMETERS 7

CMachineParameter const paraDry = 
{ 
	"Dry",
	"Dry",						// description
	-32768,												// MinValue	
	32768,											// MaxValue
	MPF_STATE,										// Flags
	32768,
};

CMachineParameter const paraWet = 
{ 
	"Wet",
	"Wet",						// description
	-32768,												// MinValue	
	32768,											// MaxValue
	MPF_STATE,										// Flags
	0,
};

CMachineParameter const paraFBL = 
{ 
	"Feedback left",
	"Feedback left",						// description
	-32768,												// MinValue	
	32768,											// MaxValue
	MPF_STATE,										// Flags
	16384,
};

CMachineParameter const paraFBR = 
{ 
	"Feedback right",
	"Feedback right",						// description
	-32768,												// MinValue	
	32768,											// MaxValue
	MPF_STATE,										// Flags
	16384,
};

CMachineParameter const paraMinD = 
{ 
	"Min Delay",
	"Min Delay",											// description
	1,												// MinValue	
	6000,											// MaxValue
	MPF_STATE,										// Flags
	1,
};

CMachineParameter const paraMaxD = 
{ 
	"Max Delay",
	"Max Delay",											// description
	1,												// MinValue	
	6000,											// MaxValue
	MPF_STATE,										// Flags
	5,
};

CMachineParameter const paraRate = 
{ 
	"Rate",
	"Rate",										// description
	0,												// MinValue	
	1000,												// MaxValue
	MPF_STATE,										// Flags
	1,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraDry,
	&paraWet,
	&paraFBL,
	&paraFBR,
	&paraMinD,
	&paraMaxD,
	&paraRate
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	EFFECT,										// flags
	NUMPARAMETERS,								// numParameters
	pParameters,								// Pointer to parameters
#ifdef _DEBUG
	"SChorus (Debug build)",		// name
#else
	"SChorus",						// name
#endif
	"SChorus",							// short name
	"Sartorius, Alexey Smoli",							// author
	"About",								// A command, that could be use for open an editor, etc...
	3
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
	int *DM_l,*DM_r;
	unsigned int buf_count;
	double min_sweep,max_sweep,sweep,step,outval[2];

};

DLL_EXPORTS

mi::mi()
{
	// The constructor zone
	Vals = new int[NUMPARAMETERS];
	DM_l = new int[MAXIMUM_DELAY];
	DM_r = new int[MAXIMUM_DELAY];
}

mi::~mi()
{
	delete Vals;
	delete DM_l;
	delete DM_r;
// Destroy dinamically allocated objects/memory here
}

void mi::Init()
{
// Initialize your stuff here
	buf_count=0;
	min_sweep = Vals[4] * 0.00029675445556640625;
    max_sweep = Vals[5] * 0.00029675445556640625;
    step = Vals[6]/1000.0;
    sweep = min_sweep;
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
pCB->MessBox("Sartorius Chorus","SChorus",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;

	min_sweep = Vals[4] * 0.00029675445556640625;
    max_sweep = Vals[5] * 0.00029675445556640625;
    step = Vals[6]/1000.0;
    sweep = min_sweep;

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	float const dry		=(float)(Vals[0])*0.00006103515625f;
	float const wet		=(float)(Vals[1])*0.00006103515625f;
	float const fbl		=(float)(Vals[2])*0.000030517578125f;
	float const fbr		=(float)(Vals[3])*0.000030517578125f;

		do
		{
			float sl = *psamplesleft;
			float sr = *psamplesright; 

	        outval[0] = sl*fbl + DM_l[(buf_count+MAXIMUM_DELAY-(int)sweep) % MAXIMUM_DELAY]*wet;
		    outval[0] += sl*dry;
			outval[1] = sr*fbr + DM_r[(buf_count+MAXIMUM_DELAY-(int)max_sweep+(int)sweep)%MAXIMUM_DELAY]*wet;
			outval[1] += sr*dry;
			
			if(outval[0] > 32767.0)
				sl = 32767;
			else if(outval[0] < -32768.0)
				sl = -32768;
            else
				sl = (float)outval[0];
			if(outval[1] > 32767.0)
				sr = 32767;
			else if(outval[1] < -32768.0)
				sr = -32768;
            else
				sr = (float)outval[1];

			DM_l[buf_count] = (int)sl;
			DM_r[buf_count] = (int)sr;
			
			buf_count = (buf_count + 1) % MAXIMUM_DELAY;

			*psamplesleft=sl;
			*psamplesright=sr;


			sweep += step;
	        /* check for sweep reversal */
		    if(sweep > max_sweep)               /* see if we hit top of sweep */
			    step = -step;                   /* reverse */
			else if(sweep < min_sweep)          /* or if we hit bottom of sweep */
				step = -step;                   /* reverse */

			++psamplesleft;
			++psamplesright;
		
		} while(--numsamples);
	
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{

	if(param==0 || param==1)
	{
		sprintf(txt,"%.6f",(float)value/16384.0);
		return true;
	}
	if(param==2 || param==3)
	{
		sprintf(txt,"%.6f",(float)value/32768.0);
		return true;
	}

	if(param==4 || param==5)
	{
		sprintf(txt,"%i ms",value);
		return true;
	}

	if(param==6)
	{
		sprintf(txt,"%i ms/s",value);
		return true;
	}

	return false;
}