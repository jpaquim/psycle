/////////////////////////////////////////////////////////////////////
// Dmitry "Sartorius" Kulikov stk Plucked plugin for PSYCLE
// v0.2a
//
// Based on The Synthesis ToolKit in C++ (STK)
// By Perry R. Cook and Gary P. Scavone, 1995-2004.
// http://ccrma.stanford.edu/software/stk/

#include <project.h>
#include <psycle/plugin/machineinterface.h>
#include <stk/psyStk.h>
#include <stk/psyPlucked.h>
#include <stdlib.h>

#define MAX_ENV_TIME	250000
#define MAX_TRACKS	32
#define NUMPARAMETERS 1

/*
CMachineParameter const paraTuner = 
{
	"Detune",
	"Detune",									// description
	1,											// MinValue	
	32767,											// MaxValue
	MPF_STATE,										// Flags
	1
};

*/
CMachineParameter const paraVolume = 
{
	"Volume",
	"Volume",									// description
	0,											// MinValue	
	32767,											// MaxValue
	MPF_STATE,										// Flags
	32767
};

CMachineParameter const *pParameters[] = 
{ 
	&paraVolume
};


CMachineInfo const MacInfo = 
{
	MI_VERSION,	
	GENERATOR,								// flags
	NUMPARAMETERS,							// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	"stk Plucked (Debug build)",		// name
#else
	"stk Plucked ",						// name
#endif
	"stk Plucked",							// short name
	"Sartorius, Bohan and STK 4.2.0 developers",							// author
	"Help",									// A command, that could be use for open an editor, etc...
	4
};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples,int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
	virtual void Stop();

private:

	Plucked track[MAX_TRACKS];
	bool noteonoff[MAX_TRACKS];


};

DLL_EXPORTS		// To export DLL functions to host

mi::mi()
{
	Vals=new int[NUMPARAMETERS];
}

mi::~mi()
{
	// Destroy dinamically allocated objects/memory here
	delete Vals;
}

void mi::Init()
{
// Initialize your stuff here
	Stk::setSampleRate(44100);
	for(int i=0;i<MAX_TRACKS;i++)
	{
		track[i].clear();
		track[i].noteOff(0.0);
		noteonoff[i]=true;
	}
}

void mi::Stop()
{
	for(int c=0;c<MAX_TRACKS;c++)
	{
		track[c].noteOff(0.0);
		track[c].clear();
		noteonoff[c]=false;
	}
}

void mi::SequencerTick()
{
// Called on each tick while sequencer is playing
}

void mi::ParameterTweak(int par, int val)
{
	// Called when a parameter is changed by the host app / user gui
	Vals[par]=val;
}

void mi::Command()
{
// Called when user presses editor button
// Probably you want to show your custom window here
// or an about button
char buffer[2048];

sprintf(
		buffer,"%s",
		"Simple Plucked\n"
		);

pCB->MessBox(buffer,"stk Plucked",0);

}

// Work... where all is cooked 
void mi::Work(float *psamplesleft, float *psamplesright , int numsamples,int tracks)
{
	float sl=0;
//	float sr=0;
	float const vol=(float)Vals[0];
	for(int c=0;c<tracks;c++)
	{
		if(noteonoff[c])
		{
			float *xpsamplesleft=psamplesleft;
			float *xpsamplesright=psamplesright;
			--xpsamplesleft;
			--xpsamplesright;

			int xnumsamples=numsamples;

			Plucked *ptrack=&track[c];
					
			do
				{
					
					sl=(float)ptrack->tick()*vol;
					if (sl<-vol)sl=-vol;
					if (sl>vol)sl=vol;
					*++xpsamplesleft+=sl;
					*++xpsamplesright+=sl;
				} while(--xnumsamples);
		}
	}

}

// Function that describes value on client's displaying
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	return false;
}

//////////////////////////////////////////////////////////////////////
// The SeqTick function where your notes and pattern command handlers
// should be processed. Called each tick.
// Is called by the host sequencer
	
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	// Note Off			== 120
	// Empty Note Row	== 255
	// Less than note off value??? == NoteON!

	// Note off
	if(note==120)
	{
		track[channel].noteOff(0.0);
		noteonoff[channel]=false;
	}
	else
	{
		StkFloat const offset(-36.3763165623); // 6 * 12 - 3 - 12 * ln(440) / ln(2)
		StkFloat const frequency = std::pow(2., (note - offset) / 12);
		track[channel].noteOn(frequency,1.0);
		noteonoff[channel]=true;
	}
	track[channel].tick();
}
