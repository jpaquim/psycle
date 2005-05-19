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




#include <project.private.hpp>
#include <psycle/plugin_interface.hpp>
#include <cstdlib>
#include <cstring>
#include <math.h>

#define lfoskipsamples 30
#define M_PI 3.14159265359
#define NUMPARAMETERS 5

CMachineParameter const paraLFOFreq = 
{ 
	"LFO Freq",
	"LFOFreq",						// description
	1,												// MinValue	
	100,											// MaxValue
	MPF_STATE,										// Flags
	15,
};

CMachineParameter const paraLFOStartPhase = 
{ 
	"LFO start phase",
	"LFOStartPhase",						// description
	0,												// MinValue	
	359,											// MaxValue
	MPF_STATE,										// Flags
	0,
};

CMachineParameter const paraDepth = 
{ 
	"Depth",
	"Depth",						// description
	0,												// MinValue	
	100,											// MaxValue
	MPF_STATE,										// Flags
	70,
};

CMachineParameter const paraResonance = 
{ 
	"Resonance",
	"Resonance",											// description
	1,												// MinValue	
	100,											// MaxValue
	MPF_STATE,										// Flags
	25,
};

CMachineParameter const paraWahFreqOff = 
{ 
	"Wah freq offset",
	"WahFreqOff",											// description
	0,												// MinValue	
	100,											// MaxValue
	MPF_STATE,										// Flags
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

CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	EFFECT,										// flags
	NUMPARAMETERS,								// numParameters
	pParameters,								// Pointer to parameters
#ifdef _DEBUG
	"Audacity WahWah (Debug build)",		// name
#else
	"Audacity WahWah",						// name
#endif
	"WahWah",							// short name
	"Nasca Octavian Paul, wrapper by Sartorius",							// author
	"About",								// A command, that could be use for open an editor, etc...
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
	float phase_l,phase_r;
	float lfoskip;
	unsigned long skipcount;
	float xn1_l, xn2_l, yn1_l, yn2_l;
	float xn1_r, xn2_r, yn1_r, yn2_r;
	float b0, b1, b2, a0, a1, a2;
	float freq, startphase;
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
	delete Vals;
}

void mi::Init()
{
// Initialize your stuff here
   freq = 1.5f;
   startphase = .0f;
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

   b0 = 0;
   b1 = 0;
   b2 = 0;
   a0 = 0;
   a1 = 0;
   a2 = 0;

   phase_l = startphase;
   phase_r = phase_l+M_PI;

}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
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
		case 0: freq = val * .1f; lfoskip = freq * 2 * M_PI / pCB->GetSamplingRate(); break;
		case 1: startphase = val  * 0.0055555555555555555555555555555556f * M_PI;  break;
		case 2: depth = val * .01f;  break;
		case 3: res = val * .1f;  break;
		case 4: freqofs = val * .01f; break;
		default:
			 break;
	}
}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{

   float frequency, omega, sn, cs, alpha;


		do
		{
			float in_l = *psamplesleft * 0.000030517578125f;  // divide by 32768 =>  -1..1
			float in_r = *psamplesright * 0.000030517578125f; 

	        if ((skipcount++) % lfoskipsamples == 0) {
				frequency = (1 + cos(skipcount * lfoskip + phase_l)) * .5f;
				frequency = frequency * depth * (1 - freqofs) + freqofs;
				frequency = exp((frequency - 1) * 6);
				omega = M_PI * frequency;
				sn = sin(omega);
				cs = cos(omega);
				alpha = sn / (2 * res);
				b0 = (1 - cs) / 2;
				b1 = 1 - cs;
				b2 = (1 - cs) / 2;
				a0 = 1 + alpha;
				a1	= -2 * cs;
				a2 = 1 - alpha;
			};
			float out_l = (b0 * in_l + b1 * xn1_l + b2 * xn2_l - a1 * yn1_l - a2 * yn2_l) / a0;

			xn2_l = xn1_l;
			xn1_l = in_l;
			yn2_l = yn1_l;
			yn1_l = out_l;

	        if ((skipcount++) % lfoskipsamples == 0) {
				frequency = (1 + cos(skipcount * lfoskip + phase_r)) * .5f;
				frequency = frequency * depth * (1 - freqofs) + freqofs;
				frequency = exp((frequency - 1) * 6);
				omega = M_PI * frequency;
				sn = sin(omega);
				cs = cos(omega);
				alpha = sn / (2 * res);
				b0 = (1 - cs) / 2;
				b1 = 1 - cs;
				b2 = (1 - cs) / 2;
				a0 = 1 + alpha;
				a1	= -2 * cs;
				a2 = 1 - alpha;
			};
			float out_r = (b0 * in_r + b1 * xn1_r + b2 * xn2_r - a1 * yn1_r - a2 * yn2_r) / a0;

			xn2_r = xn1_r;
			xn1_r = in_r;
			yn2_r = yn1_r;
			yn1_r = out_r;


			// Prevents clipping

			if (out_l < -1.0) 
				out_l = float(-1.0);
			else if (out_l > 1.0)
				out_l = float(1.0);                   
			
			*psamplesleft = out_l * 32767.0f;
			
			if (out_r < -1.0) 
				out_r = float(-1.0);
			else if (out_r > 1.0)
				out_r = float(1.0); 

			*psamplesright = out_r * 32767.0f;

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
			std::sprintf(txt,"%i°",value);
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
