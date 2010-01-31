///\file
///\brief Arguru compressor plugin for PSYCLE
/// Reverse engineered by JosepMa from the dissasembled sources

#include <psycle/plugin_interface.hpp>
#include <psycle/helpers/math.hpp>
#include <cstdio> // for std::sprintf

using namespace psycle::plugin_interface;
using namespace psycle::helpers::math;

CMachineParameter const paraGain = {"Input Gain","Input Gain",0,128,MPF_STATE, 0};
CMachineParameter const paraThreshold = {"Threshold","Threshold",0,128,MPF_STATE,64};
CMachineParameter const paraRatio = {"Ratio","Ratio",0,16,MPF_STATE,16};
CMachineParameter const paraAttack = {"Attack","Attack",0,128,MPF_STATE,6};
CMachineParameter const paraRelease = {"Release","Release",0,128,MPF_STATE,0x2C};
CMachineParameter const paraClip = {"Soft clip","Soft clip",0,1,MPF_STATE,0};

enum { paramGain = 0, paramThreshold, paramRatio, paramAttack, paramRelease, paramClip };

CMachineParameter const *pParameters[] = { 
	&paraGain,
	&paraThreshold,
	&paraRatio,
	&paraAttack,
	&paraRelease,
	&paraClip
};

CMachineInfo const MacInfo (
	MI_VERSION,
	0x0120,
	EFFECT,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"Arguru Compressor"
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
	
		/*override*/ void Init();
		/*override*/ void SequencerTick();
		/*override*/ void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
		/*override*/ bool DescribeValue(char* txt,int const param, int const value);
		/*override*/ void Command();
		/*override*/ void ParameterTweak(int par, int val);
	private:
		float currentGain;
		//float targetGain;
		//float gainStep;
		std::int32_t currentSR;
		//enum action { none, actAttack, actRelease} currentAction;
	
};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)

mi::mi()
:
	currentGain(1.0f),
	//targetGain(1.0f),
	//gainStep(0.0f),
	currentSR(0)
	//, currentAction(none)
{
	Vals = new int[MacInfo.numParameters];
}

mi::~mi() {
	delete[] Vals;
}

void mi::Init() {
	currentSR = pCB->GetSamplingRate();
}

void mi::SequencerTick() {
	if (currentSR != pCB->GetSamplingRate()) {
		currentSR = pCB->GetSamplingRate();
	}
}

void mi::Command(){
	pCB->MessBox("By Arguru <arguru@smartelectronix.com>","Arguru compressor",0);
}

void mi::ParameterTweak(int par, int val) {
	Vals[par] = val;
}


void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {
#if 1
	float const corrected_gain =  (Vals[paramGain]*0.015625000f+1.0f)*0.000030517578125f;
	float* pleft = psamplesleft;
	float* pright = psamplesright;

	for (int cont = numsamples; cont > 0; cont--) {
		*(pleft++) *= corrected_gain;
		*(pright++) *= corrected_gain;
	}

	if (Vals[paramRatio] != 0) {
		float const correctedthreshold = Vals[paramThreshold]*0.0078125000f;
		double const corrected_ratio = (Vals[paramRatio] <16)? 1.0f/(1.0f + Vals[paramRatio]) : 0.0f;
		double const attackconst = 1.0/((1.0+Vals[paramAttack])*currentSR*0.001);
		double const releaseconst = 1.0/((1.0+Vals[paramRelease])*currentSR*0.001);
		
		float* pleft = psamplesleft;
		float* pright = psamplesright;

		for (int cont = numsamples; cont > 0; cont--) {
			double targetGain;
			double const analyzedValue = std::max(fabs(*pleft),fabs(*pright));
			if(analyzedValue <= correctedthreshold) {
				targetGain = 1.0f;
			}
			else {
				targetGain = ((analyzedValue - correctedthreshold)*corrected_ratio+correctedthreshold)/analyzedValue;
			}
			double newgain = (targetGain - currentGain);
			if (targetGain < currentGain) {
				newgain*=attackconst;
			}
			else {
				newgain*=releaseconst;
			}

			currentGain += newgain;
			*(pleft++) *= currentGain;
			*(pright++) *= currentGain;
		}
	}

	if (Vals[paramClip] != 0) {
		float* pleft = psamplesleft;
		float* pright = psamplesright;
		for (int cont = numsamples; cont > 0; cont--) {
			*pleft =tanh(*pleft);
			*pright =tanh(*pright);
			pleft++;
			pright++;
		}
	}
	for (int cont = numsamples; cont > 0; cont--) {
		*(psamplesleft++) *= 32768.0f;
		*(psamplesright++) *= 32768.0f;
	}
#elif 0
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
		float const thresholdb16 = Vals[paramThreshold] * 256.0f;
		//float const thresholdb1 = Vals[paramThreshold] * 0.0078125f;
		float const threshold = Vals[paramThreshold] * 0.0078125f;
		float const attack = Vals[paramAttack] * currentSR * 0.001f;
		float const release = Vals[paramRelease] * currentSR * 0.001f;
	
	
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
#if 1
			float const corrected_ratio = (Vals[paramRatio] <16)? 1.0f/(1.0f + Vals[paramRatio]) : 0.0f;
			do {
				//float const sl = *psamplesleft*gain;
				//float const sr = *psamplesright*gain;
				//float const analyzedValue = std::max(fabs(sl),fabs(sr));
				float const inputLevel = (std::fabs(*psamplesleft) + std::fabs(*psamplesright)) * 0.5f * 0.000030517578125f;

				//if (analyzedValue > thresholdb16) {
				if(inputLevel > threshold) {
					if(currentAction != actAttack) {
						currentAction = actAttack;
						//targetGain = thresholdb1*((analyzedValue - thresholdb16)*corrected_ratio);
						//gainStep = (1.0f-targetGain)/attack;
						currentGain = 1.0f;
						gainStep = gainStepSpeed / attack;
					} else {
						// Note: Vals[paramRatio] is unusual: it does not express a dB ratio, but a linear one!
						float const targetGain = (threshold + (inputLevel - threshold) / (1 + Vals[paramRatio])) / inputLevel;
						if(currentGain > targetGain)
							currentGain  = std::max(currentGain - gainStep, targetGain);
					}
					#if 0
					if (currentGain > targetGain) {
						currentGain-=gainStep;
						if (currentGain < targetGain) {
							currentGain = targetGain;
						}
					}
					#endif
				}
				else if(currentGain < 1.0f) {
					if(currentAction != actRelease) {
						currentAction = actRelease;
						//gainStep = (1.0f-targetGain)/release;
						gainStep = gainStepSpeed / release;
					}
					currentGain+=gainStep;
	
				} else {
					currentAction = none;
					currentGain = 1.0f;
				}
				//*psamplesleft = sl*currentGain;
				//*psamplesright = sr*currentGain;
				*psamplesleft *= makeUpGain * currentGain;
				*psamplesright *= makeUpGain * currentGain;
				++psamplesleft;
				++psamplesright;
			} while(--numsamples);
#elif 0
			// This is a compression over the sampled values themselves. It sounds like a saturator.
			float const targetGain = 1.0f/(1.0f+Vals[paramRatio]);
			do
			{
				float const sl = *psamplesleft*makeUpGain;
				float const sr = *psamplesright*makeUpGain;
	
				float const analyzedValue = std::max(fabs(sl),fabs(sr));
				if (analyzedValue > thresholdb16) {
					if (currentAction != actAttack) {
						gainStep = (1.0f-targetGain)/attack;
						currentAction = actAttack;
					}
					if (currentGain > targetGain) {
						currentGain-=gainStep;
					}
					if (sl > 0.0f) {
						*psamplesleft = thresholdb16- thresholdb16*currentGain + sl*currentGain;
					}
					else {
						*psamplesleft = -(thresholdb16- thresholdb16*currentGain) + sl*currentGain;
					}
					if (sr > 0.0f) {
						*psamplesright = thresholdb16- thresholdb16*currentGain + sr*currentGain;
					}
					else {
						*psamplesright = -(thresholdb16- thresholdb16*currentGain) + sr*currentGain;
					}
				}
				else if (currentGain < 1.0f) {
					if (currentAction != actRelease) {
						gainStep = (1.0f-targetGain)/release;
						currentAction = actRelease;
					}
					currentGain+=gainStep;
					*psamplesleft *= makeUpGain;
					*psamplesright *= makeUpGain;
				}
				else {
					currentAction = none;
					*psamplesleft *= makeUpGain;
					*psamplesright *= makeUpGain;
				}
				++psamplesleft;
				++psamplesright;
			} while(--numsamples);
#endif
		}
	}
#endif
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
			std::sprintf(txt,"1:inf (Limiter)");
		else if(value == 0)
			std::sprintf(txt,"1:1 (Bypass)");
		else
			std::sprintf(txt,"1:%d", value + 1);
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
