///\file
///\brief Arguru compressor plugin for PSYCLE
#include <psycle/plugin_interface.hpp>
#include <psycle/helpers/math.hpp>
#include <cstdio> // for std::sprintf

using namespace psycle::plugin_interface;
using namespace psycle::helpers::math;

CMachineParameter const paraGain = {"Input Gain", "Input Gain", 0, 128, MPF_STATE, 0};
CMachineParameter const paraThreshold = {"Threshold", "Threshold", 0, 128,MPF_STATE, 64};
CMachineParameter const paraRatio = {"Ratio", "Ratio", 0, 16, MPF_STATE, 16};
CMachineParameter const paraAttack = {"Attack", "Attack", 0, 128, MPF_STATE, 6};
CMachineParameter const paraRelease = {"Release", "Release", 0, 128, MPF_STATE, 0x2C};
CMachineParameter const paraClip = {"Soft clip", "Soft clip", 0, 1,MPF_STATE, 0};

enum { paramGain = 0, paramThreshold, paramRatio, paramAttack, paramRelease, paramClip };

CMachineParameter const *pParameters[] =  { 
	&paraGain,
	&paraThreshold,
	&paraRatio,
	&paraAttack,
	&paraRelease,
	&paraClip
};

CMachineInfo const MacInfo (
	MI_VERSION,				
	EFFECT,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"Arguru Compressor 1.2"
	#ifndef NDEBUG
		" (debug build)"
	#endif
	,
	"Compressor",
	"J. Arguelles & psycledelics",
	"About",
	1
);

class mi : public CMachineInterface {
	public:
		mi();
		~mi();
		/*override*/ void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
		/*override*/ bool DescribeValue(char* txt,int const param, int const value);
		/*override*/ void Command();
		/*override*/ void ParameterTweak(int par, int val);
	private:
		//float avgInputLevel;
		float currentGain;
		float gainStep;
		enum action { none, actAttack, actRelease} currentAction;
};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)

mi::mi()
:
	//avgInputLevel(0),
	currentGain(1),
	gainStep(0),
	currentAction(none)
{
	Vals = new int[MacInfo.numParameters];
}

mi::~mi() {
	delete[] Vals;
}

void mi::Command() {
	pCB->MessBox("By Arguru <arguru@smartelectronix.com>", "Arguru compressor", 0);
}

void mi::ParameterTweak(int par, int val) {
	Vals[par] = val;
}


void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {
	float const makeUpGain = 1 + Vals[paramGain] * 0.015625f;
	if(Vals[paramRatio] == 0) {
		// No compression. 
		do {
			*psamplesleft *= makeUpGain;
			*psamplesright *= makeUpGain;
			++psamplesleft;
			++psamplesright;
		} while(--numsamples);
	} else {
		float const static gainStepSpeed = deci_bell_to_linear(10.0f);
		float const threshold = Vals[paramThreshold] * 0.0078125f;
		float const attack = Vals[paramAttack] * pCB->GetSamplingRate() * 0.001f;
		float const release = Vals[paramRelease] * pCB->GetSamplingRate() * 0.001f;

		// TODO use peak or RMS?
		//float inputLevel = (std::fabs(*psamplesleft) + std::fabs(*psamplesright)) * 0.5f * 0.000030517578125f;
		//float const avgSamples = pCB->GetSamplingRate() / 20; // window for average (should lead to a constant level for 20Hz)
		//inputLevel = (avgInputLevel + inputLevel) / (avgSamples + 1); // average ponderation
		
		/// TODO soft clip

		if(Vals[paramRatio] >= 16) {
			// Hard limiter
			do {
				float const inputLevel = (std::fabs(*psamplesleft) + std::fabs(*psamplesright)) * 0.5f * 0.000030517578125f;
				if(inputLevel > threshold) {
					currentGain = threshold / inputLevel;
				} else if(currentGain < 1) {
					if(currentAction != actRelease) {
						currentAction = actRelease;
						gainStep = gainStepSpeed / (release * pCB->GetSamplingRate());
					}
					currentGain = std::min(currentGain + gainStep, 1.0f);
				} else {
					//currentAction = none;
					currentGain = 1;
				}
				*psamplesleft *= makeUpGain * currentGain;
				*psamplesright *= makeUpGain * currentGain;
				++psamplesleft;
				++psamplesright;
			} while(--numsamples);
		} else {
			// Compressor
			do {
				float const inputLevel = (std::fabs(*psamplesleft) + std::fabs(*psamplesright)) * 0.5f * 0.000030517578125f;
				if(inputLevel > threshold) {
					if(currentAction != actAttack) {
						currentAction = actAttack;
						currentGain = 1;
						gainStep = gainStepSpeed / (attack * pCB->GetSamplingRate());
					} else {
						// Note: Vals[paramRatio] is unusual: it does not express a dB ratio, but a linear one!
						float const targetGain = (threshold + (inputLevel - threshold) / (1 + Vals[paramRatio])) / inputLevel;
						if(currentGain > targetGain)
							currentGain  = std::max(currentGain - gainStep, targetGain);
					}
				} else if(currentGain < 1) {
					if(currentAction != actRelease) {
						currentAction = actRelease;
						gainStep = gainStepSpeed / (release * pCB->GetSamplingRate());
					}
					currentGain = std::min(currentGain + gainStep, 1.0f);
				} else {
					//currentAction = none;
					currentGain = 1;
				}
				*psamplesleft *= makeUpGain * currentGain;
				*psamplesright *= makeUpGain * currentGain;
				++psamplesleft;
				++psamplesright;
			} while(--numsamples);
		}
	}
}

bool mi::DescribeValue(char* txt,int const param, int const value) {
	switch(param) {
		case paramGain:
			std::sprintf(txt,"+%.1f dB", linear_to_deci_bell(1 + value * 0.015625f));
			return true;
		case paramThreshold:
			if(value == 0)
				std::sprintf(txt,"-inf. dB");
			else
				std::sprintf(txt,"%.1f dB", linear_to_deci_bell(value * 0.0078125f));
			return true;
		case paramRatio:
			if(value >= 16)
				std::sprintf(txt,"inf:1 (Limiter)");
			else if(value == 0)
				std::sprintf(txt,"1:1 (Bypass)");
			else
				std::sprintf(txt,"%d:1", value + 1);
			return true;
		case paramAttack: //fallthrough
		case paramRelease:
			std::sprintf(txt,"%d ms.", value);
			return true;
		case paramClip:
			std::sprintf(txt, value == 0 ? "Off" : "On");
			return true;
		default: return false;
	}
}
