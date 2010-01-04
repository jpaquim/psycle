///\file
///\brief Arguru simple distortion/saturator plugin for PSYCLE
#include <psycle/plugin_interface.hpp>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

using namespace psycle::plugin_interface;

CMachineParameter const paraLength = {"Length","Length",1,8192,MPF_STATE,2048};
CMachineParameter const paraSlope = {"Slope","Slope",1,2048,MPF_STATE,512};

CMachineParameter const *pParameters[] = {
	&paraLength,
	&paraSlope
};

CMachineInfo const MacInfo (
	MI_VERSION,
	EFFECT,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"Arguru Goaslicer 1.2"
	#ifndef NDEBUG
		" (Debug build)"
	#endif
	,
	"Goaslicer",
	"J. Arguelles",
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
		float slopeAmount;
		int timerSamples;
		bool muted;
		bool changing;
		float m_CurrentVolume;
		float m_TargetVolume;
		int currentSR;
		int m_Timer;
};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)

mi::mi(): slopeAmount(0.01f), timerSamples(2048), muted(false), changing(false),
m_CurrentVolume(1.0f), m_TargetVolume(1.0f), currentSR(44100), m_Timer(0) {
	Vals = new int[MacInfo.numParameters];
}

mi::~mi() {
	delete[] Vals;
}

void mi::Init() {
}

void mi::SequencerTick() {
	if (currentSR != pCB->GetSamplingRate()) {
		currentSR = pCB->GetSamplingRate();
		timerSamples = Vals[0]*44100/currentSR;
		slopeAmount = Vals[1]*5.38330078125f/currentSR;
	}
	m_Timer = 0;
	if (muted) {
		muted = false;
		changing = true;
	}
}

void mi::Command() {
	pCB->MessBox("Made 18/5/2000 by Juan Antonio Arguelles Rius for Psycl3!","-=<([aRgUrU's G-o-a-s-l-i-c-e-r])>=-",0);
}

void mi::ParameterTweak(int par, int val) {
	Vals[par] = val;
	timerSamples = Vals[0]*44100/currentSR;
	slopeAmount = Vals[1]*5.38330078125f/currentSR;
}

void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {

	if (m_Timer < timerSamples && m_Timer+numsamples >= timerSamples) {
		int diff = timerSamples - m_Timer;
		m_Timer+= diff;
		numsamples-=diff;
		if (changing) {
			while(m_CurrentVolume<0.99f && diff--) {
				(*psamplesleft)*=m_CurrentVolume; (*psamplesright)*=m_CurrentVolume;
				psamplesleft++; psamplesright++;
				m_CurrentVolume+=slopeAmount;
			}
			diff++;
		}
		psamplesleft+=diff;
		psamplesright+=diff;
		changing = true;
		muted = true;
	}
	m_Timer+=numsamples;
	if (!changing) {
		if(!muted) return;
		while(numsamples--) *psamplesleft++ = *psamplesright++ = 0;
	} else if (muted){
		// mute enabled
		while(m_CurrentVolume>0.01f && numsamples--) {
			(*psamplesleft)*=m_CurrentVolume; (*psamplesright)*=m_CurrentVolume;
			psamplesleft++; psamplesright++;
			m_CurrentVolume-=slopeAmount;
		}
		numsamples++;
		while(numsamples--) *psamplesleft++ = *psamplesright++ = 0;
		if (m_CurrentVolume<=0.01f) {
			changing=false;
		}
	} else {
		// mute disabled
		while(m_CurrentVolume<0.99f && numsamples--) {
			(*psamplesleft)*=m_CurrentVolume; (*psamplesright)*=m_CurrentVolume;
			psamplesleft++; psamplesright++;
			m_CurrentVolume+=slopeAmount;
		}
		if (m_CurrentVolume>=0.99f) {
			changing=false;
		}
	}
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value) {
	switch(param) {
	case 0:
	{
		int samp = value*44100/currentSR;
		if (samp < pCB->GetTickLength()) {
			std::sprintf(txt, "%.02f ticks (%dms)", (float)samp/pCB->GetTickLength(), 1000*samp/currentSR);
		}
		else {
			std::sprintf(txt, "1 tick (%dms)", 1000*pCB->GetTickLength()/currentSR);
		}
		return true;
	}
	case 1:
	{
		int slopms = 1000000/(value*5.38330078125);
		std::sprintf(txt, "%d mcs", slopms);
		return true;
	}
	default: return false; // returning false will simply show the value as a raw integral number
	}
}

