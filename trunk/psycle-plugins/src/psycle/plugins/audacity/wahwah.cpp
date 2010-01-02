//////////////////////////////////////////////////////////////////////
// AudaCity WahWah effect plugin for PSYCLE by Sartorius
//
//   Original
/**********************************************************************

	Audacity: A Digital Audio Editor

	Wahwah.cpp

	Effect programming:
	Nasca Octavian Paul

	UI programming:
	Dominic Mazzoni (with the help of wxDesigner)
	Vaughan Johnson (Preview)

**********************************************************************/

#include <psycle/plugin_interface.hpp>
#include <psycle/helpers/math.hpp>
#include <cstdlib>
#include <cstring>
#include <cmath>

using namespace psycle::plugin_interface;
using namespace psycle::helpers::math;

#define LFO_SKIP_SAMPLES 30

#ifndef M_PI
	#define M_PI 3.14159265359f
#endif

#define NUMPARAMETERS 5

CMachineParameter const paraLFOFreq = 
{ 
	"LFO Freq",
	"LFOFreq",																								// description
	1,																																																// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	15,
};

CMachineParameter const paraLFOStartPhase = 
{ 
	"LFO start phase",
	"LFOStartPhase",																								// description
	0,																																																// MinValue				
	359,																																												// MaxValue
	MPF_STATE,																																								// Flags
	0,
};

CMachineParameter const paraDepth = 
{ 
	"Depth",
	"Depth",																								// description
	0,																																																// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	70,
};

CMachineParameter const paraResonance = 
{ 
	"Resonance",
	"Resonance",																																												// description
	1,																																																// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	25,
};

CMachineParameter const paraWahFreqOff = 
{ 
	"Wah freq offset",
	"WahFreqOff",																																												// description
	0,																																																// MinValue				
	100,																																												// MaxValue
	MPF_STATE,																																								// Flags
	30,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraLFOFreq,
	&paraLFOStartPhase,
	&paraDepth,
	&paraResonance,
	&paraWahFreqOff
};

CMachineInfo const MacInfo (
	MI_VERSION,				
	EFFECT,																																								// flags
	NUMPARAMETERS,																																// numParameters
	pParameters,																																// Pointer to parameters
#ifdef _DEBUG
	"Audacity WahWah (Debug build)",								// name
#else
	"Audacity WahWah",																								// name
#endif
	"WahWah",																												// short name
	"Nasca Octavian Paul/Sartorius",																												// author
	"About",																																// A command, that could be use for open an editor, etc...
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
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);

private:
	float phase;
	float lfoskip;
	unsigned int skipcount;
	float xn1_l, xn2_l, yn1_l, yn2_l;
	float xn1_r, xn2_r, yn1_r, yn2_r;
	float b0_l, b1_l, b2_l, a0_l, a1_l, a2_l;
	float b0_r, b1_r, b2_r, a0_r, a1_r, a2_r;
	float freq;
	float depth, freqofs, res;

};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)

mi::mi()
{
	// The constructor zone
	Vals = new int[NUMPARAMETERS];
}

mi::~mi()
{
	delete[] Vals;
}

void mi::Init()
{
// Initialize your stuff here
	freq = 1.5f;
	phase = 0;
	depth = .7f;
	freqofs = .3f;
	res = 2.5f;

	lfoskip = freq * 2 * M_PI / pCB->GetSamplingRate();
	skipcount = 0;
	xn1_l = 0;
	xn2_l = 0;
	yn1_l = 0;
	yn2_l = 0;

	xn1_r = 0;
	xn2_r = 0;
	yn1_r = 0;
	yn2_r = 0;

	b0_l = 0;
	b1_l = 0;
	b2_l = 0;
	a0_l = 0;
	a1_l = 0;
	a2_l = 0;

	b0_r = 0;
	b1_r = 0;
	b2_r = 0;
	a0_r = 0;
	a1_r = 0;
	a2_r = 0;
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
	lfoskip = freq * 2 * M_PI / pCB->GetSamplingRate();
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
pCB->MessBox("Audacity WahWah","WahWah",0);
}

void mi::ParameterTweak(int par, int val)
{
	Vals[par]=val;
	switch(par)
	{
		case 0: freq = float(val) * .1f; lfoskip = freq * 2 * M_PI / pCB->GetSamplingRate(); break;
		case 1: phase = float(val)  * (0.0055555555555555555555555555555556f * M_PI); break;
		case 2: depth = float(val) * .01f;  break;
		case 3: res = 1.f / ( float(val) * .2f);  break;
		case 4: freqofs = float(val) * .01f; break;
		default:
				break;
	}
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{

	double frequency, omega, sn, cs, alpha;
	const float depth_mul_1_minus_freqofs = depth * (1.f - freqofs) * .5f;

		do
			{

			float in_l = *psamplesleft * 0.000030517578125f;  // divide by 32768 =>  -1..1
			float in_r = *psamplesright * 0.000030517578125f; 

			if ((skipcount++) % LFO_SKIP_SAMPLES == 0) {
				float calc_1_time = float(skipcount) * lfoskip + phase; // :-)
				//Ensure limits.
				if (calc_1_time > 2*M_PI) {
					int memo = skipcount;
					skipcount-=pCB->GetSamplingRate()/freq;
				}
				float sintime;
				float costime;	
				psycle::helpers::math::sin_cos(calc_1_time, sintime, costime);
				frequency = 1.f + costime; // Left channel
				frequency = frequency * depth_mul_1_minus_freqofs + freqofs;
				frequency = exp((frequency - 1.f) * 6.f);
				omega = M_PI * frequency;
				if (omega > 2*M_PI) do {
					omega = omega - 2*M_PI;
				} while(omega > 2*M_PI); 
				

				sincos(omega, sn, cs);
				//sn = std::sin(omega);
				//cs = std::cos(omega);

				alpha = sn * res;
				//b0_l = (1 - cs) * .5f;
				b1_l = 1.f - cs;
				b2_l = b0_l = b1_l * .5f;
				a0_l = 1.f + alpha;
				a1_l = -2.f * cs;
				a2_l = 1.f - alpha;
				
				frequency = 1.f + sintime; // Right channel
				frequency = frequency * depth_mul_1_minus_freqofs + freqofs;
				frequency = exp((frequency - 1.f) * 6.f);
				omega = M_PI * frequency;
				if (omega > 2*M_PI) do {
					omega = omega - 2*M_PI;
				} while(omega > 2*M_PI); 

				sincos(omega, sn, cs);
				//sn = std::sin(omega);
				//cs = std::cos(omega);

				alpha = sn * res;
				//b0_r = (1 - cs) * .5f;
				b1_r = 1.f - cs;
				b2_r = b0_r = b1_r * .5f;
				a0_r = 1.f + alpha;
				a1_r = -2.f * cs;
				a2_r = 1.f - alpha;
			}

			float recip = 1 / (a0_l * a0_r);


			float out_l = (b0_l * in_l + b1_l * xn1_l + b2_l * xn2_l - a1_l * yn1_l - a2_l * yn2_l) * a0_r* recip; // /a0_l;
			psycle::helpers::math::erase_all_nans_infinities_and_denormals(out_l);
			// Prevents clipping
			if (out_l < -1.f) {
				out_l = -1.f;
			}
			else if (out_l > 1.f) {
				out_l = 1.f;                   
			}

			xn2_l = xn1_l;
			xn1_l = in_l;
			yn2_l = yn1_l;
			yn1_l = out_l;

			float out_r = (b0_r * in_r + b1_r * xn1_r + b2_r * xn2_r - a1_r * yn1_r - a2_r * yn2_r) * a0_l* recip; // / a0_r;
			psycle::helpers::math::erase_all_nans_infinities_and_denormals(out_r); 
			
			if (out_r < -1.f) {
				out_r = -1.f;
			}
			else if (out_r > 1.f) {
				out_r = 1.f; 
			}

			xn2_r = xn1_r;
			xn1_r = in_r;
			yn2_r = yn1_r;
			yn1_r = out_r;



			*psamplesleft = out_l * 32767.0f;  // Amplify

			*psamplesright = out_r * 32767.0f; // Amplify

			++psamplesleft;
			++psamplesright;

		} while(--numsamples);
	
}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param)
	{
		case 0:
			std::sprintf(txt,"%.1f Hz",(float)value*.1f);
			return true;
		case 1:
			std::sprintf(txt,"%i�",value);
			return true;
		case 2:
			std::sprintf(txt,"%i%%",value);
			return true;
		case 3:
			std::sprintf(txt,"%.1f",(float)value*.1f);
			return true;
		case 4:
			std::sprintf(txt,"%i%%",value);
			return true;
		default:
			return false;
	}
}
