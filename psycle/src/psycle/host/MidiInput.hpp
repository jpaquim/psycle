// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief implementation file for psycle::host::CMidiInput.
/// original code 21st April by Mark McCormack (mark_jj_mccormak@yahoo.co.uk) for Psycle - v2.2b -virtually complete-
#pragma once
#include "constants.hpp"
#include "songstructs.hpp"
#include <diversalis/compiler.hpp>

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

	#include <mmsystem.h>
	#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
		#pragma comment(lib, "winmm")
	#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

#include <cassert>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		#define MAX_MIDI_CHANNELS 16
		#define MAX_CONTROLLERS   127
		#define MAX_PARAMETERS    127

		///\todo might want to make these two user-configurable rather than hard-coded values.
		#define MIDI_BUFFER_SIZE 1024
		#define MIDI_PREDELAY_MS 200

		#define VERSION_STRING "v2.2b"

		///\name FLAGS
		///\{
			/// are we waiting to inject MIDI?
			#define FSTAT_ACTIVE                0x0001
			/// the three MIDI sync codes
			#define FSTAT_FASTART               0x0002
			/// the three MIDI sync codes
			#define FSTAT_F8CLOCK               0x0004
			/// the three MIDI sync codes
			#define FSTAT_FCSTOP                0x0008

			/// Controller 121 - emulated FASTART
			#define FSTAT_EMULATED_FASTART      0x0010
			/// Controller 122 - emulated F8CLOCK
			#define FSTAT_EMULATED_F8CLOCK      0x0020
			/// Controller 124 - emulated FCSTOP
			#define FSTAT_EMULATED_FCSTOP       0x0040
			/// midi buffer overflow?
			#define FSTAT_BUFFER_OVERFLOW       0x0080

			/// midi data going into the buffer?
			#define FSTAT_IN_BUFFER             0x0100
			/// midi data going out of the buffer?
			#define FSTAT_OUT_BUFFER            0x0200
			/// midi data being cleared from the buffer?
			#define FSTAT_CLEAR_BUFFER          0x0400
			/// simulated tracker tick?
			#define FSTAT_SYNC_TICK             0x0800

			/// syncing with the audio?
			#define FSTAT_SYNC                  0x1000
			/// just resynced with the audio?
			#define FSTAT_RESYNC                0x2000
			/// we are receiving MIDI input data?
			#define FSTAT_MIDI_INPUT            0x4000

			/// to clear correct flags
			#define FSTAT_CLEAR_WHEN_READ       0x0001
		///\}

		class MIDI_BUFFER
		{
		public:
			/// tracker pattern info struct
			PatternEntry entry;
			/// MIDI input device's timestamp
			std::uint32_t timeStamp;
			/// MIDI channel
			int channel;
		};

		class MIDI_CONFIG
		{
		public:
			/// version string (e.g. v2.0b)
			char versionStr[ 16 ];
			/// milliseconds allowed for MIDI buffer fill
			int midiHeadroom;
		};

		class MIDI_STATS
		{
		public:
			/// error (latency) of the last sync event (samples)
			int syncEventLatency;	
			/// last sync adjustment value (samples)
			int syncAdjuster;		
			/// amount of MIDI messages currently in the buffer (events)
			int bufferCount;		
			/// capacity of the MIDI message buffer (events)
			int bufferSize;			
			/// how many events have been lost and not played? (events)
			int eventsLost;			
			/// how far off the audio engine is from the MIDI (ms)
			int syncOffset;			
			/// bitmapped channel active map	(CLEAR AFTER READ)
			unsigned int channelMap;
			/// strobe for the channel map list
			bool channelMapUpdate;
			/// 32 bits of boolean info (see FLAGS, CLEAR AFTER READ)
			std::uint32_t flags;			
		};

		enum MODES
		{
			MODE_REALTIME, ///< use Psycle as a cool softsynth via MIDI
			MODE_STEP,     ///< enter pattern notes using MIDI
			MAX_MODES
		};

		enum DRIVERS
		{
			DRIVER_MIDI, ///< the main MIDI input driver
			DRIVER_SYNC, ///< the driver for MIDI Sync
			MAX_DRIVERS
		};

		/// midi input handler.
		class CMidiInput
		{
		public:
			CMidiInput();
			virtual ~CMidiInput();

			/// returns the current instance
			static CMidiInput * Instance() { return s_Instance; }

			/// set MIDI input device identifier
			void SetDeviceId(unsigned int driver, int devId);	
			/// open the midi input devices
			bool Open();				
			/// start input (reset timestamps)
			bool Sync();			
			/// resync the MIDI with the audio engine
			void ReSync();			
			/// close the midi input device
			bool Close( );				

			/// find out if we are open
			bool Active() { return m_midiInHandle!=NULL; }

			/// for external access
			MIDI_STATS * GetStatsPtr() { return &m_stats; }		
			/// for external access
			MIDI_CONFIG * GetConfigPtr() { return &m_config; }

			/// returns the number of midi devices on the system
			int GetNumDevices( void );	
			/// convert a name identifier into a index identifier (or -1 if fail)
			int FindDevByName( CString nameString );	
			/// fill a listbox with a list of the available input devices
			std::uint32_t PopulateListbox( CComboBox * listbox , bool issync );	

			/// return the current device handle
			HMIDIIN GetHandle(unsigned int driver) { assert(driver < MAX_DRIVERS); return m_midiInHandle[driver]; }

			/// set a instrument map
			void SetInstMap( int machine, int inst );	
			/// get the mapped instrument for the given machine
			int GetInstMap( int machine );	

			/// set a instrument map
			void SetGenMap( int channel, int generator );	
			/// get the mapped instrument for the given machine
			int GetGenMap( int channel );	

			/// set a controller map
			void SetControllerMap( int channel, int controller, int parameter );	
			/// get a controller map
			int GetControllerMap( int channel, int controller );	

			/// get the channel's note off status
			bool GetNoteOffStatus( int channel );	

			/// called to inject MIDI data
			bool InjectMIDI( int amount );	

			/// the master MIDI handler mode (see enum MODES), external objects can change this at will
			int m_midiMode;	

		private:
			/// the midi callback functions (just a static linker to the instance one)
			static void CALLBACK fnMidiCallbackStatic( HMIDIIN handle, std::uint32_t uMsg, std::uint32_t dwInstance, std::uint32_t dwParam1, std::uint32_t dwParam2 );

			/// the real callbacks
			void CALLBACK fnMidiCallback_Inject( HMIDIIN handle, std::uint32_t uMsg, std::uint32_t dwInstance, std::uint32_t dwParam1, std::uint32_t dwParam2 );
			/// the real callbacks
			void CALLBACK fnMidiCallback_Step( HMIDIIN handle, std::uint32_t uMsg, std::uint32_t dwInstance, std::uint32_t dwParam1, std::uint32_t dwParam2 );

			/// interal engine resync process
			void InternalReSync( std::uint32_t dwParam2 );
			/// ???
			void InternalClock( std::uint32_t dwParam2 );

			/// the current instance pointer
			static CMidiInput * s_Instance;	

			/// current input device handles
			HMIDIIN m_midiInHandle[ MAX_DRIVERS ];	
			/// for once-only tell of problems
			bool m_midiInHandlesTried;	

			/// channel->instrument map
			std::uint32_t m_channelInstMap[ MAX_MACHINES ];				
			/// channel->generator map
			std::uint32_t m_channelGeneratorMap[ MAX_MIDI_CHANNELS ];	
			/// channel, note off setting
			bool m_channelNoteOff[ MAX_MIDI_CHANNELS ];			
			/// channel->controller->parameter map
			int m_channelController[ MAX_MIDI_CHANNELS ][ MAX_CONTROLLERS ];		

			///\name audio-engine timing vars
			///\{
				/// .
				std::uint32_t m_timingCounter;
				/// .
				std::uint32_t m_timingAccumulator;
				/// .
				std::uint32_t m_prevTimingCounter;
				/// the base sync stamp
				std::uint32_t m_baseStampTime;	
				/// .
				std::uint32_t m_tickBase;
			///\}

			/// midi device identifiers
			int m_devId[ MAX_DRIVERS ];		

			/// configuration information
			MIDI_CONFIG	m_config;	
			/// statistics information
			MIDI_STATS	m_stats;	
			
			/// midi buffer
			MIDI_BUFFER m_midiBuffer[ MIDI_BUFFER_SIZE ];	

			/// used in setting a controller map
			int m_channelSetting[ MAX_MIDI_CHANNELS ];	

			///\name buffer indexes
			///\{
				/// .
				int m_patIn;
				/// .
				int m_patOut;
				/// .
				int m_patCount;
			///\}

			/// external telling us we need a resync
			bool m_reSync;			
			/// have we been synced with the audio engine yet?
			bool m_synced;			
			/// are we in the process of syncing?
			bool m_syncing;			

			///\name syncronization variables
			///\{
				/// .
				std::uint32_t m_baseStampTimeFix;
				/// .
				int m_fixOffset;
				/// .
				int m_lastPlayPos;
				/// .
				int m_wraps;
				/// adjusted play position
				int m_adjustedPlayPos;	
			///\}
		};
	}
}
