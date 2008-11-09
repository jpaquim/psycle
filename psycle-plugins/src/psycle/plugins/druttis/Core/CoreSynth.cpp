//============================================================================
//
//				WGFlute.cpp
//				----------
//				druttis@darkface.pp.se
//
//============================================================================
#include <memory.h>
#include "../MachineInterface2.h"
#include "CVoice.h"
//============================================================================
//				Defines
//============================================================================
#define MAC_NAME				"WGFlute"
#define MAC_VERSION				"1.0"
#define MAC_AUTHOR				"Druttis"
#define				MAX_TRACKS				32
#define				MAX_VOICES				2
#define NUM_TICKS				32
//============================================================================
//				Parameters
//============================================================================

#define PARAM_DUMMY1 0
CMachineParameter const paramDummy1 = {
	"Dummy1",
	"Dummy1",
	0,
	1,
	MPF_STATE,
	0
};
//============================================================================
//				Parameter list
//============================================================================
CMachineParameter const *pParams[] = {
	&paramDummy1
};

#define NUM_PARAMS 1
//============================================================================
//				Machine info
//============================================================================
CMachineInfo const MacInfo =
{
	MI_VERSION,
	GENERATOR,
	NUM_PARAMS,
	pParams,
#ifdef _DEBUG
	MAC_NAME " " MAC_VERSION " (Debug)",
#else
	MAC_NAME " " MAC_VERSION,
#endif
	MAC_NAME,
	MAC_AUTHOR " on " __DATE__,
	"Command Help",
	1
};
//============================================================================
//				Machine
//============================================================================
class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();
	virtual void Init();
	virtual void Stop();
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void SequencerTick();
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples, int numtracks);
public:
	GLOBALS globals;
	CVoice				voices[MAX_TRACKS][MAX_VOICES];
	int								ticks_remaining;
};

DLL_EXPORTS
//============================================================================
//				Constructor
//============================================================================
mi::mi()
{
	Vals = new int[NUM_PARAMS];
	for (int ti = 0; ti < MAX_TRACKS; ti++) {
		for (int vi = 0; vi < MAX_VOICES; vi++)
			voices[ti][vi].globals = &globals;
	}
	Stop();
}
//============================================================================
//				Destructor
//============================================================================
mi::~mi()
{
	Stop();
	delete Vals;
}
//============================================================================
//				Init
//============================================================================
void mi::Init()
{
}
//============================================================================
//				Stop
//============================================================================
void mi::Stop()
{
	for (int ti = 0; ti < MAX_TRACKS; ti++) {
		for (int vi = 0; vi < MAX_VOICES; vi++)
			voices[ti][vi].Stop();
	}
	ticks_remaining = NUM_TICKS;
}
//============================================================================
//				Command
//============================================================================
void mi::Command()
{
	pCB->MessBox(
		"Yo!",
		MAC_AUTHOR " " MAC_NAME " v." MAC_VERSION,
		0
	);
}
//============================================================================
//				ParameterTweak
//============================================================================
void mi::ParameterTweak(int par, int val)
{
	Vals[par] = val;
}
//============================================================================
//				DescribeValue
//============================================================================
bool mi::DescribeValue(char* txt,int const param, int const value) {
	return false;
}
//============================================================================
//				SequencerTick
//============================================================================
void mi::SequencerTick()
{
	//				Code here if this machine is an effect
}
//============================================================================
//				SequencerTick
//============================================================================
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	int vol = 254;
	//
	//				Update some stuf
	globals.samplingrate = pCB->GetSamplingRate();
	//
	//				Handle commands
	switch (cmd)
	{
		case 0x0c:				//				Volume
			vol = val - 2;
			if (vol < 0)
				vol = 0;
			break;
	}
	//
	//				Route
	if (note == 120) {
		voices[channel][0].NoteOff();
	} else if (note < 120) {
		voices[channel][0].NoteOff();
		memcpy(&voices[channel][1], &voices[channel][0], sizeof(CVoice) * (MAX_VOICES - 1));
		voices[channel][0].NoteOn(note, vol);
		voices[channel][0].ticks_remaining = 0;
	}
}
//============================================================================
//				Work
//============================================================================
void mi::Work(float *psamplesleft, float* psamplesright, int numsamples, int numtracks)
{
	//
	//				Variables
	int ti;
	int vi;
	int amount;
	int amt;
	int nsamples;
	float *pleft;
	float *pright;
	//
	//				Adjust sample pointers
	--psamplesleft;
	--psamplesright;
	do {
		//
		//				Global tick handling
		if (!ticks_remaining) {
			ticks_remaining = NUM_TICKS;
			CVoice::GlobalTick();
		}
		//
		//				Compute amount of samples to render for all voices
		amount = numsamples;
		if (amount > ticks_remaining)
			amount = ticks_remaining;
		//
		//				Render all voices now
		for (ti = 0; ti < numtracks; ti++) {
			for (vi = 0; vi < MAX_VOICES; vi++) {
				if (voices[ti][vi].IsActive()) {
					pleft = psamplesleft;
					pright = psamplesright;
					nsamples = amount;
					do {
						//
						//				Voice tick handing
						if (!voices[ti][vi].ticks_remaining) {
							voices[ti][vi].ticks_remaining = NUM_TICKS;
							voices[ti][vi].VoiceTick();
						}
						//
						//				Compute amount of samples to render this voice
						amt = nsamples;
						if (amt > voices[ti][vi].ticks_remaining)
							amt = voices[ti][vi].ticks_remaining;
						//
						//				Render voice now
						voices[ti][vi].Work(pleft, pright, amt);
						//
						//				Adjust for next voice iteration
						voices[ti][vi].ticks_remaining -= amt;
						pleft += amt;
						pright += amt;
						nsamples -= amt;
					} while (nsamples);
				}
			}
		}
		//
		//				Adjust for next iteration
		ticks_remaining -= amount;
		psamplesleft += amount;
		psamplesright += amount;
		numsamples -= amount;
	} while (numsamples);
	//
	//				Possible effects may be coded here
}
