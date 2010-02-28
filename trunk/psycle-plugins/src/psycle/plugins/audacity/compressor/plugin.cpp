//////////////////////////////////////////////////////////////////////
// AudaCity Compressor plugin for PSYCLE by Sartorius
//
//   Original
/**********************************************************************

	Audacity: A Digital Audio Editor

	Compressor.cpp

	Dominic Mazzoni

	Steve Jolly made it inherit from EffectSimpleMono.
	GUI added and implementation improved by Dominic Mazzoni, 5/11/2003.

**********************************************************************/

#include <psycle/plugin_interface.hpp>
#include "Compressor.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdio>

using namespace psycle::plugin_interface;

#define MAX_SAMPLES 3000

#ifndef M_PI
	#define M_PI 3.14159265359f // note that's supposed to be a double!
#endif

CMachineParameter const paraThreshold = {"Threshold","Threshold", -36 , -1 ,MPF_STATE, -12 };
CMachineParameter const paraRatio = {"Ratio","Ratio", 100 , 10000 ,MPF_STATE, 200 };
CMachineParameter const paraAttackTime = {"Attack","Attack", 100 , 10000 ,MPF_STATE, 200 };
CMachineParameter const paraDecayTime = {"Decay","Decay", 100 ,	10000 ,MPF_STATE, 1000 };
CMachineParameter const paraUseGain = {"Use gain","Use gain", 0 , 1 ,MPF_STATE, 0 };
CMachineParameter const *pParameters[] = {
	&paraThreshold,
	&paraRatio,
	&paraAttackTime,
	&paraDecayTime,
	&paraUseGain
};

CMachineInfo const MacInfo (
	MI_VERSION,
	EFFECT,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"Audacity Compressor"
		#ifndef NDEBUG
		" (Debug build)"
		#endif
		,
	"ACompressor",
	"Dominic Mazzoni/Sartorius",
	"About",
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
		virtual void Command();
		virtual void ParameterTweak(int par, int val);

	private:
		EffectCompressor sl,sr;
		int samplerate;
};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)

mi::mi() {
	Vals = new int[MacInfo.numParameters];
}

mi::~mi() {
	delete[] Vals;
}

void mi::Init() {
	samplerate = pCB->GetSamplingRate();
	sl.setSampleRate(samplerate);
	sr.setSampleRate(samplerate);
}

void mi::SequencerTick() {
	if(samplerate!=pCB->GetSamplingRate()) Init();
}

void mi::Command() {
	pCB->MessBox("Audacity Compressor","ACompressor",0);
}

void mi::ParameterTweak(int par, int val) {
	Vals[par]=val;
	switch(par) {
		case 0:
			sl.setThreshold(val);
			sl.setGainDB();
			sr.setThreshold(val);
			sr.setGainDB();
			break;
		case 1: 
			sl.setRatio(val*.01);
			sr.setRatio(val*.01);
			break;
		case 2: 
			sl.setAttack(val*.001);
			sr.setAttack(val*.001);
			break;
		case 3: 
			sl.setDecay(val*.001);
			sr.setDecay(val*.001);
			break;
		case 4:
			sl.setGain(val==1);
			sr.setGain(val==1);
			break;
		default:
			break;
	}
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {
	if (sl.BufferIn(psamplesleft,numsamples)!=0)
		sl.Process(numsamples);
	if (sr.BufferIn(psamplesright,numsamples)!=0)
		sr.Process(numsamples);
	sl.BufferOut(psamplesleft,numsamples);
	sr.BufferOut(psamplesright,numsamples);
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value) {
	switch(param) {
		case 0:
			std::sprintf(txt,"%i dB",value);
			return true;
		case 1:
			std::sprintf(txt,"%.1f:1",(float)value*.01f);
			return true;
		case 2:
		case 3:
			std::sprintf(txt,"%.01f s",(float)value*.001f);
			return true;
		case 4:
			std::sprintf(txt,value?"yes":"no");
			return true;

		default:
			return false;
	}
}
