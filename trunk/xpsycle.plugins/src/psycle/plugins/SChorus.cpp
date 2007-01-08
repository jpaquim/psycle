//////////////////////////////////////////////////////////////////////
// Sartorius SChorus effect plugin for PSYCLE
//
// Based on Digital Effects Algorithms
// Alexey Smoli, http://st.karelia.ru/~smlalx
// smlalx@yahoo.com
//
// and Toby Kurien's Audio Effects Algorithms
// http://toby.za.net/dsp/index.htm

#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define MAXIMUM_DELAY   40000

#define NUMPARAMETERS 8

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

CMachineParameter const paraDelay = 
{ 
	"Delayer",
	"Delayer",										// description
	1,												// MinValue	
	MAXIMUM_DELAY,												// MaxValue
	MPF_STATE,										// Flags
	MAXIMUM_DELAY,
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
	&paraRate,
	&paraDelay
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
	"Sartorius",							// author
	"About",								// A command, that could be use for open an editor, etc...
	4
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
	float *DM_l,*DM_r;
	unsigned int buf_count;
	float min_sweep,max_sweep,sweep,step,outval[2];
	int samplerate;

};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[NUMPARAMETERS];
	DM_l = new float[MAXIMUM_DELAY];
	DM_r = new float[MAXIMUM_DELAY];
	for (int i=0;i<MAXIMUM_DELAY;i++)
	{
		DM_l[i] = DM_r[i] = 0;
	}
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
	samplerate = pCB->GetSamplingRate();
	buf_count=0;
	min_sweep = Vals[4] * .001 * samplerate;
    max_sweep = Vals[5] * .001 * samplerate;
    step = Vals[6]*.001;
    sweep = min_sweep;
	outval[0] = outval[1] = 0;
	for (int i=0;i<MAXIMUM_DELAY;i++)
	{
		DM_l[i] = DM_r[i] = 0;
	}
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
	if(samplerate!=pCB->GetSamplingRate())
	{
		samplerate = pCB->GetSamplingRate();
	
		min_sweep = Vals[4] * .001 * samplerate;
		max_sweep = Vals[5] * .001 * samplerate;
	}
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
pCB->MessBox("Sartorius Chorus\nBe carefull with wet and delayer!!!","SChorus",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;

	min_sweep = Vals[4] * .001 * samplerate;
    max_sweep = Vals[5] * .001 * samplerate;
    step = Vals[6]*.001;
	if (step == 0)
	{
		sweep = 0;
	}else{
		sweep = min_sweep;
	}

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	float const dry		=(float)(Vals[0])*0.00006103515625f;
	float const wet		=(float)(Vals[1])*0.000030517578125f;
	float const fbl		=(float)(Vals[2])*0.000030517578125f;
	float const fbr		=(float)(Vals[3])*0.000030517578125f;
	int const del		=Vals[7];

		do
		{
			float sl = *psamplesleft;
			float sr = *psamplesright; 

			outval[0] = outval[0]*fbl + DM_l[(buf_count+del -(int)sweep) % del ]*wet;
		    outval[0] += sl*dry;
			outval[1] = outval[1]*fbr + DM_r[(buf_count+del -(int)(max_sweep-sweep)) % del ]*wet;
			outval[1] += sr*dry;
			
			//if(outval[0] > 32767.f)
			//	sl = 32767;
			//else if(outval[0] < -32768.f)
			//	sl = -32768;
			//else
				sl = outval[0];
			//if(outval[1] > 32767.f)
			//	sr = 32767.f;
			//else if(outval[1] < -32768.f)
			//	sr = -32768.f;
			//else
				sr = outval[1];

			DM_l[buf_count] = sl;
			DM_r[buf_count] = sr;
			
			buf_count = (buf_count + 1) % del ;

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
	switch(param)
	{
		case 0:
			std::sprintf(txt,"%.6f",(float)value*0.00006103515625f);
			return true;
		case 1:
		case 2:
		case 3:
			std::sprintf(txt,"%.6f",(float)value*0.000030517578125f);
			return true;
		case 4:
		case 5:
			std::sprintf(txt,"%i ms",value);
			return true;
		case 6:
			std::sprintf(txt,"%i ms/s",value);
			return true;
		default:
			return false;
	}
}
