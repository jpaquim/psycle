///////////////////////////////////////////////////////////////////////////////////////////////////
// MidiInput.h : CMidiInput class
//
// 21st April 2001 - v2.2b

#ifndef	_MIDIINPUT_H
#define	_MIDIINPUT_H

#include <mmsystem.h>
#include <assert.h>
#include "constants.h"
#include "songstructs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Defines

#define BYTES_PER_SAMPLE 4				// make sure this never gets changed in WaveOut/DSound!

#define	MAX_MIDI_CHANNELS	16
#define	MAX_CONTROLLERS		127
#define	MAX_PARAMETERS		127

#define MIDI_BUFFER_SIZE	1024		// TODO: might want to make these two user-configurable rather
#define	MIDI_PREDELAY_MS	200			// than hard-coded values.

#define	VERSION_STRING		"v2.2b"

// FLAGS
#define	FSTAT_ACTIVE			0x0001			// are we waiting to inject MIDI?
#define	FSTAT_FASTART			0x0002			// the three MIDI sync codes
#define	FSTAT_F8CLOCK			0x0004
#define	FSTAT_FCSTOP			0x0008

#define	FSTAT_EMULATED_FASTART	0x0010			// Controller 121 - emulated FASTART
#define	FSTAT_EMULATED_F8CLOCK	0x0020			// Controller 122 - emulated F8CLOCK
#define	FSTAT_EMULATED_FCSTOP	0x0040			// Controller 124 - emulated FCSTOP
#define	FSTAT_BUFFER_OVERFLOW	0x0080			// midi buffer overflow?

#define	FSTAT_IN_BUFFER			0x0100			// midi data going into the buffer?
#define	FSTAT_OUT_BUFFER		0x0200			// midi data going out of the buffer?
#define	FSTAT_CLEAR_BUFFER		0x0400			// midi data being cleared from the buffer?
#define	FSTAT_SYNC_TICK			0x0800			// simulated tracker tick?

#define	FSTAT_SYNC				0x1000			// syncing with the audio?
#define	FSTAT_RESYNC			0x2000			// just resynced with the audio?
#define	FSTAT_MIDI_INPUT		0x4000			// we are receiving MIDI input data?

// (AND with this to clear correct flags)
#define	FSTAT_CLEAR_WHEN_READ	0x0001

///////////////////////////////////////////////////////////////////////////////////////////////////
// Structures
//

struct MIDI_BUFFER
{
	PatternEntry entry;		// tracker pattern info struct
	DWORD timeStamp;		// MIDI input device's timestamp
	int channel;			// MIDI channel
};

struct MIDI_CONFIG
{
	char versionStr[ 16 ];	// version string (e.g. v2.0b)
	int	midiHeadroom;		// milliseconds allowed for MIDI buffer fill
};

struct MIDI_STATS
{
	int syncEventLatency;	// error (latency) of the last sync event (samples)
	int syncAdjuster;		// last sync adjustment value (samples)
	int bufferCount;		// amount of MIDI messages currently in the buffer (events)
	int bufferSize;			// capacity of the MIDI message buffer (events)
	int eventsLost;			// how many events have been lost and not played? (events)
	int syncOffset;			// how far off the audio engine is from the MIDI (ms)
	unsigned int channelMap;// bitmapped channel active map	(CLEAR AFTER READ)
	bool channelMapUpdate;	// strobe for the channel map list
	DWORD flags;			// 32 bits of boolean info (see FLAGS, CLEAR AFTER READ)
};

enum MODES
{
	MODE_REALTIME,			// use Psycle as a cool softsynth via MIDI
	MODE_STEP,				// enter pattern notes using MIDI
	MAX_MODES
};

enum DRIVERS
{
	DRIVER_MIDI,			// the main MIDI input driver
	DRIVER_SYNC,			// the driver for MIDI Sync
	MAX_DRIVERS
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CMidiInput
//
// DESCRIPTION: MIDI Input handler class

class CMidiInput
{
public:
	CMidiInput();
	virtual	~CMidiInput();

	static CMidiInput * Instance( void ){ return s_Instance; };	// return the current instance

	void SetDeviceId( int driver, UINT devId );	// set MIDI input device identifier
	bool Open( void );				// open the midi input devices
	bool Sync( void );				// start input (reset timestamps)
	void ReSync( void );			// resync the MIDI with the audio engine
	bool Close( void );				// close the midi input device

	bool Active( void ){ return m_midiInHandle!=NULL; };		// find out if we are open

	MIDI_STATS * GetStatsPtr( void ){ return &m_stats; };		// for external access
	MIDI_CONFIG * GetConfigPtr( void ){ return &m_config; };	// for external access

	int GetNumDevices( void );	// returns the number of midi devices on the system
	UINT FindDevByName( CString nameString );	// convert a name identifier into a index identifier (or -1 if fail)
	UINT PopulateListbox( CComboBox * listbox );	// fill a listbox with a list of the available input devices

	HMIDIIN GetHandle( int driver ){ assert( driver < MAX_DRIVERS ); return m_midiInHandle[ driver ]; };	// return the current device handle

	void SetInstMap( int machine, int inst );	// set a instrument map
	int GetInstMap( int machine );	// get the mapped instrument for the given machine

	void SetGenMap( int channel, int generator );	// set a instrument map
	int GetGenMap( int channel );	// get the mapped instrument for the given machine

	void SetControllerMap( int channel, int controller, int parameter );	// set a controller map
	int GetControllerMap( int channel, int controller );	// get a controller map

	bool GetNoteOffStatus( int channel );	// get the channel's note off status

	bool InjectMIDI( int amount );	// called to inject MIDI data

	int m_midiMode;	// the master MIDI handler mode (see enum MODES), external objects can change this at will

private:
	// the midi callback functions (just a static linker to the instance one)
	static void CALLBACK fnMidiCallbackStatic( HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );

	// the real callbacks
	void CALLBACK fnMidiCallback_Inject( HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );
	void CALLBACK fnMidiCallback_Step( HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 );

	void InternalReSync( DWORD dwParam2 );	// interal engine resync process
	void InternalClock( DWORD dwParam2 );

	static CMidiInput * s_Instance;	// the current instance pointer

	HMIDIIN m_midiInHandle[ MAX_DRIVERS ];	// current input device handles
	bool m_midiInHandlesTried;	// for once-only tell of problems

	UINT m_channelInstMap[ MAX_MACHINES ];				// channel->instrument map
	UINT m_channelGeneratorMap[ MAX_MIDI_CHANNELS ];	// channel->generator map
	bool m_channelNoteOff[ MAX_MIDI_CHANNELS ];			// channel, note off setting
	int m_channelController[ MAX_MIDI_CHANNELS ][ MAX_CONTROLLERS ];		// channel->controller->parameter map

	// audio-engine timing vars
	DWORD m_timingCounter;
	DWORD m_timingAccumulator;
	DWORD m_prevTimingCounter;
	DWORD m_baseStampTime;	// the base sync stamp
	DWORD m_tickBase;

	int m_devId[ MAX_DRIVERS ];		// midi device identifiers

	MIDI_CONFIG	m_config;	// configuration information
	MIDI_STATS	m_stats;	// statistics information
	
	MIDI_BUFFER m_midiBuffer[ MIDI_BUFFER_SIZE ];	// midi buffer

	int m_channelSetting[ MAX_MIDI_CHANNELS ];	// used in setting a controller map

	// buffer indexes
	int m_patIn;
	int m_patOut;
	int m_patCount;

	bool m_reSync;			// external telling us we need a resync
	bool m_synced;			// have we been synced with the audio engine yet?
	bool m_syncing;			// are we in the process of syncing?

	// syncronization variables
	DWORD m_baseStampTimeFix;
	int m_fixOffset;
	int m_lastPlayPos;
	int m_wraps;
	int m_adjustedPlayPos;	// adjusted play position
};

#endif