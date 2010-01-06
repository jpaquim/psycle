///\file
///\brief Arguru compressor plugin for PSYCLE
#include <psycle/plugin_interface.hpp>
#include <cstdio> // for std::sprintf

using namespace psycle::plugin_interface;

CMachineParameter const paraGain = {"Input Gain","Input Gain",0,128,MPF_STATE, 0};
CMachineParameter const paraThreshold = {"Threshold","Threshold",0,128,MPF_STATE,64};
CMachineParameter const paraRatio = {"Ratio","Ratio",0,16,MPF_STATE,16};
CMachineParameter const paraAttack = {"Attack","Attack",0,128,MPF_STATE,6};
CMachineParameter const paraRelease = {"Release","Release",0,128,MPF_STATE,0x2C};
CMachineParameter const paraClip = {"Soft clip","Soft clip",0,1,MPF_STATE,0};
enum { paramGain = 0, paramThreshold, paramRatio, paramAttack, paramRelease, paramClip };

CMachineParameter const *pParameters[] = 
{ 
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
typedef enum { none=0, actAttack, actRelease}  action;


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
	float currentGain;
	float gainShift;
	action currentAction;

};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)

mi::mi(): currentGain(1.0f), gainShift(0.0f), currentAction(none)
{
	Vals = new int[MacInfo.numParameters];
}

mi::~mi()
{
	delete[] Vals;
}

void mi::Init()
{
}

void mi::SequencerTick()
{
}

void mi::Command()
{
	pCB->MessBox("By Arguru <arguru@smartelectronix.com>","Arguru compressor",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
}


void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	float const gain = 1.0f + Vals[paramGain]*0.015625f;
	float const threshold = Vals[paramThreshold]*0.0078125f;
	float const attack = Vals[paramAttack]*pCB->GetSamplingRate()*0.001f;
	float const release = Vals[paramRelease]*pCB->GetSamplingRate()*0.001f;

	if (Vals[paramRatio] == 0) {
		// No compression. 
		do
		{
			++psamplesleft;
			++psamplesright;
		} while(--numsamples);
	}
	else if (Vals[paramRatio] >= 16){
		// HardLimiter
		do
		{
			float analyzedValue = (*psamplesleft + *psamplesright) * 0.5f;

			++psamplesleft;
			++psamplesright;
		} while(--numsamples);
	}
	else {
		//target gain is incorrect, and currentGain is not constant. it depends on the analyzed value.
		float const targetGain = 1.0f/(1.0f+Vals[paramRatio]);
		// Compressor
		do
		{
			float analyzedValue = fabs(*psamplesleft) + fabs(*psamplesright) * 0.5f * 0.000030517578125f;
			if (analyzedValue > threshold) {
				if (currentAction != actAttack) {
					gainShift = (1.0f-targetGain)/attack;
					currentAction = actAttack;
				}
				if (currentGain > targetGain) {
					currentGain-=gainShift;
				}
			}
			else if (currentGain < 1.0f) {
				if (currentAction != actRelease) {
					gainShift = (1.0f-targetGain)/release;
					currentAction = actRelease;
				}
				currentGain+=gainShift;
			}
			*psamplesleft *= currentGain*gain;
			*psamplesright *= currentGain*gain;

			++psamplesleft;
			++psamplesright;
		} while(--numsamples);
	}
}

bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param) {
	case paramGain:
	{
		std::sprintf(txt,"+%.1f dB",20.0f * log10(1.0f+value*0.015625f));
		return true;
	}
	case paramThreshold:
	{
		if(value == 0)
			std::sprintf(txt,"-inf. dB");
		else
			std::sprintf(txt,"%.1f dB",20.0f * log10(value*0.0078125f));
		return true;
	}
	case paramRatio:
	{
		if (value >= 16) {
			std::sprintf(txt,"Limiter");
		}
		else if (value == 0) {
			std::sprintf(txt,"1:1 (Bypass)");
		}
		else {
			std::sprintf(txt,"1:%d", value+1);
		}
		return true;
	}
	case paramAttack: //fallthrough
	case paramRelease:
	{
		std::sprintf(txt,"%d ms.", value);
		return true;
	}
	case paramClip:
		{
			if (value == 0) std::sprintf(txt,"Off");
			else std::sprintf(txt,"On");
			return true;
		}
	default: return false;
	}
}
