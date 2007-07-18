#pragma once
#include <psycle/plugin_interface.hpp>
#include <windows.h>
#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#include <mmsystem.h>
	#pragma comment(lib, "winmm")
#pragma warning(pop)
#include <stdio.h>

#define YMIDI_VERSION "1.1"
#define NUMPARAMETERS 17
#define MIDI_TRACKS 16					// Maximum tracks allowed.

//===================================================================
// TWO BYTE PARAMETER MIDI EVENTS
#define MIDI_NOTEOFF		0x80        // note-off
#define MIDI_NOTEON			0x90        // note-on
#define MIDI_AFTERTOUCH		0xa0        // aftertouch
#define MIDI_CCONTROLLER	0xb0        // continuous controller
#define MIDI_PITCHWHEEL		0xe0        // pitch wheel
//===================================================================
// ONE BYTE PARAMETER MIDI EVENTS
#define MIDI_PATCHCHANGE	0xc0        // patch change
#define MIDI_CHANPRESSURE	0xd0		// channel pressure
//===================================================================


typedef	union
{
	struct split_mtype
	{
		unsigned char message;
		unsigned char byte1;
		unsigned char byte2;
		unsigned char byte3;
	} split;
	unsigned long value;
}MidiEvent;
// =========================================
// Midi channel class - will need 16 of them
// =========================================
class midichannel
{
public:
	midichannel();
	~midichannel();

	void Init(HMIDIOUT handle_in, const int channel);
	void Play(const int note, const int vol);
	void Stop(const int note);
	void SetPatch(const int patchnum);
	void StopMidi();

private:
	void SendMidi(MidiEvent event);
	MidiEvent BuildEvent(const int eventtype, const int p1=0, const int p2=0);

	int  Chan;
	HMIDIOUT handle;	// Midi device/channel handler.
};

typedef struct {
	int portidx;
	int patch1;
	int patch2;
	int patch3;
	int patch4;
	int patch5;
	int patch6;
	int patch7;
	int patch8;
	int patch9;
	int patch10;
	int patch11;
	int patch12;
	int patch13;
	int patch14;
	int patch15;
	int patch16;
}MacParams;
CMachineParameter const prPort = {"Output Port","Output Port",0,15,MPF_STATE,0};
CMachineParameter const prPatch1 = {"Program Channel 1","Program Channel 1",0,127,MPF_STATE,0};
CMachineParameter const prPatch2 = {"Program Channel 2","Program Channel 2",0,127,MPF_STATE,0};
CMachineParameter const prPatch3 = {"Program Channel 3","Program Channel 3",0,127,MPF_STATE,0};
CMachineParameter const prPatch4 = {"Program Channel 4","Program Channel 4",0,127,MPF_STATE,0};
CMachineParameter const prPatch5 = {"Program Channel 5","Program Channel 5",0,127,MPF_STATE,0};
CMachineParameter const prPatch6 = {"Program Channel 6","Program Channel 6",0,127,MPF_STATE,0};
CMachineParameter const prPatch7 = {"Program Channel 7","Program Channel 7",0,127,MPF_STATE,0};
CMachineParameter const prPatch8 = {"Program Channel 8","Program Channel 8",0,127,MPF_STATE,0};
CMachineParameter const prPatch9 = {"Program Channel 9","Program Channel 9",0,127,MPF_STATE,0};
CMachineParameter const prPatch10 = {"Program Channel 10","Program Channel 10",0,127,MPF_STATE,0};
CMachineParameter const prPatch11 = {"Program Channel 11","Program Channel 11",0,127,MPF_STATE,0};
CMachineParameter const prPatch12 = {"Program Channel 12","Program Channel 12",0,127,MPF_STATE,0};
CMachineParameter const prPatch13 = {"Program Channel 13","Program Channel 13",0,127,MPF_STATE,0};
CMachineParameter const prPatch14 = {"Program Channel 14","Program Channel 14",0,127,MPF_STATE,0};
CMachineParameter const prPatch15 = {"Program Channel 15","Program Channel 15",0,127,MPF_STATE,0};
CMachineParameter const prPatch16 = {"Program Channel 16","Program Channel 16",0,127,MPF_STATE,0};

CMachineParameter const *pParameters[] = 
{ 
	&prPort,
	&prPatch1,
	&prPatch2,
	&prPatch3,
	&prPatch4,
	&prPatch5,
	&prPatch6,
	&prPatch7,
	&prPatch8,
	&prPatch9,
	&prPatch10,
	&prPatch11,
	&prPatch12,
	&prPatch13,
	&prPatch14,
	&prPatch15,
	&prPatch16,
};

CMachineInfo const MacInfo =
{
	MI_VERSION,
	GENERATOR,
	NUMPARAMETERS,
	pParameters,
	"YMidi - Midi Out "
		"v. " YMIDI_VERSION
		#if !defined NDEBUG
			" (debug)"
		#endif
		,
	"YMidi" YMIDI_VERSION,
	"YanniS and JosepMa on " __DATE__,
	"Command Help",
	1
};

// =====================================
// mi plugin class
// =====================================
class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void ParameterTweak(int par, int val);
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples, int tracks);
	virtual void Stop();
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);

protected:
	void InitMidi();
	void FreeMidi();

private:
	midichannel numChannel[MIDI_TRACKS];	// List of MAX_TRACKS (16 usually) which hold channel note information
	int numC[MAX_TRACKS];					// Assignation of tracker track to midi channel.
	int notes[MAX_TRACKS];					// Last note being played in this track.
	MacParams pars;

	static int midiopencount;
	static HMIDIOUT handle;

};
