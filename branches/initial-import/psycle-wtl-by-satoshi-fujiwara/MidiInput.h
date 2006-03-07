/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

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
#include "IMidiInput.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Defines

#define BYTES_PER_SAMPLE 4				// make sure this never gets changed in WaveOut/DSound!

#define	MAX_MIDI_CHANNELS	16
#define	MAX_CONTROLLERS		127
#define	MAX_PARAMETERS		127

#define MIDI_BUFFER_SIZE	1024		// TODO: might want to make these two user-configurable rather
#define	MIDI_PREDELAY_MS	200			// than hard-coded values.

#define	VERSION_STRING		_T("v2.2b")

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
	TCHAR versionStr[ 16 ];	// version string (e.g. v2.0b)
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

#include "iCompornentConfiguration.h"
namespace configuration  {
	class MidiInput : public iCompornentConfiguration {
	public:
		MidiInput(){
			_midiDriverIndex = 0;			// MIDI IMPLEMENTATION
			_syncDriverIndex = 0;
			_midiHeadroom = 100;

			_midiRecordVel = FALSE;
			_midiTypeVel = 0;
			_midiCommandVel = 0x0c;
			_midiFromVel = 0x00;
			_midiToVel = 0xff;

			_midiRecordPit = FALSE;
			_midiTypePit = 0;
			_midiCommandPit = 1;
			_midiFromPit = 0;
			_midiToPit = 0xff;

			_midiRecord0 = FALSE;
			_midiType0 = 0;
			_midiMessage0 = 1;
			_midiCommand0 = 1;
			_midiFrom0 = 0;
			_midiTo0 = 0xff;

			_midiRecord1 = FALSE;
			_midiType1 = 0;
			_midiMessage1 = 2;
			_midiCommand1 = 2;
			_midiFrom1 = 0;
			_midiTo1 = 0xff;

			_midiRecord2= FALSE;
			_midiType2 = 0;
			_midiMessage2 = 3;
			_midiCommand2 = 3;
			_midiFrom2 = 0;
			_midiTo2 = 0xff;

			_midiRecord3 = FALSE;
			_midiType3 = 0;
			_midiMessage3 = 4;
			_midiCommand3 = 4;
			_midiFrom3 = 0;
			_midiTo3 = 0xff;

			_midiRecord4 = FALSE;
			_midiType4 = 0;
			_midiMessage4 = 5;
			_midiCommand4 = 5;
			_midiFrom4 = 0;
			_midiTo4 = 0xff;

			_midiRecord5 = FALSE;
			_midiType5 = 0;
			_midiMessage5 = 6;
			_midiCommand5 = 6;
			_midiFrom5 = 0;
			_midiTo5 = 0xff;

			_midiRecord6 = FALSE;
			_midiType6 = 0;
			_midiMessage6 = 7;
			_midiCommand6 = 7;
			_midiFrom6 = 0;
			_midiTo6 = 0xff;

			_midiRecord7 = FALSE;
			_midiType7 = 0;
			_midiMessage7 = 8;
			_midiCommand7 = 8;
			_midiFrom7 = 0;
			_midiTo7 = 0xff;

			_midiRecord8 = FALSE;
			_midiType8 = 0;
			_midiMessage8 = 9;
			_midiCommand8 = 9;
			_midiFrom8 = 0;
			_midiTo8 = 0xff;

			_midiRecord9 = FALSE;
			_midiType9 = 0;
			_midiMessage9 = 10;
			_midiCommand9 = 10;
			_midiFrom9 = 0;
			_midiTo9 = 0xff;

			_midiRecord10 = FALSE;
			_midiType10 = 0;
			_midiMessage10 = 11;
			_midiCommand10 = 11;
			_midiFrom10 = 0;
			_midiTo10 = 0xff;

			_midiRecord11 = FALSE;
			_midiType11 = 0;
			_midiMessage11 = 12;
			_midiCommand11 = 12;
			_midiFrom11 = 0;
			_midiTo11 = 0xff;

			_midiRecord12 = FALSE;
			_midiType12 = 0;
			_midiMessage12 = 13;
			_midiCommand12 = 13;
			_midiFrom12 = 0;
			_midiTo12 = 0xff;

			_midiRecord13 = FALSE;
			_midiType13 = 0;
			_midiMessage13 = 14;
			_midiCommand13 = 14;
			_midiFrom13 = 0;
			_midiTo13 = 0xff;

			_midiRecord14 = FALSE;
			_midiType14 = 0;
			_midiMessage14 = 15;
			_midiCommand14 = 15;
			_midiFrom14 = 0;
			_midiTo14 = 0xff;

			_midiRecord15 = FALSE;
			_midiType15 = 0;
			_midiMessage15 = 16;
			_midiCommand15 = 16;
			_midiFrom15 = 0;
			_midiTo15 = 0xff;
		};

	private:
		bool _midiRecordVel;
		int _midiTypeVel;
		int _midiCommandVel;
		int _midiFromVel;
		int _midiToVel;

		bool _midiRecordPit;
		int _midiTypePit;
		int _midiCommandPit;
		int _midiFromPit;
		int _midiToPit;

		bool _midiRecord0;
		int _midiType0;
		int _midiMessage0;
		int _midiCommand0;
		int _midiFrom0;
		int _midiTo0;

		bool _midiRecord1;
		int _midiType1;
		int _midiMessage1;
		int _midiCommand1;
		int _midiFrom1;
		int _midiTo1;

		bool _midiRecord2;
		int _midiType2;
		int _midiMessage2;
		int _midiCommand2;
		int _midiFrom2;
		int _midiTo2;

		bool _midiRecord3;
		int _midiType3;
		int _midiMessage3;
		int _midiCommand3;
		int _midiFrom3;
		int _midiTo3;

		bool _midiRecord4;
		int _midiType4;
		int _midiMessage4;
		int _midiCommand4;
		int _midiFrom4;
		int _midiTo4;

		bool _midiRecord5;
		int _midiType5;
		int _midiMessage5;
		int _midiCommand5;
		int _midiFrom5;
		int _midiTo5;

		bool _midiRecord6;
		int _midiType6;
		int _midiMessage6;
		int _midiCommand6;
		int _midiFrom6;
		int _midiTo6;

		bool _midiRecord7;
		int _midiType7;
		int _midiMessage7;
		int _midiCommand7;
		int _midiFrom7;
		int _midiTo7;

		bool _midiRecord8;
		int _midiType8;
		int _midiMessage8;
		int _midiCommand8;
		int _midiFrom8;
		int _midiTo8;

		bool _midiRecord9;
		int _midiType9;
		int _midiMessage9;
		int _midiCommand9;
		int _midiFrom9;
		int _midiTo9;

		bool _midiRecord10;
		int _midiType10;
		int _midiMessage10;
		int _midiCommand10;
		int _midiFrom10;
		int _midiTo10;

		bool _midiRecord11;
		int _midiType11;
		int _midiMessage11;
		int _midiCommand11;
		int _midiFrom11;
		int _midiTo11;

		bool _midiRecord12;
		int _midiType12;
		int _midiMessage12;
		int _midiCommand12;
		int _midiFrom12;
		int _midiTo12;

		bool _midiRecord13;
		int _midiType13;
		int _midiMessage13;
		int _midiCommand13;
		int _midiFrom13;
		int _midiTo13;

		bool _midiRecord14;
		int _midiType14;
		int _midiMessage14;
		int _midiCommand14;
		int _midiFrom14;
		int _midiTo14;

		bool _midiRecord15;
		int _midiType15;
		int _midiMessage15;
		int _midiCommand15;
		int _midiFrom15;
		int _midiTo15;

		int _midiDriverIndex;		// MIDI IMPLEMENTATION
		int _midiHeadroom;
		int _syncDriverIndex;


	};
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// CMidiInput
//
// DESCRIPTION: MIDI Input handler class

class CMidiInput : public IMidiInput
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
	int FindDevByName( CString nameString );	// convert a name identifier into a index identifier (or -1 if fail)
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