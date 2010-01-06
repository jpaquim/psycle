#include <psycle/plugin_interface.hpp>
#include <cstdio>
#include <math.h>

using namespace psycle::plugin_interface;

CMachineParameter const paraDelay = {"effect","Fractal Effect",0,0xfffe,MPF_STATE,0x80};
CMachineParameter const paraFeedback = {"depth","Fractal Depth",0,0x20,MPF_STATE,0x01};

CMachineParameter const *pParameters[] = 
{ 
	&paraDelay,
	&paraFeedback,
};

CMachineInfo const MacInfo(
	MI_VERSION,	
	EFFECT,
	sizeof pParameters / sizeof *pParameters,
	pParameters,
	"Ninereeds Fractal 7900s Port",
	"Fractal Dist",
	"Ninereeds & 7900",
	"About",
	1
);

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();
	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
	virtual void ParameterTweak(int par, int val);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();

private:
	float Effect;
	float EffectB;
	float EffectA;
	int   Depth;	
};

PSYCLE__PLUGIN__INSTANTIATOR(mi, MacInfo)


mi::mi() {
	Vals=new int[MacInfo.numParameters];
}

mi::~mi() {
	delete Vals;
}

void mi::Init() {
	Effect = (32768.0f * 9.0f) / (65534.0f);
	EffectB = 3.0f - (3.0f * Effect);
	EffectA = 1.0f - (Effect + EffectB);
	Depth  = 1;
}

void mi::SequencerTick() {
	Effect = (((float) Vals[0]) * 9.0f) / (65534.0f);
	EffectB = 3.0f - (3.0f * Effect);
	EffectA = 1.0f - (Effect + EffectB);
	Depth = Vals[1];
}

void mi::Command() {
	pCB->MessBox("Code: Steve Horne aka Ninereeds\nPsyVsn: Jochem vd. Lubbe aka 7900","Fractal Dist",0);
}

void mi::ParameterTweak(int par, int val) {
	Vals[par]=val;
	if(par==0)
	{
		Effect = (((float) Vals[0]) * 9.0f) / ((float) 65534); //effect
		EffectB = 3.0f - (3.0f * Effect);
		EffectA = 1.0f - (Effect + EffectB);
	}
	else
		Depth = Vals[1];//depth;
}

float const scale = (1.0f * 65536.0f);
float const halfscale = (0.5f * 65536.0f);

void mi::Work(float *psamples, float *psamplesright , int numsamples, int tracks)
{
		float s, ss;
		int   n;
	do
	{
		s = ((*psamples) + halfscale) / scale;
		n = Depth;
		while (n-- > 0)		{
			ss = s * s;

			s =   (EffectA * ss * s)
				+ (EffectB * ss    )
				+ (Effect  * s     );
		}
		if (s < 0.0)
		{			*psamples = -halfscale;		}
		else if (s > 1.0)
		{			*psamples = halfscale;		}
		else
		{			*psamples = (s * scale) - halfscale;		}

		s = ((*psamplesright) + halfscale) / scale;
		n = Depth;
		while (n-- > 0)		{
			ss = s * s;

			s =   (EffectA * ss * s)
				+ (EffectB * ss    )
				+ (Effect  * s     );
		}
		if (s < 0.0)
		{			*psamplesright = -halfscale;		}
		else if (s > 1.0)
		{			*psamplesright = halfscale;		}
		else
		{			*psamplesright = (s * scale) - halfscale;		}
		
	psamples++;;
	psamplesright++;
	}while(--numsamples);
}

bool mi::DescribeValue(char* txt,int const param, int const value)
{

	if(param==0)
	{
		std::sprintf(txt,"%u",value);
		return true;
	}
	if(param==1)
	{
		std::sprintf(txt,"%u",value);
		return true;
	}
	return false;
}