// -*- mode:c++; indent-tabs-mode:t -*-
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

//////////////////////////////////////////////////////////////////////
// Arguru simple distortion/saturator plugin for PSYCLE

CMachineParameter const paraThreshold = 
{ 
	"Threshold",
	"Threshold level", // description
	1,  // MinValue				
	32768, // MaxValue
	MPF_STATE, // Flags
	0x200,
};

CMachineParameter const paraGain = 
{ 
	"Gain",
	"Gain", // description
	1, // MinValue				
	2048, // MaxValue
	MPF_STATE, // Flags
	1024,
};

CMachineParameter const paraInvert = 
{ 
	"Phase inversor",
	"Stereo phase inversor", // description
	0, // MinValue
	1, // MaxValue
	MPF_STATE, // Flags
	0,
};
CMachineParameter const paraMode = 
{ 
	"Mode",
	"Operational mode", // description
	0, // MinValue
	1, // MaxValue
	MPF_STATE, // Flags
	0,
};
CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraThreshold,
	&paraGain,
	&paraInvert,
	&paraMode
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	0, // flags
	4, // numParameters
	pParameters, // Pointer to parameters
#ifdef _DEBUG
	"Arguru Distortion (Debug build)", // name
#else
	"Arguru Distortion", // name
#endif
	"Distortion", // short name
	"J. Arguelles", // author
	"About", // A command, that could be use for open an editor, etc...
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
	float leftLim;
	float rightLim;

};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi():leftLim(1.0f), rightLim(1.0f)
{
	// The constructor zone
	Vals = new int[4];
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
pCB->MessBox("Made 18/5/2000 by Juan Antonio Arguelles Rius for Psycl3!","-=<([aRgUrU's sAtUrAt0R])>=-",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{

	float const threshold = (float)(Vals[0]);
	float const negthreshold = -(float)(Vals[0]);

	float const wet = (float)Vals[1]*0.00390625f;

	if(Vals[2]==0)
	{
		// No Phase inversion
		if (Vals[3]==0) {
			// Clip.
			do
			{
			float sl = *psamplesleft;
			float sr = *psamplesright;

			if (sl > threshold)sl = threshold;
			if (sl <= negthreshold)sl = negthreshold;

			if (sr >= threshold)sr = threshold;
			if (sr <= negthreshold)sr = negthreshold;

			*psamplesleft=sl*wet;
			*psamplesright=sr*wet;

			++psamplesleft;
			++psamplesright;
				
			} while(--numsamples);
		}
		else 
		{
			//Saturate
			do
			{
			float sl = (*psamplesleft)*leftLim;
			float sr = (*psamplesright)*rightLim;

			if (sl > threshold) {
				leftLim -= (sl-threshold)*0.1f/(*psamplesleft);
			} else if (sl <= negthreshold) {
				leftLim -= (sl-negthreshold)*0.1f/(*psamplesleft); //psamplesleft is already negative, so no need to multiply by -1
			} else if (leftLim > 1.0f){
				leftLim = 1.0f;
			} else leftLim = 0.01f +(leftLim*0.99f);

			if (sr > threshold) {
				rightLim -= (sr-threshold)*0.1f/(*psamplesright);
			} else if (sr <= negthreshold) {
				rightLim -= (sr-negthreshold)*0.1f/(*psamplesright); //psamplesright is already negative, so no need to multiply by -1
			} else if (rightLim > 1.0f){
				rightLim = 1.0f;
			} else rightLim = 0.01f +(rightLim*0.99f);


			*psamplesleft=sl*wet;
			*psamplesright=sr*wet;

			++psamplesleft;
			++psamplesright;
				
			} while(--numsamples);
		}
	}
	else
	{
		// Phase inversion
		do
		{
		float sl = *psamplesleft;
		float sr = *psamplesright;

		if (sl > threshold)sl = threshold;
		if (sl <= negthreshold)sl = negthreshold;

		if (sr >= threshold)sr = threshold;
		if (sr <= negthreshold)sr = negthreshold;

		*psamplesleft=sl*wet;
		*psamplesright=sr*-wet;

		++psamplesleft;
		++psamplesright;
			
		} while(--numsamples);
	}
	
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	if(param==2)
	{
		switch(value)
		{
			case 0:sprintf(txt,"Off");				break;
			case 1:sprintf(txt,"On");				break;
		}
		return true;
	}
	
	if(param==1)
	{
	float coef=value*0.00390625f;
	
	if(coef>0.0f)
	sprintf(txt,"%.1f dB",20.0f * log10(coef));
	else
	sprintf(txt,"-Inf. dB");				
	return true;
	}

	return false;
}