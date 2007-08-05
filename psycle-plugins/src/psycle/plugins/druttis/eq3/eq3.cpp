//////////////////////////////////////////////////////////////////////
//
//	EQ5
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <packageneric/pre-compiled.private.hpp>
#include <psycle/plugin_interface.hpp>
#include "../dsp/Biquad.h"
//////////////////////////////////////////////////////////////////////
//
//	Machine Defs
//
//////////////////////////////////////////////////////////////////////
#define MAC_NAME "EQ-3"
#define MAC_VERSION "1.0"
#define MAC_AUTHOR "Druttis"
#define NUM_PARAM_COLS 4
//////////////////////////////////////////////////////////////////////
//
//	Machine Parameters
//
//////////////////////////////////////////////////////////////////////
#define PARAM_LSH_GAIN_LEFT 0
CMachineParameter const param_lsh_gain_left =
{
	"LSH - Gain Left",
	"LSH - Gain Left",
	0,
	256,
	MPF_STATE,
	128
};

#define PARAM_LSH_GAIN_RIGHT 1
CMachineParameter const param_lsh_gain_right =
{
	"LSH - Gain Right",
	"LSH - Gain Right",
	-1,
	256,
	MPF_STATE,
	-1
};

#define PARAM_LSH_FREQ 2
CMachineParameter const param_lsh_freq =
{
	"LSH - Freq",
	"LSH - Freq",
	0,
	4096,
	MPF_STATE,
	512
};

#define PARAM_PEQ_GAIN_LEFT 3
CMachineParameter const param_peq_gain_left =
{
	"PEQ - Gain Left",
	"PEQ - Gain Left",
	0,
	256,
	MPF_STATE,
	128
};

#define PARAM_PEQ_GAIN_RIGHT 4
CMachineParameter const param_peq_gain_right =
{
	"PEQ - Gain Right",
	"PEQ - Gain Right",
	-1,
	256,
	MPF_STATE,
	-1
};

#define PARAM_PEQ_FREQ 5
CMachineParameter const param_peq_freq =
{
	"PEQ - Freq",
	"PEQ - Freq",
	0,
	4096,
	MPF_STATE,
	1024
};

#define PARAM_HSH_GAIN_LEFT 6
CMachineParameter const param_hsh_gain_left =
{
	"HSH - Gain Left",
	"HSH - Gain Left",
	0,
	256,
	MPF_STATE,
	128
};

#define PARAM_HSH_GAIN_RIGHT 7
CMachineParameter const param_hsh_gain_right =
{
	"HSH - Gain Right",
	"HSH - Gain Right",
	-1,
	256,
	MPF_STATE,
	-1
};

#define PARAM_HSH_FREQ 8
CMachineParameter const param_hsh_freq =
{
	"HSH - Freq",
	"HSH - Freq",
	0,
	4096,
	MPF_STATE,
	2048
};

#define PARAM_MASTER_LEFT 9
CMachineParameter const param_master_left =
{
	"Master - Left",
	"Master - Left",
	0,
	256,
	MPF_STATE,
	128
};

#define PARAM_MASTER_RIGHT 10
CMachineParameter const param_master_right =
{
	"Master - Right",
	"Master - Right",
	-1,
	256,
	MPF_STATE,
	-1
};

#define PARAM_BASS_BOOST 11
CMachineParameter const param_bass_boost =
{
	"Bass boost",
	"Bass boost",
	0,
	1,
	MPF_STATE,
	0
};

#define NUM_PARAMS 12
//////////////////////////////////////////////////////////////////////
//
//	Machine Parameter List
//
//////////////////////////////////////////////////////////////////////
CMachineParameter const *pParams[] =
{
	&param_lsh_gain_left,
	&param_lsh_gain_right,
	&param_lsh_freq,
	&param_peq_gain_left,
	&param_peq_gain_right,
	&param_peq_freq,
	&param_hsh_gain_left,
	&param_hsh_gain_right,
	&param_hsh_freq,
	&param_master_left,
	&param_master_right,
	&param_bass_boost
};
//////////////////////////////////////////////////////////////////////
//
//	Machine Info
//
//////////////////////////////////////////////////////////////////////
CMachineInfo MacInfo =
{
	MI_VERSION,
	EFFECT,
	NUM_PARAMS,
	pParams,
#ifdef _DEBUG
	MAC_NAME "   v." MAC_VERSION " (Debug)",
#else
	MAC_NAME "   v." MAC_VERSION,
#endif
	MAC_NAME,
	MAC_AUTHOR " on " __DATE__,
	"Command Help",
	NUM_PARAM_COLS
};
//////////////////////////////////////////////////////////////////////
//
//	InitFilter Helper Function
//
//////////////////////////////////////////////////////////////////////
void InitFilter(Biquad *b, int type, int gain, int freq, int sr, float bw)
{
	b->Init(
		type,
		(float) (gain - 128) / 10.0f,
		12000.0f * (float) freq / 4096.0f + 20.0f,
		sr,
		bw
	);
}
//////////////////////////////////////////////////////////////////////
//
//	InitFilter Helper Function
//
//////////////////////////////////////////////////////////////////////
unsigned long randSeed = 22222; 
inline unsigned long GetRandomNumber()
{ 
	randSeed = (randSeed * 196314165) + 907633515; 
	return randSeed; 
}
inline double GetRandomSignal()
{
	return (float) ((int) GetRandomNumber()  & 0xffff) * 0.000030517578125 - 1.0;
}
//////////////////////////////////////////////////////////////////////
//
//	Machine Class
//
//////////////////////////////////////////////////////////////////////
class mi : public CMachineInterface
{
	//////////////////////////////////////////////////////////////////
	//
	//	Machine Variables
	//
	//////////////////////////////////////////////////////////////////
private:
	// Static Generic
	static int instances;

	// Instance variables
	Biquad bands[5][2];

	float master[2];
	//////////////////////////////////////////////////////////////////
	//
	//	Machine Methods
	//
	//////////////////////////////////////////////////////////////////
public:
	// Generic
	mi();
	virtual ~mi();
	virtual void Load();
	virtual void Unload();
	virtual void Command();
	virtual void Init();
	virtual void Stop();
	virtual void ParameterTweak(int par, int val);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int numtracks);
	// Custom
};
PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)
//////////////////////////////////////////////////////////////////////
//
//	Initialize Static Machine Variables
//
//////////////////////////////////////////////////////////////////////
// Static Generic
int mi::instances = 0;
//////////////////////////////////////////////////////////////////////
//
//	Machine Constructor
//
//////////////////////////////////////////////////////////////////////
mi::mi()
{
	Vals = new int[NUM_PARAMS];
	if (mi::instances == 0)
		Load();
	mi::instances++;
	Stop();
}
//////////////////////////////////////////////////////////////////////
//
//	Machine Destructor
//
//////////////////////////////////////////////////////////////////////
mi::~mi()
{
	delete[] Vals;
	mi::instances--;
	if (mi::instances == 0)
		Unload();
}
//////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////
void mi::Load()
{
}
//////////////////////////////////////////////////////////////////////
//
//	load
//
//////////////////////////////////////////////////////////////////////
void mi::Unload()
{
}
//////////////////////////////////////////////////////////////////////
//
//	Command
//
//////////////////////////////////////////////////////////////////////
void mi::Command()
{
	pCB->MessBox(
		"Decription"
		"\n"
		,
		MAC_AUTHOR " " MAC_NAME " v." MAC_VERSION
		,
		0
	);
}
//////////////////////////////////////////////////////////////////////
//
//	Init
//
//////////////////////////////////////////////////////////////////////
void mi::Init()
{
	Stop();
}
//////////////////////////////////////////////////////////////////////
//
//	Stop
//
//////////////////////////////////////////////////////////////////////
void mi::Stop()
{
	for (int i = 0; i < 3; i++)
	{
		bands[i][0].Reset();
		bands[i][1].Reset();
	}
}

//////////////////////////////////////////////////////////////////////
//
//	ParameterTweak
//
//////////////////////////////////////////////////////////////////////
void mi::ParameterTweak(int par, int val)
{
	Vals[par] = val;

	int left, right, freq;

	int sr = pCB->GetSamplingRate();

	switch (par)
	{
	case PARAM_LSH_GAIN_LEFT :
	case PARAM_LSH_GAIN_RIGHT :
	case PARAM_LSH_FREQ :
	case PARAM_BASS_BOOST :
		left = Vals[PARAM_LSH_GAIN_LEFT];
		right = Vals[PARAM_LSH_GAIN_RIGHT];
		freq = Vals[PARAM_LSH_FREQ];
		if (right == -1)
		{
			right = left;
		}
		if (Vals[PARAM_BASS_BOOST])
		{
			InitFilter(&bands[0][0], LSH, left, freq, sr, 1.0f);
			InitFilter(&bands[0][1], LSH, right, freq, sr, 1.0f);
		}
		else
		{
			InitFilter(&bands[0][0], LSH, left, freq, sr, 0.0f);
			InitFilter(&bands[0][1], LSH, right, freq, sr, 0.0f);
		}
		break;
	case PARAM_PEQ_GAIN_LEFT :
	case PARAM_PEQ_GAIN_RIGHT :
	case PARAM_PEQ_FREQ :
		left = Vals[PARAM_PEQ_GAIN_LEFT];
		right = Vals[PARAM_PEQ_GAIN_RIGHT];
		freq = Vals[PARAM_PEQ_FREQ];
		if (right == -1)
		{
			right = left;
		}
		InitFilter(&bands[1][0], PEQ, left, freq, sr, 0.85f);
		InitFilter(&bands[1][1], PEQ, right, freq, sr, 0.85f);
		break;
	case PARAM_HSH_GAIN_LEFT :
	case PARAM_HSH_GAIN_RIGHT :
	case PARAM_HSH_FREQ :
		left = Vals[PARAM_HSH_GAIN_LEFT];
		right = Vals[PARAM_HSH_GAIN_RIGHT];
		freq = Vals[PARAM_HSH_FREQ];
		if (right == -1)
		{
			right = left;
		}
		InitFilter(&bands[2][0], HSH, left, freq, sr, 0.0f);
		InitFilter(&bands[2][1], HSH, right, freq, sr, 0.0f);
		break;
	case PARAM_MASTER_LEFT :
	case PARAM_MASTER_RIGHT :
		left = Vals[PARAM_MASTER_LEFT];
		right = Vals[PARAM_MASTER_RIGHT];
		if (right == -1)
		{
			right = left;
		}
		master[0] = (float) left / 256.0f;
		master[1] = (float) right / 256.0f;
		break;
	}
}
//////////////////////////////////////////////////////////////////////
//
//	DescribeValue
//
//////////////////////////////////////////////////////////////////////
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch (param)
	{
	case PARAM_LSH_GAIN_LEFT :
	case PARAM_LSH_GAIN_RIGHT :
	case PARAM_PEQ_GAIN_LEFT :
	case PARAM_PEQ_GAIN_RIGHT :
	case PARAM_HSH_GAIN_LEFT :
	case PARAM_HSH_GAIN_RIGHT :
		if (value > -1)
		{
			sprintf(txt, "%.2f dB", (float) (value - 128) / 10.0f);
		}
		else
		{
			sprintf(txt, "(left)");
		}
		return true;
	case PARAM_LSH_FREQ :
	case PARAM_PEQ_FREQ :
	case PARAM_HSH_FREQ :
		sprintf(txt, "%.2f Hz", (float) (value * 12000) / 4096.0f + 20.0f);
		return true;
	case PARAM_MASTER_LEFT :
	case PARAM_MASTER_RIGHT :
		if (value > -1)
		{
			sprintf(txt, "%.2f %%", (float) (value * 100) / 256.0f);
		}
		else
		{
			sprintf(txt, "(left)");
		}
		return true;
	case PARAM_BASS_BOOST :
		sprintf(txt, "%s", (value ? "on" : "off"));
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////
//
//	SequencerTick
//
//////////////////////////////////////////////////////////////////////
void mi::SequencerTick()
{
}
//////////////////////////////////////////////////////////////////////
//
//	Work
//
//////////////////////////////////////////////////////////////////////
void mi::Work(float *psamplesleft, float *psamplesright, int numsamples, int numtracks)
{
	float il, ir;
	float rnd;
	--psamplesleft;
	--psamplesright;
	do
	{
		rnd = (float) GetRandomSignal() * 0.001f;
		il = *++psamplesleft + rnd;
		*psamplesleft = (bands[0][0].Next(il) +
						 bands[1][0].Next(il) +
						 bands[2][0].Next(il))
						 * master[0];
		ir = *++psamplesright + rnd;
		*psamplesright = (bands[0][1].Next(ir) +
						  bands[1][1].Next(ir) +
						  bands[2][1].Next(ir))
						  * master[1];
	}
	while (--numsamples);
}
