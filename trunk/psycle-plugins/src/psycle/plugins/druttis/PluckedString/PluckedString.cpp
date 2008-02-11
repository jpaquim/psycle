// -*- mode:c++; indent-tabs-mode:t -*-
//============================================================================
//
//				WGFlute.cpp
//				----------
//				druttis@darkface.pp.se
//
//============================================================================
#include <packageneric/pre-compiled.private.hpp>
#include "CVoice.h"
#include <psycle/plugin_interface.hpp>
#include <memory>
//============================================================================
//				Defines
//============================================================================
#define MAC_NAME				"Plucked String"
#define MAC_VERSION				"1.1"
#define MAC_AUTHOR				"Druttis"
#define				MAX_TRACKS				64
#define				MAX_VOICES				2
#define NUM_TICKS				32
//============================================================================
//				Parameters
//============================================================================

#define PARAM_PLUCK_POS 0
CMachineParameter const paramPluckPos = {
	"Pluck Position",
	"Pluck Position",
	0,
	255,
	MPF_STATE,
	32
};

#define PARAM_DAMPING 1
CMachineParameter const paramDamping = {
	"Damping",
	"Damping",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_VOLUME 2
CMachineParameter const paramVolume = {
	"Volume",
	"Volume",
	0,
	255,
	MPF_STATE,
	128
};

#define PARAM_VIB_SPEED 3
CMachineParameter const paramVibSpeed = {
	"Vibrato Speed",
	"Vibrato Speed",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_VIB_AMOUNT 4
CMachineParameter const paramVibAmount = {
	"Vibrato Amount",
	"Vibrato Amount",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_VIB_DELAY 5
CMachineParameter const paramVibDelay = {
	"Vibrato Delay",
	"Vibrato Delay",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_SCHOOLNESS 6
CMachineParameter const paramSchoolness = {
	"Schoolness",
	"Schoolness",
	0,
	255,
	MPF_STATE,
	0
};
//============================================================================
//				Parameter list
//============================================================================
CMachineParameter const *pParams[] = {
	&paramPluckPos,
	&paramDamping,
	&paramVolume,
	&paramVibSpeed,
	&paramVibAmount,
	&paramVibDelay,
	&paramSchoolness
};

#define NUM_PARAMS 7
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
	bool				initialized;
	bool				working;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)
//============================================================================
//				Constructor
//============================================================================
mi::mi()
{
	working = false;
	initialized = false;
	Vals = new int[NUM_PARAMS];
	ticks_remaining = NUM_TICKS;
}
//============================================================================
//				Destructor
//============================================================================
mi::~mi()
{
	initialized = false;
	while (working);
	delete Vals;
}
//============================================================================
//				Init
//============================================================================
void mi::Init()
{
	globals.srate = pCB->GetSamplingRate();
	for (int ti = 0; ti < MAX_TRACKS; ti++) {
		for (int vi = 0; vi < MAX_VOICES; vi++) {
			voices[ti][vi].globals = &globals;
			voices[ti][vi].Init();
		}
	}
	initialized = true;
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
}
//============================================================================
//				Command
//============================================================================
void mi::Command()
{
	pCB->MessBox(
		"Tracker commands:\n\n"
		"0Cxx : Volume / Velocity\n"
		"0Dxx : Glide\n"
		"0Exx : Note delay\n"
		,
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
	switch (par)
	{
		case PARAM_PLUCK_POS:
			globals.pluckPos = (float) val / 255.0f;
			break;
		case PARAM_DAMPING:
			globals.damping = 0.97f + (1.0f - (float) val / 255.0f) * 0.03f;
			break;
		case PARAM_VOLUME:
			globals.volume = (float) val * 64.0f;
			break;
		case PARAM_VIB_SPEED:
			globals.vib_speed = (float) val / 255.0f * NUM_TICKS * 10.0f / globals.srate;
			break;
		case PARAM_VIB_AMOUNT:
			globals.vib_amount = (float) val / 3060.0f;
			break;
		case PARAM_VIB_DELAY:
			globals.vib_delay = (float) val / 255.0f * globals.srate / NUM_TICKS * 2.0f;
			break;
		case PARAM_SCHOOLNESS:
			globals.schoolness = (float) val / 510.0f;
			break;
	}
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
	//
	//				Update some stuf
	globals.srate = pCB->GetSamplingRate();
}
//============================================================================
//				SequencerTick
//============================================================================
void mi::SeqTick(int channel, int note, int ins, int cmd, int val)
{
	int vol = 254;
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
		if ((cmd != 0x0d) || (voices[channel][0].currentFreq == -1)) {
			voices[channel][0].NoteOff();
//												int size = sizeof(CVoice) * (MAX_VOICES - 1);
//												if (size > 0) {
//																memcpy(&voices[channel][1], &voices[channel][0], size);
//												}
		}
		voices[channel][0].NoteOn(note, vol, cmd, val);
		voices[channel][0].ticks_remaining = 0;
	}
}
//============================================================================
//				Work
//============================================================================
void mi::Work(float *psamplesleft, float *psamplesright, int numsamples, int numtracks)
{
	working = true;
	if (initialized) {
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
		//
		//				Loop
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
	//												for (vi = MAX_VOICES - 1; vi >= 0; vi--) {
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
						} while (nsamples > 0);
					}
				}
			}
			//
			//				Adjust for next iteration
			ticks_remaining -= amount;
			psamplesleft += amount;
			psamplesright += amount;
			numsamples -= amount;
		} while (numsamples > 0);
		//
		//				Possible effects may be coded here
	}
	working = false;
}
