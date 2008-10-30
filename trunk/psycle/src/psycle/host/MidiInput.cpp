///\file
///\brief implementation file for psycle::host::CMidiInput.
/// original code 21st April by Mark McCormack (mark_jj_mccormak@yahoo.co.uk) for Psycle - v2.2b -virtually complete-
#include <psycle/project.private.hpp>
#include "MidiInput.hpp"
#include "Psycle.hpp"
#include "Song.hpp"
#include "Player.hpp"
#include "Configuration.hpp"
#include "Plugin.hpp"
#include "VstHost24.hpp"
#include "ChildView.hpp"
#include "MainFrm.hpp"
#include "Helpers.hpp"
#include "InputHandler.hpp"
#include <cassert>
namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		CMidiInput * CMidiInput::s_Instance(0);	// the current instance

		CMidiInput::CMidiInput() : 
			m_midiInHandlesTried( 0 ),
			m_patIn( 0 ),
			m_patOut( 0 ),
			m_patCount( 0 ),
			m_timingCounter( 0 ),
			m_timingAccumulator( 0 ),
			m_baseStampTime( 0 ),
			m_reSync( false ),
			m_synced( false ),
			m_syncing( false )
		{
			assert(!Instance());

			// assign instance
			s_Instance = this;

			// clear down buffers
			std::memset( m_midiBuffer, 0, sizeof( MIDI_BUFFER ) * MIDI_BUFFER_SIZE );
			std::memset( m_channelSetting, -1, sizeof( int ) * MAX_MIDI_CHANNELS );
			std::memset( m_channelInstMap, 0, sizeof( std::uint32_t ) * MAX_MACHINES );
			std::memset( m_channelGeneratorMap, -1, sizeof( std::uint32_t ) * MAX_MIDI_CHANNELS );
			std::memset( m_channelNoteOff, 0, sizeof( bool ) * MAX_MIDI_CHANNELS );	
			std::memset( m_channelController, -1, sizeof( int ) * MAX_MIDI_CHANNELS * MAX_CONTROLLERS );	
			std::memset( m_devId, -1, sizeof( std::uint32_t ) * MAX_DRIVERS );
			std::memset( m_midiInHandle, 0, sizeof( HMIDIIN ) * MAX_DRIVERS );

			// setup config defaults (override some by registry settings)
			m_config.midiHeadroom = MIDI_PREDELAY_MS;
			std::strcpy( m_config.versionStr, VERSION_STRING );
			
			// clear down stats
			m_stats.flags = 0;
			m_stats.syncEventLatency = 0;
			m_stats.syncAdjuster = 0;
			m_stats.bufferCount = 0;
			m_stats.eventsLost = 0;
			m_stats.syncOffset = 0;
			m_stats.channelMapUpdate = true;
			m_stats.channelMap = 0;
			m_stats.bufferSize = MIDI_BUFFER_SIZE;

			// set initial mode
			m_midiMode = MODE_REALTIME;
		}

		CMidiInput::~CMidiInput()
		{
			// close midi now, if not already done explicitly
			if( m_midiInHandle[ DRIVER_MIDI ] || m_midiInHandle[ DRIVER_SYNC ] )
			{
				Close();
			}

			// clear instance
			s_Instance = NULL;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SetDeviceId
		//
		// DESCRIPTION	  : Set the MIDI device identifier without opening it
		// PARAMETERS     : int driver - driver identifier
		//                : std::uint32_t devId - device identifier
		// RETURNS		  : <void>

		void CMidiInput::SetDeviceId( unsigned int driver, int devId )
		{
			assert( driver < MAX_DRIVERS );
			m_devId[ driver ] = devId;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Open
		//
		// DESCRIPTION	  : Open the midi input devices
		// PARAMETERS     : <void>
		// RETURNS		  : bool - Worked true/Failed false

		bool CMidiInput::Open()
		{
			MMRESULT result;

			int problem = 0;
			int opened = 0;

			// open the main input driver
			if( !m_midiInHandle[ DRIVER_MIDI ] && m_devId[ DRIVER_MIDI ] != -1 )
			{

				result = midiInOpen( &m_midiInHandle[ DRIVER_MIDI ], m_devId[ DRIVER_MIDI ], (DWORD)fnMidiCallbackStatic, 0, CALLBACK_FUNCTION );
				if( result != MMSYSERR_NOERROR )
				{
					problem |= 0x01;
				}
				else
				{
					opened |= 0x01;
				}
			}

			// open the sync input driver (only if not the same as the main MIDI one)
			if( m_devId[ DRIVER_MIDI ] != m_devId[ DRIVER_SYNC ] &&
				!m_midiInHandle[ DRIVER_SYNC ] && m_devId[ DRIVER_SYNC ] != -1 )
			{
				// open
				result = midiInOpen( &m_midiInHandle[ DRIVER_SYNC ], m_devId[ DRIVER_SYNC ], (DWORD)fnMidiCallbackStatic, 0, CALLBACK_FUNCTION );
				if( result != MMSYSERR_NOERROR )
				{
					problem |= 0x02;
				}
				else
				{
					opened |= 0x01;
				}
			}

			// any problems?
			if( !m_midiInHandlesTried && problem )
			{
				CString messageText;
				int errorCount = 0;

				// create text error message
				messageText = "ERROR: Could not open the ";
				if( problem & 0x01 )
				{
					messageText += "MIDI input device";
					errorCount++;
				}
				if( problem & 0x02 )
				{
					if( errorCount == 0 )
					{
						messageText += "MIDI sync device";
					}
					else
					{
						messageText += "and MIDI sync device";
					}
				}
				messageText += "?";

				m_midiInHandlesTried = true;

				// messagebox to the user
				AfxMessageBox( messageText, MB_ICONEXCLAMATION+MB_OK );
			}

			// make sure we have good enough resolution from the system timer
			// (Win95/98 default to 1ms accuracy, but Win2000/NT can default to 5ms.  This makes sure we
			//  have an accurate enough timer!)
			timeBeginPeriod( 1 );

			// start midi input? (& zero timestamps)
			if( !opened )
				return problem != 0;
			else
				return Sync();

		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Sync
		//
		// DESCRIPTION	  : Starts the midi input devices (resets the timestamps)
		// PARAMETERS     : <void>
		// RETURNS		  : bool - Worked true/Failed false

		bool CMidiInput::Sync()
		{
			MMRESULT result;
			
			bool syncSet = false;
			int problem = 0;

			// main midi device open?
			if( m_midiInHandle[ DRIVER_MIDI ] )
			{
				result = midiInStart( m_midiInHandle[ DRIVER_MIDI ] );
				if (result != MMSYSERR_NOERROR)
				{
					problem |= 0x01;
				}


				// start (resets the dwParam2 timestamps)?
				if( m_devId[ DRIVER_MIDI ] == m_devId[ DRIVER_SYNC ] )
				{
					m_tickBase = timeGetTime();
					syncSet = true;
				}
			}

			// sync midi device open?
			if( !syncSet && m_midiInHandle[ DRIVER_SYNC ] )
			{
				result = midiInStart( m_midiInHandle[ DRIVER_SYNC ] );
				if (result != MMSYSERR_NOERROR)
				{
					problem |= 0x02;
				}
				
				// start (resets the dwParam2 timestamps)
				m_tickBase = timeGetTime();
			}

			// Bug: In an ideal world we would like our counter (timeGetTime) and the internal MIDI counter (from
			// the MIDI driver) to be exactly spot on and started at the same time.  However, sometimes these clocks
			// are not dead on an we this can give slight inaccuracy (up to 4ms) when running the clocks off against
			// each other to work out the latency of a MIDI message.

			m_synced = false;
			return problem != 0;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// ReSync
		//
		// DESCRIPTION	  : Call the resyncronise the MIDI with the audio engine
		//                  (for example after audio drop-outs/stalls)
		// PARAMETERS     : <void>
		// RETURNS		  : <void>

		void CMidiInput::ReSync()
		{
			m_reSync = true;

			// When using MIDI Sync codes, calling this function has the effect of forcing the MIDI input
			// module to wait until the next suitable 'timing clock' MIDI message (0xF8), and then perform
			// a MIDI->audio resync using this event as the base.  This function should be called after any
			// external code that causes the audio engine to stall/pause.

			// Bug: waveGetOutPos() is failing during some resyncs.  This is probably because the audio
			// engine has not been restarted by the time we are asking for the position.  We probably just
			// need to change the WaveOut.cpp so that it just returns zero to us and does not bring up the
			// error messagebox.
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Close
		//
		// DESCRIPTION	  : Close the midi input device
		// PARAMETERS     : <void>
		// RETURNS		  : bool - Worked true/Failed false

		bool CMidiInput::Close()
		{
			MMRESULT result;

			int problem = 0;

			// close drivers
			if( m_midiInHandle[ DRIVER_MIDI ] )
			{
				result = midiInClose( m_midiInHandle[ DRIVER_MIDI ] );
				if( result != MMSYSERR_NOERROR )
				{
					problem |= 0x01;
				}
				else
				{
					m_midiInHandle[ DRIVER_MIDI ] = NULL;
				}
			}

			if( m_midiInHandle[ DRIVER_SYNC ] )
			{
				result = midiInClose( m_midiInHandle[ DRIVER_SYNC ] );
				if( result != MMSYSERR_NOERROR )
				{
					problem |= 0x02;
				}
				else
				{
					m_midiInHandle[ DRIVER_SYNC ] = NULL;
				}
			}

			// allow messagebox errors again
			m_midiInHandlesTried = false;

			// we have finished with the resolution set in open
			timeEndPeriod( 1 );

			return problem != 0;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// FindDevByName
		//
		// DESCRIPTION	  : Convert a name identifier into a index identifier
		// PARAMETERS     : CString nameString - identifier by name
		// RETURNS		  : int - device identifier found (or -1 if fail)

		int CMidiInput::FindDevByName( CString nameString )
		{
			MIDIINCAPS mic;
			std::uint32_t numDevs;

			// get the number of MIDI input devices
			numDevs = midiInGetNumDevs();

			// scan for text-id match
			for( std::uint32_t idx = 0; idx < numDevs; idx++ )
			{
				// get info about the next device
				if( !midiInGetDevCaps( idx, &mic, sizeof(MIDIINCAPS) ) )
				{
					// compare
					if( nameString == mic.szPname )
					{
						return idx;
					}
				}
			}

			// not found
			return -1;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// GetNumDevices
		//
		// DESCRIPTION	  : Returns the number of midi input devices available
		// PARAMETERS     : <void?
		// RETURNS		  : int - amount of devices found

		int CMidiInput::GetNumDevices()
		{
			// get the number of MIDI input devices
			return midiInGetNumDevs();
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// PopulateListbox
		//
		// DESCRIPTION	  : Fill a listbox with a list of the available input devices
		// PARAMETERS     : CComboBox * listbox - pointer to the listbox to fill
		// RETURNS		  : std::uint32_t - amount of devices found

		std::uint32_t CMidiInput::PopulateListbox( CComboBox * listbox, bool issync )
		{
			MIDIINCAPS mic;
			std::uint32_t numDevs;

			// clear listbox
			listbox->ResetContent();

			// always add the null dev
			listbox->AddString( issync?"Same as MIDI Input Device":"None" );

			// get the number of MIDI input devices
			numDevs = midiInGetNumDevs();

			// add each to the listbox
			for( std::uint32_t idx = 0; idx < numDevs; idx++ )
			{
				midiInGetDevCaps( idx, &mic, sizeof( mic ) );
				listbox->AddString( mic.szPname );
			}

			return numDevs; 
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// GetNoteOffStatus
		//
		// DESCRIPTION	  : Returns the current note off status for a channel
		// PARAMETERS     : int channel - midi channel
		// RETURNS		  : bool - note off recognised?

		bool CMidiInput::GetNoteOffStatus( int channel )
		{
			assert( channel < MAX_MIDI_CHANNELS );
			return m_channelNoteOff[ channel ];
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SetInstMap
		// 
		// DESCRIPTION	  : Set an instrument map
		// PARAMETERS     : int channel - channel
		//                : int inst - instrument
		// RETURNS		  : <void>

		void CMidiInput::SetInstMap( int channel, int inst )
		{
			assert( channel < MAX_MIDI_CHANNELS );
			assert( inst < MAX_INSTRUMENTS );
			m_channelInstMap[ channel ] = inst;
			m_stats.channelMapUpdate = true;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// GetInstMap
		//
		// DESCRIPTION	  : Get the mapped instrument for the given machine
		// PARAMETERS     : int channel - channel
		// RETURNS		  : int - instrument

		int CMidiInput::GetInstMap( int channel )
		{
			assert( channel < MAX_MIDI_CHANNELS );
			return m_channelInstMap[ channel ];
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SetGenMap
		// 
		// DESCRIPTION	  : Set a generator map
		// PARAMETERS     : int channel - midi channel
		//                : int generator - generator
		// RETURNS		  : <void>

		void CMidiInput::SetGenMap( int channel, int generator )
		{
			assert( channel < MAX_MIDI_CHANNELS );
			assert( generator < MAX_MACHINES );
			m_channelGeneratorMap[ channel ] = generator;
			m_stats.channelMapUpdate = true;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// GetGenMap
		//
		// DESCRIPTION	  : Get the mapped generator for the given channel
		// PARAMETERS     : int channel - midi channel
		// RETURNS		  : int generator - generator

		int CMidiInput::GetGenMap( int channel )
		{
			assert( channel < MAX_MIDI_CHANNELS );
			return m_channelGeneratorMap[ channel ];
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// SetControllerMap
		// 
		// DESCRIPTION	  : Set a controller->parameter mapping (for channel)
		// PARAMETERS     : int channel - MIDI channel
		//                : int controller - MIDI controller
		//                : int parameter - generator parameter
		// RETURNS		  : <void>

		void CMidiInput::SetControllerMap( int channel, int controller, int parameter )
		{
			assert( channel < MAX_MIDI_CHANNELS );
			assert( controller < MAX_CONTROLLERS );
			assert( parameter < MAX_PARAMETERS );
			m_channelController[ channel ][ controller ] = parameter;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// GetControllerMap
		//
		// DESCRIPTION	  : Get a controller->parameter mapping (for channel)
		// PARAMETERS     : int channel - MIDI channel
		//                : int controller - MIDI controller
		// RETURNS        : int - generator parameter

		int CMidiInput::GetControllerMap( int channel, int controller )
		{
 			assert( channel < MAX_MIDI_CHANNELS );
			assert( controller < MAX_CONTROLLERS );
			return m_channelController[ channel ][ controller ];
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// fnMidiCallbackStatic <static>
		//
		// DESCRIPTION	  : The MIDI input callback function for our opened device <linker>
		// PARAMETERS     : HMIDIIN handle - midi input handle identifier
		//                : std::uint32_t uMsg - message identifier
		//                : DWORD dwInstance - user instance data (not used)
		//                : DWORD dwParam1 - various midi message info
		//                : DWORD dwParam2 - various midi message info
		// RETURNS		  : <void>

		void CALLBACK CMidiInput::fnMidiCallbackStatic( HMIDIIN handle, std::uint32_t uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
		{
			CMidiInput * pMidiInput = CMidiInput::Instance();

			// do nothing if there is no reference object or the object is not active
			if( !pMidiInput || (!pMidiInput->GetHandle( DRIVER_MIDI ) && !pMidiInput->GetHandle( DRIVER_SYNC )))
			{
				return;
			}

			// strobe the master input
			pMidiInput->GetStatsPtr()->flags |= FSTAT_MIDI_INPUT;

			// link to reference object
			switch( pMidiInput->m_midiMode )
			{
			case MODE_REALTIME:
				pMidiInput->fnMidiCallback_Inject( handle, uMsg, dwInstance, dwParam1, dwParam2 );
				break;

			case MODE_STEP:
				pMidiInput->fnMidiCallback_Step( handle, uMsg, dwInstance, dwParam1, dwParam2 );
				break;

			default:
				// no mode, you will loose the data
				break;
			};
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// fnMidiCallback_Inject
		//
		// DESCRIPTION	  : The MIDI input callback function for our opened device
		// PARAMETERS     : HMIDIIN handle - midi input handle identifier
		//                : std::uint32_t uMsg - message identifier
		//                : DWORD dwInstance - user instance data (not used)
		//                : DWORD dwParam1 - various midi message info
		//                : DWORD dwParam2 - various midi message info
		// RETURNS		  : <void>

		void CALLBACK CMidiInput::fnMidiCallback_Inject( HMIDIIN handle, std::uint32_t uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
		{
			// branch on type of midi message
			switch( uMsg )
			{
				// normal data message
				case MIM_DATA:
				{
					// split the first parameter DWORD into bytes
					int p1HiWordHB = (dwParam1 & 0xFF000000) >>24; p1HiWordHB; // not used
					int p1HiWordLB = (dwParam1 & 0xFF0000) >>16;
					int p1LowWordHB = (dwParam1 & 0xFF00) >>8;
					int p1LowWordLB = (dwParam1 & 0xFF);

					// assign uses
					int status = p1LowWordLB;
					int note = p1LowWordHB;
					int program = p1LowWordHB;
					int noteOn = p1HiWordLB;
					int data1 = p1LowWordHB;
					int data2 = p1HiWordLB;
					int cmd = 0;
					int parameter = 0;

					// and a bit more...
					int statusHN = (status & 0xF0) >> 4;
					int statusLN = (status & 0x0F);
					int channel = statusLN;

					// map channel -> generator
					int busMachine = GetGenMap( channel );

					// branch on status code
					switch( statusHN )
					{
						// note on/off
						case 0x09:
						{
							// note on?
							if( noteOn )
							{
								// limit to playable range (above this is special codes)
								if( note > 119 ) note = 119;
							}
							else
							{
								// note off
								if( m_channelNoteOff[ channel ] )
								{
									note = 120;
								}
								else
								{
									return;
								}
							}
						}	
						break;

						// (also) note off
						case 0x08:
						{
							// note off
							if( m_channelNoteOff[ channel ] )
							{
								note = 120;
							}
							else
							{
								return;
							}
						}
						break;

						// program change
						case 0x0C:
						{
							// program change -> map generator/effect to channel

							// machine active?
							if( program < MAX_MACHINES )
							{  
								if (Global::_pSong->_pMachine[ program ] )
								{
									// ok, map
									SetGenMap( channel, program );
								}
								else
								{
									// machine not active, can't map!
									SetGenMap( channel, -1 );
								}
							}
							else
							{
								// machine not active, can't map!
								SetGenMap( channel, -1 );
							}
							return;
						}
						break;

						/*
						case 11:
							// mods
							// data 2 contains the info
							break;

						case 14:
							// pitch wheel
							// data 2 contains the info
							break;
							*/

						// extended codes
						case 0x0F:
						{
							switch( statusLN )
							{
								// MIDI SYNC: Start
								case 0x0A:
								{
									m_stats.flags |= FSTAT_FASTART;

									// force sync
									InternalReSync( dwParam2 );
									return;
								}
								break;

								// MIDI SYNC: Timing Clock
								case 0x08:
								{
									m_stats.flags |= FSTAT_F8CLOCK;

									// resync?
									if( m_reSync )
									{
										m_stats.flags |= FSTAT_RESYNC;
									
										m_reSync = false;
										
										// force sync
										InternalReSync( dwParam2 );
									}
									else
									{
										// use clocks to keep us in sync as best as we can
										InternalClock( dwParam2 );
									}
									return;
								}
								break;

								// MIDI SYNC: Stop
								case 0x0C:
								{
									m_stats.flags |= FSTAT_FCSTOP;

									// stop the song play (in effect, stops all sound)
									Global::pPlayer->Stop();
									return;
								}
								break;

								// do nothing (apart from exit) if not recognised
								default:
								{
									return;
								}
							}
						}

						// controller
						case 0x0B:
						{
							// switch on controller ID
							switch( data1 )
							{
								// BANK SELECT (controller 0)
								case 0:
								{
									// banks select -> map instrument to channel
									SetInstMap( channel, data2 );
									return;
								}
								break;

								// SIMULATED MIDI Sync: Start (controller 121)
								case 0x79:
								{
									// simulated sync start
									m_stats.flags |= FSTAT_EMULATED_FASTART;

									// force sync
									InternalReSync( dwParam2 );
									return;
								}
								break;
					
								// SIMULATED MIDI Sync: Clock (controller 122)
								case 0x7A:
								{
									// simulated sync clock
									m_stats.flags |= FSTAT_EMULATED_F8CLOCK;

									// resync?
									if( m_reSync )
									{
										m_stats.flags |= FSTAT_RESYNC;
									
										m_reSync = false;
										
										// force sync
										InternalReSync( dwParam2 );
									}
									else
									{
										// use clocks to keep us in sync as best as we can
										InternalClock( dwParam2 );
									}
									return;
								}
								break;

								// TICK SYNC (controller 123 - data not zero)
								case 0x7B:
								{
									// sync?
									if( data2 )
									{
										note = 254;
									}
									else
									{
										// zero data means do a note off
										note = 120;
									}
								}
								break;

								// SIMULATED MIDI Sync: Stop (controller 124)
								case 0x7C:
								{
									// simulated sync stop
									m_stats.flags |= FSTAT_EMULATED_FCSTOP;

									// stop the song play (in effect, stops all sound)
									Global::pPlayer->Stop();
									return;
								}
								break;

								// NOTE OFF ENABLE (controller 125)
								case 0x7D:
									{
										// enable/disable
										if( data2 )
										{
											m_channelNoteOff[ channel ] = true;
										}
										else
										{
											m_channelNoteOff[ channel ] = false;
										}
										m_stats.channelMapUpdate = true;
										return;
									}
									break;
									
								// SET CONTROLLER (stage 1 - controller 126)
								case 0x7E:
									{
										m_channelSetting[ channel ] = data2;
										return;
									}
									break;

								// SET CONTROLLER (stage 2 - controller 127)
								case 0x7F:
									{
										// controller number set? (stage1)
										if( m_channelSetting[ channel ] >= 0 )
										{
											// we can set map
											SetControllerMap( channel, m_channelSetting[ channel ], data2 );

											// clear down
											m_channelSetting[ channel ] = -1;
										}
										return;
									}
									break;

								// * ANY OTHER CONTROLLER COMES HERE *
								default:
								{
									// generic controller -> tweak
									int gParameter = GetControllerMap( channel, data1 );
									
									// set?
									if( gParameter >= 0 )
									{
										note = notecommands::tweak;
										cmd = gParameter;
										parameter = data2;
									}
									else
									{
										// controller not setup, we can't do anything
										return;
									}
								}
								break;
							}
						}
						break;

						// unhandled
						default:
						{
							return;
						}
						break;
					}

					// buffer overflow?
					if( m_patCount >= MIDI_BUFFER_SIZE )
					{
						m_stats.flags |= FSTAT_BUFFER_OVERFLOW;
						return;
					}

					// invalid machine/channel?
					if( !Global::_pSong->_pMachine[ busMachine ] && note != 254 )
					{
						return;
					}

					// create a patten entry struct in the midi buffer
					int patIn = m_patIn;
					PatternEntry * pEntry = &m_midiBuffer[ patIn ].entry;

					pEntry->_note = note;
					pEntry->_mach = busMachine;
					pEntry->_inst = GetInstMap( channel );
					pEntry->_cmd = cmd;
					pEntry->_parameter = parameter;

					// add the other necessary info
					m_midiBuffer[ patIn ].timeStamp = dwParam2;
					m_midiBuffer[ patIn ].channel = channel;
					m_stats.channelMap |= (1 << channel);

					// advance IN pointer
					m_patCount++;
					m_patIn++;
					if( m_patIn >= MIDI_BUFFER_SIZE ) m_patIn = 0;
					m_stats.flags |= FSTAT_IN_BUFFER;

					// this the 1st message, we are NOT synced already, sync to audio engine now!
					if( !m_synced )
					{
						// force sync
						InternalReSync( dwParam2 );
					}

				}
			}	// end..of midi message branch
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// fnMidiCallback_Step
		//
		// DESCRIPTION	  : The MIDI input callback function for our opened device
		// PARAMETERS     : HMIDIIN handle - midi input handle identifier
		//                : std::uint32_t uMsg - message identifier
		//                : DWORD dwInstance - user instance data (not used)
		//                : DWORD dwParam1 - various midi message info
		//                : DWORD dwParam2 - various midi message info
		// RETURNS		  : <void>

		void CALLBACK CMidiInput::fnMidiCallback_Step( HMIDIIN handle, std::uint32_t uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
		{
			// pipe MIDI note on messages into the pattern entry window

//			bool noteOff = false; noteOff; // not used

			// branch on type of midi message
			switch( uMsg )
			{
				// normal data message
				case MIM_DATA:
				{
					// split the first parameter DWORD into bytes
					int p1HiWordHB = (dwParam1 & 0xFF000000) >>24; p1HiWordHB; // not used
					int p1HiWordLB = (dwParam1 & 0xFF0000) >>16;
					int p1LowWordHB = (dwParam1 & 0xFF00) >>8;
					int p1LowWordLB = (dwParam1 & 0xFF);

					// assign uses
					int status = p1LowWordLB;
					int note = p1LowWordHB;
					int velocity = p1HiWordLB;
					int data1 = p1LowWordHB;
					int data2 = p1HiWordLB;
					int data = ((data2&0x7f)<<7)|(data1&0x7f);

//					int cmd = 0; cmd; // not used
//					int parameter = 0; parameter; // not used
					
					// and a bit more...
					int statusHN = (status & 0xF0) >> 4;
					int statusLN = (status & 0x0F);
					int channel = statusLN;

					CMainFrame & frame(*static_cast<CMainFrame*>(theApp.m_pMainWnd));
					///\todo: Disabled because it makes the aux column be setup without user control. Needs improvement.
					//frame.ChangeIns(channel);

					// branch on status code
					switch( statusHN )
					{
						// (also) note off
						case 0x08:		
							velocity=0;
						// note on/off
						case 0x09:
    						// limit to playable range (above this is special codes)
    						if(note>119) 
    							note=119;
							// TODO: watch this, it should be OK as long as we don't change things too much
							frame.m_wndView.MidiPatternNote(note,velocity);
							return;
						default:break;
					}

					if (Global::pConfig->_RecordTweaks)
					{
						if (Global::pConfig->midi().raw() && status != 0xFE )
						{
							frame.m_wndView.MidiPatternMidiCommand(status,(data1 << 8) | data2);
							return;
						}
						// branch on status code
						else switch(statusHN)
						{
							case 0x0B:
								// mods
								// data 2 contains the info
								for(int i(0) ; i < 16 ; ++i)
								{
									if(Global::pConfig->midi().group(i).record() && (Global::pConfig->midi().group(i).message() == data1 ))
									{
										int const value(Global::pConfig->midi().group(i).from() + (Global::pConfig->midi().group(i).to() - Global::pConfig->midi().group(i).from()) * data2 / 127);
										switch(Global::pConfig->midi().group(i).type())
										{
											case 0:
												frame.m_wndView.MidiPatternCommand(Global::pConfig->midi().group(i).command(), value);
												break;
											case 1:
												frame.m_wndView.MidiPatternTweak(Global::pConfig->midi().group(i).command(), value);
												break;
											case 2:
												frame.m_wndView.MidiPatternTweakSlide(Global::pConfig->midi().group(i).command(), value);
												break;
											case 3:
												frame.m_wndView.MidiPatternMidiCommand(status, (data1 << 8) | data2);
												break;
											case 4:
												frame.m_wndView.MidiPatternInstrument(value);
										}
									}
								}
								break;
							case 0x0E:
								// pitch wheel
								// data 2 contains the info
								if (Global::pConfig->_RecordTweaks)
								{
									if (Global::pConfig->midi().pitch().record())
									{
										int const value(Global::pConfig->midi().pitch().from() + (Global::pConfig->midi().pitch().to() - Global::pConfig->midi().pitch().from()) * data / 0x3fff);
										switch (Global::pConfig->midi().pitch().type())
										{
										case 0:
											frame.m_wndView.MidiPatternCommand(Global::pConfig->midi().pitch().command(), value);
											break;
										case 1:
											frame.m_wndView.MidiPatternTweak(Global::pConfig->midi().pitch().command(), value);
											break;
										case 2:
											frame.m_wndView.MidiPatternTweakSlide(Global::pConfig->midi().pitch().command(), value);
											break;
										case 3:
											frame.m_wndView.MidiPatternMidiCommand(status, (data1 << 8) | data2);
											break;
										case 4:
											frame.m_wndView.MidiPatternInstrument(value);
											break;
										}
									}
								}
						}
					}
					else
					{
						// midi controllers pass-through
						int mgn = Global::_pSong->seqBus;

						if (mgn < MAX_MACHINES)
						{
							Machine* pMachine = Global::_pSong->_pMachine[mgn];
							if (pMachine)
							{
								if (pMachine->_type == MACH_VST || pMachine->_type == MACH_VSTFX )
								{
									((vst::plugin*)pMachine)->AddMIDI(status,data1,data2);
									return;
								}
								else
								{
#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
#else
#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
									PatternEntry pentry(notecommands::midicc,status,255,data1,data2,pMachine->_macIndex);
#else
									PatternEntry pentry(notecommands::midicc,status,pMachine->_macIndex,data1,data2);
#endif
#endif
									pMachine->Tick(0,&pentry);
								}
							}
						}
					}

				}	// end of.. case NIM_DATA

			}	// end of.. uMsg switch


		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// InternalClock
		//
		// DESCRIPTION	  : Called on a MIDI Clock event to keep Psycle in sync
		// PARAMETERS     : DWORD dwParam2 - timing stamp
		// RETURNS		  : <void>

		void CMidiInput::InternalClock( DWORD dwParam2 )
		{
			int samplesPerSecond = Global::pConfig->_pOutputDriver->_samplesPerSec;

			// WARNING! GetPlayPos() has max of 0x7FFFFF

			// get the current play sample position
 			int playPos = Global::pConfig->_pOutputDriver->GetPlayPos();

			// calc the latency of the clock midi message
			int midiLatencyMs = ( timeGetTime() - m_tickBase ) - dwParam2;

			// adjust the sample position (fix latency)
			int adjPlayPos = playPos - helpers::math::rounded((midiLatencyMs/1000.f) * samplesPerSecond );
			
			// never let the adjusted play pos become negative (this really breaks things - usually
			// when trying to resync just after the audio engine has restared)
			if( adjPlayPos < 0 )
			{
				adjPlayPos = 0;
			}

			// sample counter wrap around?
			// (detects a wrap around using the transistion from the MSB of the 23 bit sample counter
			//  from a 1 to a 0.  could possible get confused if we are adjusting the play position
			//  "backwards" in the sample counter (to fix lag), but I have not seen this)
			if( (m_lastPlayPos & 0x400000) && !(adjPlayPos & 0x400000) )
			{
				m_wraps++;
			}

			// create real big sample counter
			// (if anyone is bothered to keep one song going long enough for this counter to wrap-around, then
			//  good luck to them!  They must be mad...)
			LARGE_INTEGER bigSampleCounter;
			bigSampleCounter.QuadPart = (m_wraps * 0x800000) + adjPlayPos;

			// convert this back to a ms counter
			DWORD playMs = (DWORD)(((bigSampleCounter.QuadPart - m_adjustedPlayPos)/(float)samplesPerSecond) * 1000.0f);

			// how many milliseconds should we have taken
			DWORD shouldTakenMs = dwParam2 - m_baseStampTimeFix;

			// create offset (ms)
			m_fixOffset = playMs - shouldTakenMs;
			m_stats.syncOffset = m_fixOffset;

			// move on
			m_lastPlayPos = adjPlayPos;

		#ifdef _DEBUGGING
		// (DEBUGGING)
		int static tstrobe = 0;
		if( tstrobe == 0 )
		{
			char tmp[ 128 ];
			sprintf( tmp, "(0x%x) ms difference = %d playPos = %u (samples)\n\0", dwParam2, m_fixOffset, playPos );
			TRACE( tmp );
		}
		tstrobe++;
		if( tstrobe > 40 ) tstrobe = 0;
		#endif
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// InternalReSync
		//
		// DESCRIPTION	  : Instigates an MIDI->audio resync
		// PARAMETERS     : DWORD dwParam2 - timing stamp
		// RETURNS		  : <void>

		void CMidiInput::InternalReSync( DWORD dwParam2 )
		{
			// get the current play sample position
			int playPos = Global::pConfig->_pOutputDriver->GetPlayPos();
			
			int samplesPerSecond = Global::pConfig->_pOutputDriver->_samplesPerSec;

			// calculate the latency of the MIDI message in samples (delay in getting to us)
			// using our own timer, started at the same time (hopefully!) as the MIDI
			// input timer.
 			int midiLatency = ( timeGetTime() - m_tickBase ) - dwParam2;
			int midiLatencySamples = helpers::math::rounded( (midiLatency/1000.f) * samplesPerSecond );
			m_stats.syncEventLatency = midiLatencySamples;

			// work out the real play position
			m_adjustedPlayPos = playPos - midiLatencySamples;

			// save vars
			m_baseStampTime = dwParam2;
			m_baseStampTimeFix = dwParam2;

			// set vars
			m_synced = true;
			m_syncing = true;
			m_wraps = 0;
			m_fixOffset = 0;

		#ifdef _DEBUGGING
		// (DEBUGGING)
		char tmp[128];
		sprintf( tmp, "(0x%x) MIDI latency error %d (samples) samplebase = %u\n\0", dwParam2, midiLatencySamples, m_adjustedPlayPos );
		TRACE( tmp );
		#endif
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// InjectMIDI
		//
		// DESCRIPTION	  : Called from the player to perform the custom midi injection
		// PARAMETERS     : int amount - amount of block samples
		// RETURNS		  : bool - true if we did some processing, false if nothing was done

		bool CMidiInput::InjectMIDI( int amount )
		{
			// NOTE: The DirectSound driver is currently in no fit state to be used with
			// the midi input.  The get write/play functions are not compatible with our
			// method here in that they differ in the way the MME functions have been
			// implemented!?  Oh, and the DSound functions are also not working correctly!

			// midi injection NOT enabled?
			if( !m_midiInHandle[ DRIVER_MIDI ] || 
				strcmp( Global::pConfig->_pOutputDriver->GetInfo()->_psName, "Windows WaveOut MME" ) != 0 )	// TODO: need to remove this string compare? (speed)
			{
				m_stats.flags &= ~FSTAT_ACTIVE;
				return false;
			}

			m_stats.flags |= FSTAT_ACTIVE;

			int samplesPerSecond = Global::pConfig->_pOutputDriver->_samplesPerSec;
			m_stats.bufferCount = m_patCount;

			// (waiting until we are sure we will have enough midi data in the buffer
			//  to correctly inject the data when we are next called to write samples)
			if( m_syncing )
			{
				m_stats.flags |= FSTAT_SYNC;

				// get the write position now and adjust for samples done since the
				// resync interrupt
				int writePos = Global::pConfig->_pOutputDriver->GetWritePos();
				
				int blockSamples = Global::pConfig->_pOutputDriver->GetBufferSize() / Global::pConfig->_pOutputDriver->GetSampleSize();
				int blocks = Global::pConfig->_pOutputDriver->GetNumBuffers();

				// calculate our final adjuster
				int syncAdjuster = m_adjustedPlayPos - ( writePos - (blockSamples*blocks) );
				m_stats.syncAdjuster = syncAdjuster;

		#ifdef _DEBUGGING
		// (DEBUGGING)
		char tmp[128];
		sprintf( tmp, "writePos %d (samples) syncAdjuster %d (samples) buffer %d (events)\n\0", writePos, syncAdjuster, m_stats.bufferCount );
		TRACE( tmp );
		#endif

				// adjust the time base
				// (if m_syncLimit < 0 we will probably loose a couple
				// of the initial MIDI events, BUT we should keep the sync!)
				m_baseStampTime -= (DWORD)(( syncAdjuster /(float)samplesPerSecond) * 1000.0f);

				// initial accumulator val
				m_timingAccumulator = 0;
				m_syncing = false;
			}

			// create our working stamp (account for timing fix and MIDI buffer headroom)
			DWORD tbaseStampTime = m_baseStampTime - m_fixOffset - m_config.midiHeadroom;

			// accumulate sample count
			m_timingAccumulator += amount;

			// convert accumulator to milliseconds
			m_timingCounter = (DWORD)(((float)(m_timingAccumulator)/samplesPerSecond) * 1000.0f);

			// remove any midi data that happend over before the base stamp
			while( m_patCount && m_midiBuffer[ m_patOut ].timeStamp < tbaseStampTime )
			{
				// advance OUT pointer
				m_patCount--;
				m_patOut++;
				if( m_patOut >= MIDI_BUFFER_SIZE ) m_patOut = 0;

		#ifdef _DEBUGGING
		// (DEBUGGING)
		TRACE( "*Lost Event*\n" );
		#endif
				m_stats.eventsLost++;
				m_stats.flags |= FSTAT_CLEAR_BUFFER;
			}

			// NO midi data that need to be injected during this amount block?
			if( !m_patCount || m_timingCounter < (m_midiBuffer[ m_patOut ].timeStamp - tbaseStampTime ) )
			{
				return false;
			}

			// NOTE: because we are inserting all the machine ticks before working the master
			// we will always have the notes being slightly advanced of the song.  How much will
			// be a factor of the current sample-rate and amount of samples.  For example, at
			// 44100hz with a 256 sample block the vary will be up to 5.8ms, at a lower sample
			// rate, say 22050hz, but sample block size it increases up to 11.6ms.

			// I have tried to combat this by interleaving the ->Tick(..) functions with several
			// ->Work(..) functions, but this has always produced audio pops & clicks, so I have
			// left it with the very minimal (and I don't think noticable) 5.8ms vary at 44100hz
			// (which is the sample rate most people use anyway).

			// if you can fix this, or implement better, then please do! [MJJM]

			m_stats.flags |= FSTAT_OUT_BUFFER;

			// OK, if we get here then there is at least one MIDI message that needs injecting
			do
			{
				int note = m_midiBuffer[ m_patOut ].entry._note;
				int machine = m_midiBuffer[ m_patOut ].entry._mach;
				int data1 = m_midiBuffer[ m_patOut ].entry._cmd;
				int data2 = m_midiBuffer[ m_patOut ].entry._parameter;

				// get the machine pointer
				Plugin * pMachine = (Plugin*) Global::_pSong->_pMachine[ machine ];
				// make sure machine is still valid
				if( pMachine || note == 254 )
				{
					// switch on note code
					switch( note )
					{
						// TWEAK
						case notecommands::tweakslide:
							// *********
							// midi doesn't get a tweak slide yet
						case notecommands::tweak:
						{
							int min, max;

							// any info
							if( pMachine->_type == MACH_PLUGIN )
							{
								// make sure parameter in range of machine
								if( data1 > (pMachine->GetInfo()->numParameters-1) )
								{
									break;
								}

								// get range
								min = pMachine->GetInfo()->Parameters[ data1 ]->MinValue;
								max = pMachine->GetInfo()->Parameters[ data1 ]->MaxValue;
							}
							else
							{
								// assume 0000..FFFF is the range (VST)
								min = 0;
								max = 0xFFFF;
							}

							// create actual value
							int value = min + helpers::math::rounded( (max-min) * (data2/127.f) );

							// assign
							m_midiBuffer[ m_patOut ].entry._inst = data1;
							m_midiBuffer[ m_patOut ].entry._cmd = value / 256;
							m_midiBuffer[ m_patOut ].entry._parameter = value % 256;

							// and tweak!
							pMachine->Tick( m_midiBuffer[ m_patOut ].channel, &m_midiBuffer[ m_patOut ].entry );
						}
						break;


						// SYNC TICK
						case 254:
						{
							// simulate a tracker 'tick' (i.e. a line change for all machines)
							for (int tc=0; tc<MAX_MACHINES; tc++)
							{
								if( Global::_pSong->_pMachine[tc])
								{
									Global::_pSong->_pMachine[tc]->Tick();
								}
							}

							m_stats.flags |= FSTAT_SYNC_TICK;
						}
						break;
						
						// NORMAL NOTE
						default:
						{
							// normal note tick
							pMachine->Tick( m_midiBuffer[ m_patOut ].channel, &m_midiBuffer[ m_patOut ].entry );
						}
						break;

					}	// end of note switch
				}	// end of if 'is machine still active?'

				// advance OUT pointer
				m_patCount--;
				m_patOut++;
				if( m_patOut >= MIDI_BUFFER_SIZE ) m_patOut = 0;

			} while( m_patCount && m_timingCounter >= (m_midiBuffer[ m_patOut ].timeStamp - tbaseStampTime) );

			// Master machine initiates work
			//
			Global::_pSong->_pMachine[MASTER_INDEX]->Work( amount );
			return true;
		}
	}
}
