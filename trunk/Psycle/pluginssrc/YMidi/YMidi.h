// mi.h: interface for the mi class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_mi_H__E6FCA427_B715_11D4_8B11_9FFC09C4A814__INCLUDED_)
#define AFX_mi_H__E6FCA427_B715_11D4_8B11_9FFC09C4A814__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define YMIDI_VERSION "1.0a"
#define NUMPARAMETERS 1
#define MAX_TRACKS 16					// Maximum tracks allowed.

#include <mmsystem.h>
#include <stdio.h>
#include "..\..\MachineInterface.h"

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


// =========================================
// Midi channel class - will need 16 of them
// =========================================
class midichannel
{
public:
	bool Playnote;		// when true plays the current note for this channel, false turns it off
	int  Chan;
	int	 Vol;
	int  Note;
	int	 Patch;

	void Play(const int note, const int vol);
	void Stop();
	int	 GetVolume();
	void Init(HMIDIOUT handle_in);
	void SetChannel(const int channel);
	void SetPatch(const int patchnum);

	midichannel();
	~midichannel();

private:
	HMIDIOUT handle;	// Midi device/channel handler.
	void StartMidi();
	void StopMidi();
	DWORD BuildEvent2(const int eventtype, const int channel, const int p1, const int p2);

};

CMachineParameter const prNothing = {
	"-",
	"Nothing",
	0,
	1,
	MPF_STATE,
	0
};

CMachineParameter const *pParameters[] = 
{ 
	&prNothing,
};

CMachineInfo const MacInfo =
{
	MI_VERSION,
	GENERATOR,
	NUMPARAMETERS,
	pParameters,
#ifdef _DEBUG
	"YMidi - Midi Out v." YMIDI_VERSION " (Debug)",
#else
	"YMidi - Midi Out v." YMIDI_VERSION,
#endif
	"YMidi" YMIDI_VERSION,
	"YanniS on " __DATE__,
	"Command Help",
	1
};

// =====================================
// mi plugin class
// =====================================
class mi : public CMachineInterface
{
public:
	float OutVol;		// Master volume. Values between 0 and 1!
	int Chan;
	bool Started;		// Used only to avoid New notes to start when Machine is muted.

//	void NoteOn(int note, chan int vol, int);
	void NoteOff();

	mi();
	virtual ~mi();

	virtual void Init();
	virtual void SequencerTick();
	virtual void Work(float *psamplesleft, float* psamplesright, int numsamples, int tracks);
	virtual bool DescribeValue(char* txt,int const param, int const value);
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SeqTick(int channel, int note, int ins, int cmd, int val);
	virtual void Stop();
	virtual void MidiNote(int const channel, int const value, int const velocity);

private:
	midichannel numChannel[MAX_TRACKS];	// List of MAX_TRACKS (16 usually) which hold channel note information
	int numC[MAX_TRACKS];	// Keep track of which channels are playing.
	int numchannels;
	bool midiopen;
	HMIDIOUT handle;

	void InitMidi();
	void CloseMidi();
};

#endif // !defined(AFX_mi_H__E6FCA427_B715_11D4_8B11_9FFC09C4A814__INCLUDED_)