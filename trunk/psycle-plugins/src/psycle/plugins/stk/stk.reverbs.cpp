// -*- mode:c++; indent-tabs-mode:t -*-
/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov stk Plucked plugin for PSYCLE
// v0.3
//
// Based on The Synthesis ToolKit in C++ (STK)
// By Perry R. Cook and Gary P. Scavone, 1995-2004.
// http://ccrma.stanford.edu/software/stk/

#include <psycle/plugin_interface.hpp>
#include <stk/stk.h>
#include <stk/Effect.h>
#include <stk/JCRev.h>
#include <stk/NRev.h>
#include <stk/PRCRev.h>

#define NUMPARAMETERS 4

CMachineParameter const paraRev = {"Reverb","Reverb",0,2,MPF_STATE,0};
CMachineParameter const paraTime = {"Time","Time",1,32767,MPF_STATE,32};
CMachineParameter const paraDryWet = {"Dry/Wet","Dry/Wet",0,100,MPF_STATE,50};
CMachineParameter const paraMixing = {"Mixing","Mixing",0,1,MPF_STATE,0};

CMachineParameter const *pParameters[] = 
{ 
		&paraRev,
		&paraTime,
		&paraDryWet,
		&paraMixing
};


CMachineInfo const MacInfo = 
{
		MI_VERSION,     
		EFFECT,                                                         // flags
		NUMPARAMETERS,                                                  // numParameters
		pParameters,                                                    // Pointer to parameters
#ifdef _DEBUG
		"stk Reverbs (Debug build)",            // name
#else
		"stk Reverbs",                                          // name
#endif
		"stk Reverbs",                                                  // short name
		"Sartorius and STK 4.2.1 developers",                                                   // author
		"Help",                                                                 // A command, that could be use for open an editor, etc...
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
		JCRev   jcrev[2];
		NRev    nrev[2];
		PRCRev  pcrrev[2];
		Effect  *rev_l,*rev_r;
		StkFloat samplerate;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
		Vals=new int[NUMPARAMETERS];
		rev_l = &jcrev[0];
		rev_r = &jcrev[1];
}

mi::~mi()
{
		// Destroy dinamically allocated objects/memory here
		delete Vals;
		rev_l = NULL;
		rev_r = NULL;
}

void mi::Init()
{
// Initialize your stuff here
	samplerate=(StkFloat)pCB->GetSamplingRate();
	Stk::setSampleRate(samplerate);
}


void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
	if(samplerate!=(StkFloat)pCB->GetSamplingRate())
	{
		samplerate = (StkFloat)pCB->GetSamplingRate();
		Stk::setSampleRate(samplerate);
		StkFloat const t60 = StkFloat(Vals[1])*0.03125;
		for(short i=0;i<2;i++)
		{
				jcrev[i].setT60(t60);
				nrev[i].setT60(t60);
				pcrrev[i].setT60(t60);
		}
	}
}

void mi::ParameterTweak(int par, int val)
{
		// Called when a parameter is changed by the host app / user gui
		Vals[par]=val;
		if(par==0)
		{
				switch(val)
				{
				case 0:rev_l = &jcrev[0];rev_r = &jcrev[1];break;
				case 1:rev_l = &nrev[0];rev_r = &nrev[1];break;
				case 2:rev_l = &pcrrev[0];rev_r = &pcrrev[1];break;
				}
				rev_l->clear();
				rev_r->clear();
		}


		if(par==1)
				for(short i=0;i<2;i++)
				{
						jcrev[i].setT60(StkFloat(val)*0.03125);
						nrev[i].setT60(StkFloat(val)*0.03125);
						pcrrev[i].setT60(StkFloat(val)*0.03125);
				}
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button

pCB->MessBox("stk Reverbs\n","stk Reverbs",0);

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{
		if(Vals[2])
		{
			StkFloat const drywet = StkFloat(Vals[2])*.01;
			rev_l->setEffectMix(drywet);
			rev_r->setEffectMix(drywet);
			
			if(Vals[3]==0)
			{
				do
				{
					*psamplesleft=rev_l->tick(StkFloat(*psamplesleft/65536))*65536.f;
					*psamplesright=rev_r->tick(StkFloat(*psamplesright/65536))*65536.f;

					++psamplesleft;
					++psamplesright;
				} while(--numsamples);
			} else {
				do
				{
					rev_l->tick(StkFloat(*psamplesleft/65536));
					rev_r->tick(StkFloat(*psamplesright/65536));

					*psamplesleft=float(rev_l->lastOutLeft()+0.3*rev_r->lastOutRight())*65536.f;
					*psamplesright=float(rev_l->lastOutRight()+0.3*rev_r->lastOutLeft())*65536.f;

					++psamplesleft;
					++psamplesright;
				} while(--numsamples);
			}
		}

}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
		if(param==0)
				switch(value)
				{
				case 0:
						std::sprintf(txt,"JCRev");
						return true;
				case 1:
						std::sprintf(txt,"NRev");
						return true;
				case 2:
						std::sprintf(txt,"PCRRev");
						return true;
				}
		if(param==1)
		{
				if(value>1919)
						std::sprintf(txt,"%0.2fs aah! are you nuts?",value*0.03125);
				else
						std::sprintf(txt,"%0.2fs",value*0.03125);
				return true;
		}
		if(param==2)
		{
				std::sprintf(txt,"%i%%:%i%%",(100-value),value);
				return true;
		}

		if(param==3)
		{
				std::sprintf(txt,(value==0)?"as is":"mix channels");
				return true;
		}


		return false;
}
