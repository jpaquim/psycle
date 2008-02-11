// -*- mode:c++; indent-tabs-mode:t -*-
//////////////////////////////////////////////////////////////////////
// LADSPA GVerb plugin for PSYCLE by Sartorius
//
//   Original
/*

GVerb algorithm designed and implemented by Juhana Sadeharju.
LADSPA implementation and GVerb speeds ups by Steve Harris.

Comments and suggestions should be mailed to Juhana Sadeharju
(kouhia at nic funet fi).

*/

#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "gverb.h"

#define NUMPARAMETERS 8


#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)
#define CO_DB(v) (20.0f * log10f(v))

CMachineParameter const paraRoomSize = 
{ 
	"Room size",
	"Room size",																								// description
	1,																																																// MinValue				
	300,																																												// MaxValue
	MPF_STATE,																																								// Flags
	144,
};

CMachineParameter const paraRevTime = 
{ 
	"Reverb time",
	"Reverb time",																								// description
	10,																																																// MinValue				
	3000,																																												// MaxValue
	MPF_STATE,																																								// Flags
	1800,
};

CMachineParameter const paraDamping = 
{ 
	"Damping",
	"Damping",																								// description
	0,																																																// MinValue				
	1000,																																												// MaxValue
	MPF_STATE,																																								// Flags
	1000,
};

CMachineParameter const paraBandwidth = 
{ 
	"Input bandwidth",
	"Input bandwidth",																																												// description
	0,																																																// MinValue				
	1000,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0,
};

CMachineParameter const paraDry = 
{ 
	"Dry signal level",
	"Dry",																								// description
	-70000,																																																// MinValue				
	0,																																												// MaxValue
	MPF_STATE,																																								// Flags
	-70000,
};

CMachineParameter const paraEarly = 
{ 
	"Early reflection level",
	"Early",																								// description
	-70000,																																																// MinValue				
	0,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0,
};


CMachineParameter const paraTail = 
{ 
	"Tail level",
	"Tail level",																																												// description
	-70000,																																																// MinValue				
	0,																																												// MaxValue
	MPF_STATE,																																								// Flags
	-17500,
};

CMachineParameter const paraMonoStereo = 
{ 
	"Input",
	"Input",																																												// description
	0,																																																// MinValue				
	1,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0,
};


CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraRoomSize,
	&paraRevTime,
	&paraDamping,
	&paraBandwidth,
	&paraDry,
	&paraEarly,
	&paraTail,
	&paraMonoStereo
};

CMachineInfo const MacInfo = 
{
	MI_VERSION,				
	EFFECT,																																								// flags
	NUMPARAMETERS,																																// numParameters
	pParameters,																																// Pointer to parameters
#ifdef _DEBUG
	"LADSPA GVerb (Debug build)",								// name
#else
	"LADSPA GVerb",																								// name
#endif
	"GVerb",																												// short name
	"Juhana Sadeharju/Steve Harris/Sartorius",																												// author
	"About",																																// A command, that could be use for open an editor, etc...
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

	ty_gverb *gv_l,*gv_r;
	int samplerate;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[NUMPARAMETERS];
}

mi::~mi()
{
	delete Vals;
	gverb_free(gv_l);
	gverb_free(gv_r);
}

void mi::Init()
{
// Initialize your stuff here
	samplerate = pCB->GetSamplingRate();

	gv_l = gverb_new(samplerate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);

	gv_r = gverb_new(samplerate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);

}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
	if(samplerate != pCB->GetSamplingRate())
	{
		samplerate = pCB->GetSamplingRate();

		gverb_free(gv_l);
		gverb_free(gv_r);
		
		gv_l = gverb_new(samplerate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);
		gv_r = gverb_new(samplerate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);

		gverb_set_roomsize(gv_l,(float)Vals[0]); gverb_set_roomsize(gv_r,(float)Vals[0]);
		gverb_set_revtime(gv_l,(float)Vals[1]*.01f); gverb_set_revtime(gv_r,(float)Vals[1]*.01f);
		gverb_set_damping(gv_l,(float)Vals[2] * .001f); gverb_set_damping(gv_r,(float)Vals[2] * .001f);
		gverb_set_inputbandwidth(gv_l,(float)Vals[3] * .001f); gverb_set_inputbandwidth(gv_r,(float)Vals[3] * .001f);
		gverb_set_earlylevel(gv_l,DB_CO((float)Vals[5]*.001f)); gverb_set_earlylevel(gv_r,DB_CO((float)Vals[5]*.001f));
		gverb_set_taillevel(gv_l,DB_CO((float)Vals[6]*.001f)); gverb_set_taillevel(gv_r,DB_CO((float)Vals[6]*.001f));

	}
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
pCB->MessBox("LADSPA GVerb","GVerb",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
	switch(par)
	{
		case 0: gverb_set_roomsize(gv_l,(float)val); gverb_set_roomsize(gv_r,(float)val); break;
		case 1: gverb_set_revtime(gv_l,(float)val*.01f); gverb_set_revtime(gv_r,(float)val*.01f); break;
		case 2: gverb_set_damping(gv_l,(float)val * .001f); gverb_set_damping(gv_r,(float)val * .001f); break;
		case 3: gverb_set_inputbandwidth(gv_l,(float)val * .001f); gverb_set_inputbandwidth(gv_r,(float)val * .001f); break;
		case 4: break;
		case 5: gverb_set_earlylevel(gv_l,DB_CO((float)val*.001f)); gverb_set_earlylevel(gv_r,DB_CO((float)val*.001f)); break;
		case 6: gverb_set_taillevel(gv_l,DB_CO((float)val*.001f)); gverb_set_taillevel(gv_r,DB_CO((float)val*.001f)); break;
		case 7: gverb_flush(gv_l); gverb_flush(gv_r); break;
		default:
				break;
	}
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	float const dry = DB_CO((float)Vals[4]*.001f);
	
	float sl = 0;
	float sr = 0;

	if (Vals[7])
	{
		do
		{
			float _sl = *psamplesleft;// * 0.000030517578125f;
			float _sr = *psamplesright;// * 0.000030517578125f;
			gverb_do(gv_l,_sl,&sl,&sr);
			*psamplesleft = _sl*dry + sl;//*32767.f;
			*psamplesright = _sr*dry + sr;//*32767.f;
			gverb_do(gv_r,_sr,&sl,&sr);
			*psamplesleft += sl;//*32767.f;
			*psamplesright += sr;//*32767.f;

			++psamplesleft;
			++psamplesright;
		} while(--numsamples);
	}
	else
	{
		do
		{
			float sm = (*psamplesleft + *psamplesright) / 2;// * 0.000030517578125f;
			gverb_do(gv_l,sm,&sl,&sr);

			*psamplesleft = *psamplesleft * dry + sl;//*32767.f;
			*psamplesright = *psamplesright * dry + sr;//*32767.f;

			++psamplesleft;
			++psamplesright;
		} while(--numsamples);
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{

	switch(param)
	{
		case 0:
			std::sprintf(txt,"%i m",value);
			return true;
		case 1:
			std::sprintf(txt,"%.2f s",(float)value*.01f);
			return true;
		case 2: case 3:
			std::sprintf(txt,"%.01f",(float)value*.001f);
			return true;
		case 4: case 5: case 6:
			std::sprintf(txt,"%i dB",value/1000);
			return true;
		case 7:
			std::sprintf(txt,value?"stereo":"mono");
			return true;

		default:

			return false;
	}
}
