//============================================================================
//
//				FeedMe.cpp
//				----------
//				druttis@darkface.pp.se
//
//============================================================================
#include "CTrack.h"
#include <psycle/plugin_interface.hpp>
#include <memory>
#include <cstring>
//============================================================================
//				Defines
//============================================================================
#define MAC_NAME								"FeedMe"
#define MAC_VERSION								"1.1"
#define MAC_AUTHOR								"Druttis"
#define				MAX_TRACKS								64
#define MAX_VOICES								2
//============================================================================
//				Parameters
//============================================================================

#define PARAM_WAVEFORM 0
CMachineParameter const paramWaveform = {
	"Waveform",
	"Waveform",
	0,
	NUMWAVEFORMS - 1,
	MPF_STATE,
	0
};

#define PARAM_FEEDBACK 1
CMachineParameter const paramFeedBack = {
	"Feed-back",
	"Feed-back amount",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_OVERTONES 2
CMachineParameter const paramOvertones = {
	"Overtones",
	"Overtones",
	1,
	6,
	MPF_STATE,
	1
};

#define PARAM_PHASE 3
CMachineParameter const paramPhase = {
	"Phase",
	"Phase",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_CHORUS 4
CMachineParameter const paramChorus = {
	"Chorus",
	"Chorus",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_DISTFORM 5
CMachineParameter const paramDistform = {
	"Distform",
	"Distform",
	0,
	NUMWAVEFORMS - 1,
	MPF_STATE,
	0
};

#define PARAM_DISTORTION 6
CMachineParameter const paramDistortion = {
	"Distortion",
	"Distortion",
	0,
	510,
	MPF_STATE,
	0
};

#define PARAM_ATTACK 7
CMachineParameter const paramAttack = {
	"Attack",
	"Attack",
	0,
	255,
	MPF_STATE,
	1
};

#define PARAM_RELEASE 8
CMachineParameter const paramRelease = {
	"Release",
	"Release",
	0,
	255,
	MPF_STATE,
	1
};

#define PARAM_AMPLITUDE 9
CMachineParameter const paramAmplitude = {
	"Amplitude",
	"Amplitude",
	0,
	255,
	MPF_STATE,
	128
};

#define PARAM_VIBRATO_RATE 10
CMachineParameter const paramVibratoRate = {
	"Vibrato speed",
	"Vibrato speed",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_VIBRATO_AMOUNT 11
CMachineParameter const paramVibratoAmount = {
	"Vibrato depth",
	"Vibrato depth",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_VIBRATO_DELAY 12
CMachineParameter const paramVibratoDelay = {
	"Vibrato delay",
	"Vibrato delay",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_VCF_ATTACK 13
CMachineParameter const paramVcfAttack = {
	"VCF attack",
	"VCF attack",
	0,
	255,
	MPF_STATE,
	1
};

#define PARAM_VCF_DECAY 14
CMachineParameter const paramVcfDecay = {
	"VCF decay",
	"VCF decay",
	0,
	255,
	MPF_STATE,
	1
};

#define PARAM_VCF_SUSTAIN 15
CMachineParameter const paramVcfSustain = {
	"VCF sustain",
	"VCF sustain",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_VCF_RELEASE 16
CMachineParameter const paramVcfRelease = {
	"VCF release",
	"VCF release",
	0,
	255,
	MPF_STATE,
	1
};

#define PARAM_VCF_AMOUNT 17
CMachineParameter const paramVcfAmount = {
	"VCF amount",
	"VCF amount",
	-127,
	127,
	MPF_STATE,
	0
};

#define PARAM_FILTER_TYPE 18
CMachineParameter const paramFilterType = {
	"Filter type",
	"Filter type",
	0,
	4,
	MPF_STATE,
	0
};

#define PARAM_FILTER_FREQ 19
CMachineParameter const paramFilterFreq = {
	"Filter freq",
	"Filter freq",
	0,
	255,
	MPF_STATE,
	128
};

#define PARAM_FILTER_RES 20
CMachineParameter const paramFilterRes = {
	"Filter res",
	"Filter res",
	0,
	255,
	MPF_STATE,
	128
};

#define PARAM_INERTIA 21
CMachineParameter const paramInertia = {
	"Inertia",
	"Inertia",
	0,
	255,
	MPF_STATE,
	64
};

#define PARAM_NOTE_CUT 22
CMachineParameter const paramNoteCut = {
	"Note cut",
	"Note cut",
	0,
	255,
	MPF_STATE,
	0
};

#define PARAM_SYNC_MODE 23
CMachineParameter const paramSyncMode = {
	"Sync mode",
	"Sync mode",
	0,
	3,
	MPF_STATE,
	3
};
//============================================================================
//				Parameter list
//============================================================================
CMachineParameter const *pParams[] = {
	&paramWaveform,
	&paramFeedBack,
	&paramOvertones,
	&paramPhase,
	&paramChorus,
	&paramDistform,
	&paramDistortion,
	&paramAttack,
	&paramRelease,
	&paramAmplitude,
	&paramVibratoRate,
	&paramVibratoAmount,
	&paramVibratoDelay,
	&paramVcfAttack,
	&paramVcfDecay,
	&paramVcfSustain,
	&paramVcfRelease,
	&paramVcfAmount,
	&paramFilterType,
	&paramFilterFreq,
	&paramFilterRes,
	&paramInertia,
	&paramNoteCut,
	&paramSyncMode
};

#define NUM_PARAMS 24
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
	5
};

//============================================================================
//				Machine
//============================================================================
class mi : public CMachineInterface
{
private:
	static int instances;
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
	TRACKDATA globals;
	bool setup;
	CTrack				tracks[MAX_TRACKS][2];
	int								global_ticks_remaining;
};

PSYCLE__PLUGIN__INSTANCIATOR(mi, MacInfo)
//============================================================================
//				Constructor
//============================================================================
int mi::instances = 0;
//============================================================================
//				Constructor
//============================================================================
mi::mi()
{
	instances++;
	setup = true;
	Vals = new int[NUM_PARAMS];
	global_ticks_remaining = 0;
}
//============================================================================
//				Destructor
//============================================================================
mi::~mi()
{
	delete Vals;
	instances--;
	if (instances == 0)
		CTrack::Destroy();
}
//============================================================================
//				Init
//============================================================================
void mi::Init()
{
	//
	globals.samplingrate = pCB->GetSamplingRate();
	//
	for (int ti = 0; ti < MAX_TRACKS; ti++) {
		for (int vi = 0; vi < MAX_VOICES; vi++) {
			tracks[ti][vi].globals = &globals;
		}
	}
	//
	if (instances == 1)
		CTrack::Init();
}
//============================================================================
//				Stop
//============================================================================
void mi::Stop()
{
	for (int ti = 0; ti < MAX_TRACKS; ti++) {
		tracks[ti][0].NoteOff();
	}
	setup = true;
}
//============================================================================
//				Command
//============================================================================
void mi::Command()
{
	pCB->MessBox(
		"Tracker commands :\n"
		"\n"
		"0Cxx : Volume/Velocity (xx: 00 = silent, ff = max)\n"
		"0Dxx : Slide to note   (xx: 00 = default speed (10), 01 = fast, ff = slow)\n"
		"OExx : Cut note        (xx: speed) * cuts this note only\n"
		"\n"
		"Greets to the folks in #psycle & #musicdsp on efnet"
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
		case PARAM_WAVEFORM:
			globals.waveform = val;
			break;
		case PARAM_FEEDBACK:
			SetAFloat(&globals.feedback, (float) val * 2.0f);
			break;
		case PARAM_OVERTONES:
			globals.overtype = val - 1;
			break;
		case PARAM_PHASE:
			SetAFloat(&globals.phase, (float) val * 16.0f);
			break;
		case PARAM_CHORUS:
			globals.chorus = val / 4080.0f;
			break;
		case PARAM_DISTFORM:
			globals.distform = val;
			break;
		case PARAM_DISTORTION:
			SetAFloat(&globals.distortion, (float) val / 256.0f);
			break;
		case PARAM_ATTACK:
			globals.attack = (float) val * 64.0f;
			break;
		case PARAM_RELEASE:
			globals.release = (float) val * 64.0f;
			break;
		case PARAM_AMPLITUDE:
			globals.amplitude = (float) val / 255.0f;
			break;
		case PARAM_VIBRATO_RATE:
			globals.vibrato_rate = (float) val / 8.0f;
			break;
		case PARAM_VIBRATO_AMOUNT:
			globals.vibrato_amount = (float) val / 255.0f;
			break;
		case PARAM_VIBRATO_DELAY:
			globals.vibrato_delay = (float) val * 8.0f;
			break;
		case PARAM_VCF_ATTACK:
			globals.vcf_attack = (float) val * 2.0f;
			break;
		case PARAM_VCF_DECAY:
			globals.vcf_decay = (float) val * 2.0f;
			break;
		case PARAM_VCF_SUSTAIN:
			globals.vcf_sustain = (float) val / 255.0f;
			break;
		case PARAM_VCF_RELEASE:
			globals.vcf_release = (float) val * 2.0f;
			break;
		case PARAM_VCF_AMOUNT:
			SetAFloat(&globals.vcf_amount, (float) val / 127.0f);
			break;
		case PARAM_FILTER_TYPE:
			globals.filter_type = val;
			break;
		case PARAM_FILTER_FREQ:
			SetAFloat(&globals.filter_freq, (float) val / 255.0f);
			break;
		case PARAM_FILTER_RES:
			SetAFloat(&globals.filter_res, (float) val / 256.0f);
			break;
		case PARAM_INERTIA:
			globals.inertia = (val ? 1.0f / (float) val * 2.0f : 0.0f);
			break;
		case PARAM_NOTE_CUT:
			globals.note_cut = val * 2;
			break;
		case PARAM_SYNC_MODE:
			globals.sync_mode = val;
			break;
	}
}
//============================================================================
//				DescribeValue
//============================================================================
bool mi::DescribeValue(char* txt,int const param, int const value)
{
	switch (param)
	{
		case PARAM_FEEDBACK:
		case PARAM_DISTORTION:
		case PARAM_VCF_SUSTAIN:
		case PARAM_FILTER_RES:
		case PARAM_PHASE:
		case PARAM_CHORUS:
		case PARAM_AMPLITUDE:
		case PARAM_VIBRATO_RATE:
		case PARAM_VIBRATO_AMOUNT:
			sprintf(txt, "%.2f %%", (float) value * 100.0f / 255.0f);
			break;
		case PARAM_FILTER_FREQ:
			sprintf(txt, "%.2f Hz", (float) value * 22050.0f / 255.0f);
			break;
		case PARAM_VCF_AMOUNT:
			sprintf(txt, "%.2f %%", (float) value * 100.0f / 127.0f);
			break;
		case PARAM_ATTACK:
		case PARAM_RELEASE:
		case PARAM_VCF_ATTACK:
		case PARAM_VCF_DECAY:
		case PARAM_VCF_RELEASE:
			sprintf(txt, "%.2f ms", (float) value * 64000.0f / 44100.0f);
			break;
		case PARAM_VIBRATO_DELAY:
			sprintf(txt, "%.2f ms", (float) value * 256000.0f / 44100.0f);
			break;
		case PARAM_WAVEFORM:
		case PARAM_DISTFORM:
			switch (value)
			{
				case 0: sprintf(txt, "Sine"); break;
				case 1: sprintf(txt, "Triangle"); break;
				case 2: sprintf(txt, "Pulse"); break;
				case 3: sprintf(txt, "Ramp up"); break;
				case 4: sprintf(txt, "Ramp down"); break;
				case 5: sprintf(txt, "Pow 2"); break;
			}
			break;
		case PARAM_OVERTONES:
			switch (value)
			{
				case 1: sprintf(txt, "None"); break;
				case 2: sprintf(txt, "1"); break;
				case 3: sprintf(txt, "2 (exp)"); break;
				case 4: sprintf(txt, "3 (exp)"); break;
				case 5: sprintf(txt, "2 (linear)"); break;
				case 6: sprintf(txt, "3 (linear)"); break;
			}
			break;
		case PARAM_FILTER_TYPE:
			switch (value)
			{
				case 0: sprintf(txt, "LP 12"); break;
				case 1: sprintf(txt, "LP 24"); break;
				case 4: sprintf(txt, "LP 36"); break;
				case 2: sprintf(txt, "HP 12"); break;
				case 3: sprintf(txt, "HP 24"); break;
			}
			break;
		case PARAM_INERTIA:
			if (value == 0)
				sprintf(txt, "Instantantous");
			else
				sprintf(txt, "%.2f ms", (float) value * 64.0f / 44.1f);
			break;
		case PARAM_NOTE_CUT:
			if (value == 0)
				sprintf(txt, "Off");
			else
				sprintf(txt, "%.2f ms", (float) value * 64.0f / 44.1f);
			break;
		case PARAM_SYNC_MODE:
			switch (value)
			{
				case 0: sprintf(txt, "Off"); break;
				case 1: sprintf(txt, "Vib"); break;
				case 2: sprintf(txt, "VCE"); break;
				case 3: sprintf(txt, "Both"); break;
			}
			break;
		default:
			return false;
	}
	return true;
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
	//
	//
	int vol = 254;
	//
	//				Volume command
	if (cmd == 0x0c) {
		vol = val - 2;
		if (vol < 0)
			vol = 0;
	}
	//
	//				Route
	if (note == 120) {
		tracks[channel][0].NoteOff();
	} else if (note < 120) {
		if (cmd == 0x0d) {
			if (tracks[channel][0].IsFinished()) {
				tracks[channel][0].NoteOn(-note, vol);
			}
			if (val == 0)
				val = 16;
			tracks[channel][0].slide_speed = 1.0f / (float) val;
			tracks[channel][0].SetFreq(note);
			tracks[channel][0].note_cut = (cmd == 0x0e ? val : globals.note_cut);
		} else {
			tracks[channel][0].NoteOff();
			memcpy(&tracks[channel][1], &tracks[channel][0], sizeof(CTrack) * (MAX_VOICES - 1));
			tracks[channel][0].NoteOn(note, vol);
			tracks[channel][0].note_cut = (cmd == 0x0e ? val : globals.note_cut);
		}
	}
}
//============================================================================
//				Work
//============================================================================
void mi::Work(float *psamplesleft, float* psamplesright, int numsamples, int numtracks)
{
	//
	//				Animate inertia
	if (setup) {
		const float max = 0.0f;
		AnimateAFloat(&globals.feedback, max);
		AnimateAFloat(&globals.phase, max);
		AnimateAFloat(&globals.distortion, max);
		AnimateAFloat(&globals.vcf_amount, max);
		AnimateAFloat(&globals.filter_freq, max);
		AnimateAFloat(&globals.filter_res, max);
		setup = false;
	}
	//
	//				Loop
	int ti;
	int vi;
	//
	//
	--psamplesleft;
	--psamplesright;
	//
	//				Render
	do {
		//
		//				Handle tick;
		if (global_ticks_remaining <= 0)
		{
			global_ticks_remaining = GLOBAL_TICKS;
			float inertia = globals.inertia;
			AnimateAFloat(&globals.feedback, inertia);
			AnimateAFloat(&globals.phase, inertia);
			AnimateAFloat(&globals.distortion, inertia);
			AnimateAFloat(&globals.vcf_amount, inertia);
			AnimateAFloat(&globals.filter_freq, inertia);
			AnimateAFloat(&globals.filter_res, inertia);
		}
		//
		//				Compute samples to play
		int amount = numsamples;
		if (amount > global_ticks_remaining)
			amount = global_ticks_remaining;
		//
		//				Tick
		global_ticks_remaining -= amount;
		//
		//				Render voices
		for (ti = 0; ti < numtracks; ti++) {
			for (vi = 0; vi < MAX_VOICES; vi++) {
				if (!tracks[ti][vi].IsFinished())
					tracks[ti][vi].Work(psamplesleft, psamplesright, amount);
			}
		}
		//
		//				Advance
		psamplesleft += amount;
		psamplesright += amount;
		numsamples -= amount;
		//
		//				Next
	} while (numsamples > 0);
}
