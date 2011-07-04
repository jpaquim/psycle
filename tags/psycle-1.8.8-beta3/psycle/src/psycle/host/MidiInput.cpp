///\file
///\brief implementation file for psycle::host::CMidiInput.
/// original code 21st April by Mark McCormack (mark_jj_mccormak@yahoo.co.uk) for Psycle - v2.2b -virtually complete-

#include "MidiInput.hpp"

#include "InputHandler.hpp"
#include "Configuration.hpp"
#include "ChildView.hpp"
#include "MainFrm.hpp"

#include "Song.hpp"
#include "Player.hpp"
#include "Plugin.hpp"
#include "VstHost24.hpp"

#include "cpu_time_clock.hpp"

#define BREAK_ON_ERROR(result, code) \
	if(result != MMSYSERR_NOERROR) { problem |= code; goto Exit; }

namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		CMidiInput * CMidiInput::s_Instance(0);	// the current instance

		CMidiInput::CMidiInput() : 
			m_midiInHandlesTried( false ),
			m_bufWriteIdx( 0 ),
			m_bufReadIdx( 0 ),
			m_bufCount( 0 ),
			m_timingCounter( 0 ),
			m_timingAccumulator( 0 ),
			m_resyncAdjStampTime( 0 ),
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
			std::memset( m_channelNoteOff, true, sizeof( bool ) * MAX_MIDI_CHANNELS );	
			std::memset( m_channelController, -1, sizeof( int ) * MAX_MIDI_CHANNELS * MAX_CONTROLLERS );	
			std::memset( m_devId, -1, sizeof( std::uint32_t ) * MAX_DRIVERS );
			std::memset( m_midiInHandle, 0, sizeof( HMIDIIN ) * MAX_DRIVERS );
			std::memset( notetrack, notecommands::release, sizeof( unsigned char ) * MAX_TRACKS );
			std::memset( instrtrack, -1, sizeof( unsigned char ) * MAX_TRACKS );
			currenttrack=0;

			// setup config defaults (override some by registry settings)
			m_config.midiHeadroom = MIDI_PREDELAY_MS;
			std::strcpy( m_config.versionStr, VERSION_STRING );
			
			// clear down stats
			m_stats.flags = 0;
			m_stats.clockDeviation = 0;
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
			bool opened = false;

			// open the main input driver
			if( !m_midiInHandle[ DRIVER_MIDI ] && m_devId[ DRIVER_MIDI ] != -1 )
			{
				result = midiInOpen( &m_midiInHandle[ DRIVER_MIDI ], m_devId[ DRIVER_MIDI ], (DWORD_PTR)fnMidiCallbackStatic, 0, CALLBACK_FUNCTION );
				BREAK_ON_ERROR(result, 0x01)

				opened = true;
				result = midiInStart( m_midiInHandle[ DRIVER_MIDI ] );
				BREAK_ON_ERROR(result, 0x02)

				if( m_devId[ DRIVER_MIDI ] == m_devId[ DRIVER_SYNC ] || m_devId[ DRIVER_SYNC ] == -1)
				{
					m_pc_clock_base = cpu_time_clock().get_count()*0.000001;
				}
				else
				{
					// open
					result = midiInOpen( &m_midiInHandle[ DRIVER_SYNC ], m_devId[ DRIVER_SYNC ], (DWORD_PTR)fnMidiCallbackStatic, 0, CALLBACK_FUNCTION );
					BREAK_ON_ERROR(result, 0x04)

					result = midiInStart( m_midiInHandle[ DRIVER_SYNC ] );
					BREAK_ON_ERROR(result, 0x08)

					m_pc_clock_base = cpu_time_clock().get_count()*0.000001;
				}
			}
Exit:
			// any problems?
			if( !m_midiInHandlesTried && problem )
			{
				CString messageText;

				if(problem & 0x05) {
					messageText = "ERROR: Could not open the ";
				}
				else if(problem & 0x0A) {
					messageText = "ERROR: Could not start the ";
				}
				if( problem & 0x01 )
				{
					messageText += "MIDI input device";
				}
				if( problem & 0x04 )
				{
					messageText += "MIDI sync device";
				}
				messageText += "!";

				m_midiInHandlesTried = true;

				// messagebox to the user
				AfxMessageBox( messageText, MB_ICONEXCLAMATION+MB_OK );
			}
			m_stats.channelMapUpdate = true;
			m_synced = false;
			//Allow MIDI to work even if we don't receive a clock.
			m_timingAccumulator = 0;
			m_bufCount = 0;
			m_bufReadIdx = 0;
			m_bufWriteIdx = 0;

			return problem == 0;
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
				BREAK_ON_ERROR(result, 0x01)

				m_midiInHandle[ DRIVER_MIDI ] = NULL;
			}

			if( m_midiInHandle[ DRIVER_SYNC ] )
			{
				result = midiInClose( m_midiInHandle[ DRIVER_SYNC ] );
				BREAK_ON_ERROR(result, 0x02)

				m_midiInHandle[ DRIVER_SYNC ] = NULL;
			}
Exit:
			// allow messagebox errors again
			m_midiInHandlesTried = false;

			return problem == 0;
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
			if(inst != m_channelInstMap[ channel ]) {
				m_channelInstMap[ channel ] = inst;
				m_stats.channelMapUpdate = true;
			}
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
			if(generator != m_channelGeneratorMap[ channel ])
			{
				m_channelGeneratorMap[ channel ] = generator;
				m_stats.channelMapUpdate = true;
			}
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
		//                : DWORD_PTR dwInstance - user instance data (not used)
		//                : DWORD_PTR dwParam1 - various midi message info
		//                : DWORD_PTR dwParam2 - various midi message info
		// RETURNS		  : <void>

		void CALLBACK CMidiInput::fnMidiCallbackStatic( HMIDIIN handle, std::uint32_t uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
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
		//                : DWORD_PTR dwInstance - user instance data (not used)
		//                : DWORD_PTR dwParam1 - various midi message info
		//                : DWORD_PTR dwParam2 - various midi message info
		// RETURNS		  : <void>

		void CALLBACK CMidiInput::fnMidiCallback_Inject( HMIDIIN handle, std::uint32_t uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
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
					int track = 0;

					// and a bit more...
					int statusHN = (status & 0xF0) >> 4;
					int statusLN = (status & 0x0F);
					int channel = statusLN;

					switch(Global::configuration().midi().gen_select_type())
					{
					case Configuration::midi_type::MS_USE_SELECTED:
						SetGenMap( channel, Global::song().seqBus );
						break;
					case Configuration::midi_type::MS_MIDI_CHAN:
						SetGenMap( channel, channel );
						break;
					}
					switch(Global::configuration().midi().inst_select_type())
					{
					case Configuration::midi_type::MS_USE_SELECTED:
						SetInstMap( channel, Global::song().auxcolSelected );
						break;
					case Configuration::midi_type::MS_MIDI_CHAN:
						SetInstMap( channel, channel );
						break;
					}

					// map channel -> generator
					int busMachine = GetGenMap( channel );
					int inst = GetInstMap( channel );

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
								track = GetTrackToPlay(note, noteOn, inst);
								cmd = 0x0C;
								parameter = p1HiWordLB;

							}
							else
							{
								// note off
								if( m_channelNoteOff[ channel ] )
								{
									track = GetTrackToPlay(note, 0, inst);
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
								track = GetTrackToPlay(note, 0, inst);
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
							if(Global::configuration().midi().gen_select_type() == Configuration::midi_type::MS_PROGRAM)
							{
								// machine active?
								if( program < MAX_MACHINES && Global::_pSong->_pMachine[ program ] )
								{
									// ok, map
									SetGenMap( channel, program );
								}
								else
								{
									// machine not active, can't map!
									SetGenMap( channel, -1 );
								}
								return;
							}
							else if(Global::configuration().midi().inst_select_type() == Configuration::midi_type::MS_PROGRAM)
							{
								SetInstMap( channel, program );
								return;
							}
							else {
								note = notecommands::midicc;
								inst = (status&0xF0) | (inst&0x0F);
								cmd = program;
							}
						}
						break;

						/*
						case 0x0E:
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
									// banks select -> map generator to channel
									if(Global::configuration().midi().gen_select_type() == Configuration::midi_type::MS_BANK)
									{
										// machine active?
										if( program < MAX_MACHINES && Global::_pSong->_pMachine[ data2 ] )
										{
											// ok, map
											SetGenMap( channel, data2 );
										}
										else
										{
											// machine not active, can't map!
											SetGenMap( channel, -1 );
										}
										return;
									}
									// banks select -> map instrument to channel
									else if(Global::configuration().midi().inst_select_type() == Configuration::midi_type::MS_BANK)
									{
										SetInstMap( channel, data2 );
										return;
									}
									else
									{
										note = notecommands::midicc;
										inst = (status&0xF0) | (inst&0x0F);
										cmd = data1;
										parameter = data2;
									}
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
										inst  = gParameter;
										if(Global::_pSong->_pMachine[busMachine] && inst < Global::_pSong->_pMachine[busMachine]->GetNumParams())
										{	
											int minval, maxval;
											Global::_pSong->_pMachine[busMachine]->GetParamRange(inst, minval, maxval);
											int value = minval + helpers::math::lround<int, float>( data2 * (maxval-minval) / 127.f);
											cmd = value / 256;
											parameter = value % 256;
										}
										else
											parameter = data2;
									}
									else if (Global::configuration().midi().raw())
									{
										note = notecommands::midicc;
										inst  = (status&0xF0) | (inst&0x0F);
										cmd = data1;
										parameter = data2;
									}
									else
									{
										// search if there's a remap
										int i;
										for(i =0 ; i < 16 ; ++i)
										{
											if(Global::pConfig->midi().group(i).record() && (Global::pConfig->midi().group(i).message() == data1 ))
											{
												int const value(Global::pConfig->midi().group(i).from() + (Global::pConfig->midi().group(i).to() - Global::pConfig->midi().group(i).from()) * data2 / 127);
												switch(Global::pConfig->midi().group(i).type())
												{
													case 0:
														note = notecommands::empty;
														inst = 255;
														cmd = Global::pConfig->midi().group(i).command();
														parameter = value;
														break;
													case 1:
														note = notecommands::tweak;
														inst = Global::pConfig->midi().group(i).command();
														cmd = (value>>8)&255;
														parameter = value&255;
														break;
													case 2:
														note = notecommands::tweakslide;
														inst = Global::pConfig->midi().group(i).command();
														cmd = (value>>8)&255;
														parameter = value&255;
														break;
													case 3:
														note = notecommands::midicc;
														inst = Global::pConfig->midi().group(i).command() | (inst&0x0F);
														cmd = data1;
														parameter = data2;
														break;
													case 4:
														note = notecommands::empty;
														inst = data2;
														cmd = 0;
														parameter = 0;
														break;
												}
												break;
											}
										}

										// controller not setup, we can't do anything
										if (i==16)
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
					if( m_bufCount >= MIDI_BUFFER_SIZE )
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
					int patIn = m_bufWriteIdx;
					PatternEntry * pEntry = &m_midiBuffer[ patIn ].entry;

					pEntry->_note = note;
					pEntry->_mach = busMachine;
					pEntry->_inst = inst;
					pEntry->_cmd = cmd;
					pEntry->_parameter = parameter;

					// add the other necessary info
					m_midiBuffer[ patIn ].timeStamp = dwParam2;
					m_midiBuffer[ patIn ].channel = channel;
					m_midiBuffer[ patIn ].track = track;
					m_stats.channelMap |= (1 << channel);

					// advance IN pointer
					m_bufCount++;
					m_bufWriteIdx++;
					if( m_bufWriteIdx >= MIDI_BUFFER_SIZE ) m_bufWriteIdx = 0;
					m_stats.flags |= FSTAT_IN_BUFFER;

					// this the 1st message, we are NOT synced yet, sync to audio engine now!
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
		//                : DWORD_PTR dwInstance - user instance data (not used)
		//                : DWORD_PTR dwParam1 - various midi message info
		//                : DWORD_PTR dwParam2 - various midi message info
		// RETURNS		  : <void>

		void CALLBACK CMidiInput::fnMidiCallback_Step( HMIDIIN handle, std::uint32_t uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
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
					// and a bit more...
					int statusHN = (status & 0xF0) >> 4;
					int statusLN = (status & 0x0F);
					int channel = statusLN;

					switch(Global::configuration().midi().gen_select_type())
					{
					case Configuration::midi_type::MS_USE_SELECTED:
						SetGenMap( channel, Global::song().seqBus );
						break;
					case Configuration::midi_type::MS_MIDI_CHAN:
						SetGenMap( channel, channel );
						break;
					}
					switch(Global::configuration().midi().inst_select_type())
					{
					case Configuration::midi_type::MS_USE_SELECTED:
						SetInstMap( channel, Global::song().auxcolSelected );
						break;
					case Configuration::midi_type::MS_MIDI_CHAN:
						SetInstMap( channel, channel );
						break;
					}

					// map channel -> generator
					int busMachine = GetGenMap( channel );
					int inst = GetInstMap( channel );

//					int cmd = 0; cmd; // not used
//					int parameter = 0; parameter; // not used
					
					CMainFrame & frame(*static_cast<CMainFrame*>(theApp.m_pMainWnd));
					m_stats.channelMap |= (1 << channel);

					// branch on status code
					switch( statusHN )
					{
						// (also) note off
						case 0x08:		
							velocity=0;
						//fallthrough
						case 0x09:
    						// limit to playable range (above this is special codes)
    						if(note>119) 
    							note=119;
							// TODO: watch this, it should be OK as long as we don't change things too much
							frame.m_wndView.MidiPatternNote(note, busMachine, inst, velocity);
							return;
						// controller
						case 0x0B:
						{
							// switch on controller ID
							switch( data1 )
							{
								// BANK SELECT (controller 0)
								case 0:
								{
									// banks select -> map generator to channel
									if(Global::configuration().midi().gen_select_type() == Configuration::midi_type::MS_BANK)
									{
										// machine active?
										if( data2 < MAX_MACHINES && Global::_pSong->_pMachine[ data2 ] )
										{
											// ok, map
											SetGenMap( channel, data2 );
										}
										else
										{
											// machine not active, can't map!
											SetGenMap( channel, -1 );
										}
										return;
									}
									// banks select -> map instrument to channel
									else if(Global::configuration().midi().inst_select_type() == Configuration::midi_type::MS_BANK)
									{
										SetInstMap( channel, data2 );
										return;
									}
								}
								default:break;
							}
						}
						// program change
						case 0x0C:
						{
							// program change -> map generator/effect to channel
							if(Global::configuration().midi().gen_select_type() == Configuration::midi_type::MS_PROGRAM)
							{
								// machine active?
								if( data1 < MAX_MACHINES && Global::_pSong->_pMachine[ data1 ] )
								{
									// ok, map
									SetGenMap( channel, data1 );
								}
								else
								{
									// machine not active, can't map!
									SetGenMap( channel, -1 );
								}
								return;
							}
							else if(Global::configuration().midi().inst_select_type() == Configuration::midi_type::MS_PROGRAM)
							{
								SetInstMap( channel, data1 );
								return;
							}
						}
						default:break;
					}

					if (Global::pConfig->_RecordTweaks)
					{
						if (Global::pConfig->midi().raw() && status != 0xFE )
						{
							frame.m_wndView.MidiPatternMidiCommand(status,(data1 << 8) | data2);
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
											frame.m_wndView.MidiPatternMidiCommand(status | (inst&0x0F), (data1 << 8) | data2);
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
						if (busMachine >= 0 && busMachine < MAX_MACHINES)
						{
							Machine* pMachine = Global::_pSong->_pMachine[busMachine];
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
									PatternEntry pentry(notecommands::midicc,status,255,data1,data2,busMachine);
#else
									PatternEntry pentry(notecommands::midicc,status,busMachine,data1,data2);
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
		// PARAMETERS     : DWORD_PTR dwParam2 - timing stamp
		// RETURNS		  : <void>

		void CMidiInput::InternalClock( DWORD_PTR dwParam2 )
		{
			int pcClock = cpu_time_clock().get_count()*0.000001 - m_resyncClockBase;
			int midiClock = dwParam2 - m_resyncMidiStampTime;
			// calc the deviation of the MIDI clock.
			int clockDeviation = pcClock - midiClock;
			m_stats.clockDeviation = clockDeviation;

			// get the current play sample position
 			int playPos = Global::pConfig->_pOutputDriver->GetPlayPosInSamples();
			int samplesPerSecond = Global::player().SampleRate();

			// convert this back to a ms counter
			DWORD playMs = (DWORD)(((playPos - m_resyncPlayPos)/(float)samplesPerSecond) * 1000.0f);

			// create offset (ms)
			m_fixOffset = playMs - midiClock;
			m_stats.syncOffset = m_fixOffset;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// InternalReSync
		//
		// DESCRIPTION	  : Instigates an MIDI->audio resync
		// PARAMETERS     : DWORD_PTR dwParam2 - timing stamp
		// RETURNS		  : <void>

		void CMidiInput::InternalReSync( DWORD_PTR dwParam2 )
		{
 			m_resyncClockBase = cpu_time_clock().get_count()*0.000001;
 			m_resyncPlayPos = Global::pConfig->_pOutputDriver->GetPlayPosInSamples();

			// save vars
			m_resyncMidiStampTime = dwParam2;
			m_resyncAdjStampTime = dwParam2;

			// set vars
			m_synced = true;
			m_syncing = true;
			m_stats.clockDeviation = 0;
			m_stats.syncOffset = 0;
			m_fixOffset = 0;
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// InjectMIDI
		//
		// DESCRIPTION	  : Called from the player to perform the custom midi injection
		// PARAMETERS     : int amount - amount of block samples
		// RETURNS		  : bool - true if we did some processing, false if nothing was done

		void CMidiInput::InjectMIDI( int amount )
		{
			if(m_midiMode != MODE_REALTIME)
			{
				return;
			}

			int samplesPerSecond = Global::player().SampleRate();
			m_stats.bufferCount = m_bufCount;

			if( m_syncing )
			{
				m_stats.flags |= FSTAT_SYNC;
				// get the write position
				int writePos = Global::pConfig->_pOutputDriver->GetWritePosInSamples() + Global::player().sampleOffset;
				
				// calculate our final adjuster
				int syncAdjuster = writePos - m_resyncPlayPos;
				m_stats.syncAdjuster = syncAdjuster;

				// adjust the time base
				m_resyncAdjStampTime += (syncAdjuster * 1000/samplesPerSecond) - m_config.midiHeadroom;

				// initial accumulator val
				m_timingAccumulator = 0;
				m_syncing = false;
			}

			// create our working stamp (account for timing fix)
			DWORD tbaseStampTime = m_resyncAdjStampTime - m_fixOffset;

			// accumulate sample count
			m_timingAccumulator += amount;

			// convert accumulator to milliseconds
			m_timingCounter = (DWORD)((m_timingAccumulator * 1000.0f)/samplesPerSecond);

			// remove any midi data that happend over before the base stamp
			while( m_bufCount && m_midiBuffer[ m_bufReadIdx ].timeStamp < tbaseStampTime )
			{
				// advance OUT pointer
				m_bufCount--;
				m_bufReadIdx++;
				if( m_bufReadIdx >= MIDI_BUFFER_SIZE ) m_bufReadIdx = 0;

				m_stats.eventsLost++;
				m_stats.flags |= FSTAT_CLEAR_BUFFER;
			}

			// NO midi data that need to be injected during this amount block?
			if( !m_bufCount || m_timingCounter < (m_midiBuffer[ m_bufReadIdx ].timeStamp - tbaseStampTime ) )
			{
				return;
			}

			// NOTE: because we are inserting all the machine ticks before working the master
			// we will always have the notes being slightly advanced of the song.  How much will
			// be a factor of the current sample-rate and amount of samples.  For example, at
			// 44100hz with a 256 sample block the vary will be up to 5.8ms, at a lower sample
			// rate, say 22050hz, but same block size it increases up to 11.6ms.

			// VSTs support sending timestamp with the event, and that should be the solution if 
			// it is ever tried.

			m_stats.flags |= FSTAT_OUT_BUFFER;

			// OK, if we get here then there is at least one MIDI message that needs injecting
			do
			{
				int note = m_midiBuffer[ m_bufReadIdx ].entry._note;
				int machine = m_midiBuffer[ m_bufReadIdx ].entry._mach;
				int track = m_midiBuffer[ m_bufReadIdx ].track;

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
						case notecommands::tweak:
						{
							pMachine->Tick(track , &m_midiBuffer[ m_bufReadIdx ].entry );
						}
						break;

						// SYNC TICK
						case notecommands::midi_sync:
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
							pMachine->Tick( track, &m_midiBuffer[ m_bufReadIdx ].entry );
						}
						break;

					}	// end of note switch
				}	// end of if 'is machine still active?'

				// advance OUT pointer
				m_bufCount--;
				m_bufReadIdx++;
				if( m_bufReadIdx >= MIDI_BUFFER_SIZE ) m_bufReadIdx = 0;

			} while( m_bufCount && m_timingCounter >= (m_midiBuffer[ m_bufReadIdx ].timeStamp - tbaseStampTime) );
		}

		int CMidiInput::GetTrackToPlay(int note, int velocity, int instNo)
		{
			if (velocity == 0)
			{
				int i;
				for (i = 0; i < Global::_pSong->SONGTRACKS; i++)
				{
					if (notetrack[i] == note && instrtrack[i] == instNo)
					{
						break;
					}
				}
				if(i >= Global::_pSong->SONGTRACKS) i=0;
				currenttrack = i;
				notetrack[currenttrack] = notecommands::release;
				instrtrack[currenttrack] = instNo;
			}
			else
			{
				int i;
				for (i = currenttrack+1; i < Global::_pSong->SONGTRACKS; i++)
				{
					if (notetrack[i] == notecommands::release)
					{
						break;
					}
				}
				if (i >= Global::_pSong->SONGTRACKS)
				{
					for (i = 0; i <= currenttrack; i++)
					{
						if (notetrack[i] == notecommands::release)
						{
							break;
						}
					}
				}
				currenttrack = i;
				notetrack[currenttrack] = note;
				instrtrack[currenttrack] = instNo;
			}
			return currenttrack;
		}
	}
}
