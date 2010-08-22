/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov stk Plucked plugin for PSYCLE
// v0.3
//
// Based on The Synthesis ToolKit in C++ (STK)
// By Perry R. Cook and Gary P. Scavone, 1995-2004.
// http://ccrma.stanford.edu/software/stk/

#include <psycle/plugin_interface.hpp>
#include <stk/Stk.h>
#include <stk/Effect.h>
#include <stk/JCRev.h>
#include <stk/NRev.h>
#include <stk/PRCRev.h>

#define NUMPARAMETERS 4

using namespace psycle::plugin_interface;

// Stk recently got a namespace. We (re)declare it for backward compatibility with older stk versions.
namespace stk {} using namespace stk;

CMachineParameter const paraRev = {"Reverb", "Reverb", 0, 2, MPF_STATE, 0};
CMachineParameter const paraTime = {"Time", "Time", 1, 32767, MPF_STATE, 32};
CMachineParameter const paraDryWet = {"Dry/Wet", "Dry/Wet", 0,100, MPF_STATE, 50};
CMachineParameter const paraMixing = {"Mixing", "Mixing", 0, 1, MPF_STATE, 0};

CMachineParameter const *pParameters[] = 
{ 
		&paraRev,
		&paraTime,
		&paraDryWet,
		&paraMixing
};


CMachineInfo const MacInfo(
		MI_VERSION,     
		EFFECT,
		NUMPARAMETERS,
		pParameters,
		"stk Reverbs"
			#ifndef NDEBUG
				" (debug build)"
			#endif
			,
		"stk Reverbs",
		"Sartorius and STK 4.2.1 developers",
		"Help",
		1
);

class mi : public CMachineInterface {
	public:
			mi();
			virtual ~mi();

			virtual void Init();
			virtual void SequencerTick();
			virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
			virtual bool DescribeValue(char* txt,int const param, int const value);
			virtual void ParameterTweak(int par, int val);
			virtual void Command();

	private:
			JCRev   jcrev[2];
			NRev    nrev[2];
			PRCRev  pcrrev[2];
			Effect  *rev_l, *rev_r;
			StkFloat samplerate;
};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)

mi::mi() {
		Vals = new int[NUMPARAMETERS];
		rev_l = &jcrev[0];
		rev_r = &jcrev[1];
}

mi::~mi() {
		delete[] Vals;
}
void mi::Command() {
	pCB->MessBox("stk Reverbs\n","stk Reverbs",0);
}

void mi::Init() {
	samplerate = (StkFloat)pCB->GetSamplingRate();
	Stk::setSampleRate(samplerate);
}

void mi::SequencerTick() {
	if(samplerate != (StkFloat)pCB->GetSamplingRate()) {
		samplerate = (StkFloat)pCB->GetSamplingRate();
		Stk::setSampleRate(samplerate);
		StkFloat const t60 = StkFloat(Vals[1]) * 0.03125;
		for(unsigned int i = 0; i < 2; ++i) {
			jcrev[i].setT60(t60);
			nrev[i].setT60(t60);
			pcrrev[i].setT60(t60);
		}
	}
}

void mi::ParameterTweak(int par, int val) {
	// Called when a parameter is changed by the host app / user gui
	Vals[par] = val;
	switch(par) {
		case 0:
			switch(val) {
				case 0: rev_l = &jcrev[0]; rev_r = &jcrev[1]; break;
				case 1: rev_l = &nrev[0]; rev_r = &nrev[1]; break;
				case 2: rev_l = &pcrrev[0]; rev_r = &pcrrev[1]; break;
			}
			rev_l->clear();
			rev_r->clear();
		break;
		case 1:
			for(unsigned int i = 0; i < 2; ++i) {
				jcrev[i].setT60(StkFloat(val) * 0.03125);
				nrev[i].setT60(StkFloat(val) * 0.03125);
				pcrrev[i].setT60(StkFloat(val) * 0.03125);
			}
	}
}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks) {
	if(Vals[2]) {
		StkFloat const drywet = StkFloat(Vals[2])*.01;
		rev_l->setEffectMix(drywet);
		rev_r->setEffectMix(drywet);
		if(Vals[3] == 0) {
			#if STK_VERSION != -1
				switch(Vals[0]) {
					case 0:
						do {
							jcrev[0].tick(StkFloat(*psamplesleft / 65536)) * 65536.f;
							jcrev[1].tick(StkFloat(*psamplesright / 65536)) * 65536.f;
							++psamplesleft;
							++psamplesright;
						} while(--numsamples);
					case 1:
						do {
							nrev[0].tick(StkFloat(*psamplesleft / 65536)) * 65536.f;
							nrev[1].tick(StkFloat(*psamplesright / 65536)) * 65536.f;
							++psamplesleft;
							++psamplesright;
						} while(--numsamples);
					case 2:
						do {
							pcrrev[0].tick(StkFloat(*psamplesleft / 65536)) * 65536.f;
							pcrrev[1].tick(StkFloat(*psamplesright / 65536)) * 65536.f;
							++psamplesleft;
							++psamplesright;
						} while(--numsamples);
				}
			#else
				do {
					*psamplesleft=rev_l->tick(StkFloat(*psamplesleft / 65536)) * 65536.f;
					*psamplesright=rev_r->tick(StkFloat(*psamplesright / 65536)) * 65536.f;
					++psamplesleft;
					++psamplesright;
				} while(--numsamples);
			#endif
		} else {
			#if STK_VERSION != -1
				switch(Vals[0]) {
					case 0:
						do {
							jcrev[0].tick(StkFloat(*psamplesleft / 65536));
							jcrev[1].tick(StkFloat(*psamplesright / 65536));
							*psamplesleft = float(jcrev[0].lastOut(0) + 0.3 * jcrev[0].lastOut(1)) * 65536.f;
							*psamplesright = float(jcrev[1].lastOut(1) + 0.3 * jcrev[1].lastOut(0)) * 65536.f;
							++psamplesleft;
							++psamplesright;
						} while(--numsamples);
					case 1:
						do {
							nrev[0].tick(StkFloat(*psamplesleft / 65536));
							nrev[1].tick(StkFloat(*psamplesright / 65536));
							*psamplesleft = float(nrev[0].lastOut(0) + 0.3 * nrev[0].lastOut(1)) * 65536.f;
							*psamplesright = float(nrev[1].lastOut(1) + 0.3 * nrev[1].lastOut(0)) * 65536.f;
							++psamplesleft;
							++psamplesright;
						} while(--numsamples);
					case 2:
						do {
							pcrrev[0].tick(StkFloat(*psamplesleft / 65536));
							pcrrev[1].tick(StkFloat(*psamplesright / 65536));
							*psamplesleft = float(pcrrev[0].lastOut(0) + 0.3 * pcrrev[0].lastOut(1)) * 65536.f;
							*psamplesright = float(pcrrev[1].lastOut(1) + 0.3 * pcrrev[1].lastOut(0)) * 65536.f;
							++psamplesleft;
							++psamplesright;
						} while(--numsamples);
				}
			#else
				do {
					rev_l->tick(StkFloat(*psamplesleft / 65536));
					rev_r->tick(StkFloat(*psamplesright / 65536));
					*psamplesleft = float(rev_l->lastOutLeft() + 0.3 * rev_r->lastOutRight()) * 65536.f;
					*psamplesright = float(rev_l->lastOutRight() + 0.3 * rev_r->lastOutLeft()) * 65536.f;
					++psamplesleft;
					++psamplesright;
				} while(--numsamples);
			#endif
		}
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt, int const param, int const value) {
	switch(param) {
		case 0:
			switch(value) {
				case 0:
					std::sprintf(txt, "JCRev");
					return true;
				case 1:
					std::sprintf(txt, "NRev");
					return true;
				case 2:
					std::sprintf(txt, "PCRRev");
					return true;
			}
		break;
		case 1:
			if(value > 1919)
					std::sprintf(txt, "%0.2fs aah! are you nuts?", value * 0.03125);
			else
					std::sprintf(txt, "%0.2fs", value * 0.03125);
			return true;
		break;
		case 2:
			std::sprintf(txt, "%i%%:%i%%", 100 - value, value);
			return true;
		break;
		case 3:
			std::sprintf(txt, value == 0 ? "as is" : "mix channels");
			return true;
	}
	return false;
}
