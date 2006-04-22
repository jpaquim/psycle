///\file
///\brief implementation file for psycle::host::Player.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/engine/player.hpp>
#include <psycle/host/engine/song.hpp>
#include <psycle/host/engine/machine.hpp>
#include <psycle/host/configuration.hpp>
#include <psycle/host/engine/MidiInput.hpp>
#include <psycle/host/gui/InputHandler.hpp>
namespace psycle
{
	namespace host
	{
		Player::Player(Song & song)
		:
			song_(&song),
			_playing(false),
			_playBlock(false),
			_recording(false),
			Tweaker(false),
			_samplesRemaining(0),
			_lineCounter(0),
			_loopSong(true),
			_patternjump(-1),
			_linejump(-1),
			_loop_count(0),
			_loop_line(0),
			m_SampleRate(44100),
			m_SamplesPerRow((44100*60)/(125*4)),
			tpb(4),
			bpm(125)
		{
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i]=255;
		}

		Player::~Player()
		{
			if(_recording) _outputWaveFile.Close();
		}

		void Player::Start(int pos, int line)
		{
			Stop(); // This causes all machines to reset, and samplesperRow to init.
			((Master*)(song()._pMachine[MASTER_INDEX]))->_clip = false;
			((Master*)(song()._pMachine[MASTER_INDEX]))->sampleCount = 0;
			_lineChanged = true;
			_lineCounter = line;
			_SPRChanged = false;
			_playPosition= pos;
			_playPattern = song().playOrder[_playPosition];
			_playTime = 0;
			_playTimem = 0;
			_loop_count =0;
			_loop_line = 0;
			SetBPM(Global::song().BeatsPerMin(),Global::song().LinesPerBeat());
			SampleRate(Global::pConfig->_pOutputDriver->_samplesPerSec);
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 255;
			_playing = true;
			ExecuteLine();
		}

		void Player::Stop(void)
		{
			// Stop song enviroment
			_playing = false;
			_playBlock = false;
			for(int i=0; i<MAX_MACHINES; i++)
			{
				if(Global::song()._pMachine[i])
				{
					Global::song()._pMachine[i]->Stop();
					for(int c = 0; c < MAX_TRACKS; c++) Global::song()._pMachine[i]->TriggerDelay[c]._cmd = 0;
				}
			}
			SetBPM(Global::song().BeatsPerMin(),Global::song().LinesPerBeat());
			SampleRate(Global::pConfig->_pOutputDriver->_samplesPerSec);
		}

		void Player::SampleRate(const int sampleRate)
		{
			///\todo update the source code of the plugins...
			if(m_SampleRate != sampleRate)
			{
				m_SampleRate = sampleRate;
				RecalcSPR();
				for(int i(0) ; i < MAX_MACHINES; ++i)
				{
					if(Global::song()._pMachine[i]) Global::song()._pMachine[i]->SetSampleRate(sampleRate);
				}
			}
		}
		void Player::SetBPM(float _bpm,int _tpb)
		{
			if ( _tpb != 0) tpb=_tpb;
			if ( _bpm != 0) bpm=_bpm;
			RecalcSPR();
			//\todo : Find out if we should notify the plugins of this change.
		}

		void Player::ExecuteLine(void)
		{
			ExecuteGlobalCommands();
			NotifyNewLine();
			ExecuteNotes();
		}
		// Initial Loop. Read new line and Interpret the Global commands.
		void Player::ExecuteGlobalCommands(void)
		{
			_patternjump = -1;
			_linejump = -1;
			int mIndex = 0;
			unsigned char* const plineOffset = song()._ptrackline(_playPattern,0,_lineCounter);

			for(int track=0; track<song().tracks(); track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
				if(pEntry->_note < cdefTweakM || pEntry->_note == 255) // If This isn't a tweak (twk/tws/mcm) then do
				{
					switch(pEntry->_cmd)
					{
					case PatternCmd::SET_TEMPO:
						if(pEntry->_parameter != 0)
						{	//\todo: implement the Tempo slide
							// SET_SONG_TEMPO=			20, // T0x Slide tempo down . T1x slide tempo up
							float bpmFine = song().BeatsPerMin() -floor(song().BeatsPerMin());
							bpm = pEntry->_parameter + bpmFine;
							RecalcSPR();
						}
						break;
					case PatternCmd::EXTENDED:
						if(pEntry->_parameter != 0)
						{
							if ( (pEntry->_parameter&0xE0) == 0 ) // range from 0 to 1F for LinesPerBeat.
							{
								tpb=pEntry->_parameter;
								RecalcSPR();
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_BYPASS )
							{
								mIndex = pEntry->_mach;
								if ( mIndex < MAX_MACHINES && song()._pMachine[mIndex] && song()._pMachine[mIndex]->_mode == MACHMODE_FX )
								{
									if ( pEntry->_parameter&0x0F )
										song()._pMachine[mIndex]->_bypass = true;
									else
										song()._pMachine[mIndex]->_bypass = false;
								}
							}

							else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_MUTE )
							{
								mIndex = pEntry->_mach;
								if ( mIndex < MAX_MACHINES && song()._pMachine[mIndex] && song()._pMachine[mIndex]->_mode == MACHMODE_FX )
								{
									if ( pEntry->_parameter&0x0F )
										song()._pMachine[mIndex]->_mute = true;
									else
										song()._pMachine[mIndex]->_mute = false;
								}
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::PATTERN_DELAY )
							{
								SamplesPerRow(SamplesPerRow()*(1+(pEntry->_parameter&0x0F)));
								_SPRChanged=true;
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::FINE_PATTERN_DELAY)
							{
								SamplesPerRow(SamplesPerRow()*(1.0f+((pEntry->_parameter&0x0F)*tpb/24.0f)));
								_SPRChanged=true;
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::PATTERN_LOOP)
							{
								int value = pEntry->_parameter&0x0F;
								if (value == 0 )
								{
									_loop_line = _lineCounter;
								} else {
									if ( _loop_count == 0 )
									{ 
										_loop_count = value;
										_linejump = _loop_line;
									} else {
										if (--_loop_count) _linejump = _loop_line;
										else _loop_line = _lineCounter+1; //This prevents infinite loop in specific cases.
									}
								}
							}
						}
						break;
					case PatternCmd::JUMP_TO_ORDER:
						if ( pEntry->_parameter < song().playLength ){
							_patternjump=pEntry->_parameter;
							_linejump=0;
						}
						break;
					case PatternCmd::BREAK_TO_LINE:
						if (_patternjump ==-1) 
						{
							_patternjump=(_playPosition+1>=song().playLength)?0:_playPosition+1;
						}
						if ( pEntry->_parameter >= song().patternLines[_patternjump])
						{
							_linejump = song().patternLines[_patternjump];
						} else { _linejump= pEntry->_parameter; }
						break;
					case PatternCmd::SET_VOLUME:
						if(pEntry->_mach == 255)
						{
							((Master*)(song()._pMachine[MASTER_INDEX]))->_outDry = pEntry->_parameter;
						}
						else 
						{
							Machine::id_type mIndex(pEntry->_mach);
							if(mIndex < MAX_MACHINES)
							{
								Wire::id_type wire(pEntry->_inst);
								if(song()._pMachine[mIndex]) song()._pMachine[mIndex]->SetDestWireVolume(mIndex,wire,CValueMapper::Map_255_1(pEntry->_parameter));
							}
						}
						break;
					case  PatternCmd::SET_PANNING:
						mIndex = pEntry->_mach;
						if(mIndex < MAX_MACHINES)
						{
							if(song()._pMachine[mIndex]) song()._pMachine[mIndex]->SetPan(pEntry->_parameter>>1);
						}

						break;
					}
				}
				// Check For Tweak or MIDI CC
				else if(!song()._trackMuted[track])
				{
					int mac = pEntry->_mach;
					if((mac != 255) || (prevMachines[track] != 255))
					{
						if(mac != 255) prevMachines[track] = mac;
						else mac = prevMachines[track];
						if(mac < MAX_MACHINES)
						{
							Machine *pMachine = song()._pMachine[mac];
							if(pMachine)
							{
								if(pEntry->_note == cdefMIDICC && pMachine->_type != MACH_VST && pMachine->_type != MACH_VSTFX)
								{
									// for native machines,
									// use the value in the "instrument" field of the event as a voice number
									int voice(pEntry->_inst);
									// make a copy of the pattern entry, because we're going to modify it.
									PatternEntry entry(*pEntry);
									entry._inst = 0;
									// check for out of range voice values (with the classic tracker way, it's the same as the pattern tracks)
									if(voice < song().tracks())
									{
										pMachine->Tick(voice, &entry);
									}
									else if(voice == 0xff)
									{
										// special voice value which means we want to send the same command to all voices
										for(int voice(0) ; voice < song().tracks() ; ++voice)
										{
											pMachine->Tick(voice, &entry);
										}
									}
									else ; // probably an out of range voice value (with the classic tracker way, it's limited to the number of pattern tracks)
								}
								else // midi cc for vst, or other commands
								{
									// for vst machines,
									// classic tracking, use the track number as the channel/voice number
									pMachine->Tick(track, pEntry);
								}
							}
						}
					}
				}
			}
		}

		// Notify all machines that a new Tick() comes.
		void Player::NotifyNewLine(void)
		{
			for(int tc=0; tc<MAX_MACHINES; tc++)
			{
				if(song()._pMachine[tc])
				{
					song()._pMachine[tc]->Tick();
					for(int c = 0; c < MAX_TRACKS; c++) song()._pMachine[tc]->TriggerDelay[c]._cmd = 0;
				}
			}

		}

		/// Final Loop. Read new line for notes to send to the Machines
		void Player::ExecuteNotes(void)
		{
			unsigned char* const plineOffset = song()._ptrackline(_playPattern,0,_lineCounter);
			for(int track=0; track<song().tracks(); track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
				if(( !song()._trackMuted[track]) && (pEntry->_note < cdefTweakM || pEntry->_note == 255)) // Is it not muted and is a note?
				{
					int mac = pEntry->_mach;
					if(mac != 255) prevMachines[track] = mac;
					else mac = prevMachines[track];
//					if( mac != 255 && (pEntry->_note != 255 || pEntry->_cmd != 0x00) ) // is there a machine number and it is either a note or a command?
					if( mac != 255 ) // is there a machine number and it is either a note or a command?
					{
						if(mac < MAX_MACHINES) //looks like a valid machine index?
						{
							Machine *pMachine = song()._pMachine[mac];
							if(pMachine && !(pMachine->_mute)) // Does this machine really exist and is not muted?
							{
								if(pEntry->_cmd == PatternCmd::NOTE_DELAY)
								{
									// delay
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									pMachine->TriggerDelayCounter[track] = ((pEntry->_parameter+1)*SamplesPerRow())/256;
								}
								else if(pEntry->_cmd == PatternCmd::RETRIGGER)
								{
									// retrigger
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									pMachine->RetriggerRate[track] = (pEntry->_parameter+1);
									pMachine->TriggerDelayCounter[track] = 0;
								}
								else if(pEntry->_cmd == PatternCmd::RETR_CONT)
								{
									// retrigger continue
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									if(pEntry->_parameter&0xf0) pMachine->RetriggerRate[track] = (pEntry->_parameter&0xf0);
								}
								else if (pEntry->_cmd == PatternCmd::ARPEGGIO)
								{
									// arpeggio
									//\todo : Add Memory.
									//\todo : This won't work... What about sampler's NNA's?
									if (pEntry->_parameter)
									{
										memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
										pMachine->ArpeggioCount[track] = 1;
									}
									pMachine->RetriggerRate[track] = SamplesPerRow()*tpb/24;
								}
								else
								{
									pMachine->TriggerDelay[track]._cmd = 0;
									pMachine->Tick(track, pEntry);
									pMachine->TriggerDelayCounter[track] = 0;
									pMachine->ArpeggioCount[track] = 0;
								}
							}
						}
					}
				}
			}
			_samplesRemaining = SamplesPerRow();
		}	


		void Player::AdvancePosition()
		{
			if ( _patternjump!=-1 ) _playPosition= _patternjump;
			if ( _SPRChanged ) { RecalcSPR(); _SPRChanged = true; }
			if ( _linejump!=-1 ) _lineCounter=_linejump;
			else _lineCounter++;
			_playTime += 60 / float (bpm * tpb);
			if(_playTime>60)
			{
				_playTime-=60;
				_playTimem++;
			}
			if(_lineCounter >= song().patternLines[_playPattern])
			{
				_lineCounter = 0;
				if(!_playBlock)
					_playPosition++;
				else
				{
					_playPosition++;
					while(_playPosition< song().playLength && (!song().playOrderSel[_playPosition]))
						_playPosition++;
				}
			}
			if( _playPosition >= song().playLength)
			{	
				// Don't loop the recording
				if(_recording)
				{
					StopRecording();
				}
				if( _loopSong )
				{
					_playPosition = 0;
					if(( _playBlock) && (song().playOrderSel[_playPosition] == false))
					{
						while((!song().playOrderSel[_playPosition]) && ( _playPosition< song().playLength)) _playPosition++;
					}
				}
				else 
				{
					_playing = false;
					_playBlock =false;
				}
			}
			// this is outside the if, so that _patternjump works
			_playPattern = song().playOrder[_playPosition];
			_lineChanged = true;
		}

		float * Player::Work(void* context, int & numSamples)
		{
			return reinterpret_cast<Player*>(context)->Work(numSamples);
		}

		float * Player::Work(int & numSamples)
		{
			int amount;
			Master::_pMasterSamples = _pBuffer;
			int numSamplex = numSamples;
			#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
				#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
			#else
				#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
					boost::read_write_mutex::scoped_read_write_lock lock(song().read_write_mutex(),boost::read_write_lock_state::read_locked);
				#else // original implementation
					CSingleLock crit(&song().door, TRUE);
				#endif
			#endif
			do
			{
				if(numSamplex > MAX_BUFFER_LENGTH) amount = MAX_BUFFER_LENGTH; else amount = numSamplex;
				// Tick handler function
				if((_playing) && (amount >= _samplesRemaining)) amount = _samplesRemaining;
				// Song play
				if((_samplesRemaining <=0))
				{
					if (_playing)
					{
						// Advance position in the sequencer
						AdvancePosition();
						// Global commands are executed first so that the values for BPM and alike
						// are up-to-date when "NotifyNewLine()" is called.
						ExecuteGlobalCommands();
						NotifyNewLine();
						ExecuteNotes();
					}
					else
					{
						NotifyNewLine();
					}
				}
				// Processing plant
				if(amount > 0)
				{
					PSYCLE__CPU_COST__INIT(idletime);
					if( (int)song()._sampCount > Global::pConfig->_pOutputDriver->_samplesPerSec)
					{
						song()._sampCount =0;
						for(int c=0; c<MAX_MACHINES; c++)
						{
							if(song()._pMachine[c])
							{
								song()._pMachine[c]->wire_cpu_cost(0);
								song()._pMachine[c]->work_cpu_cost(0);
							}
						}
					}
					// Reset all machines
					for(int c=0; c<MAX_MACHINES; c++)
					{
						if(song()._pMachine[c]) song()._pMachine[c]->PreWork(amount);
					}
					if(song().PW_Stage == 1)
					{
						// Mixing preview WAV
						song().PW_Work(song()._pMachine[MASTER_INDEX]->_pSamplesL,song()._pMachine[MASTER_INDEX]->_pSamplesR, amount);
					}
					// Inject Midi input data
					if(!CMidiInput::Instance()->InjectMIDI( amount ))
					{
						// if midi not enabled we just do the original tracker thing
						// Master machine initiates work
						song()._pMachine[MASTER_INDEX]->Work(amount);
					}
					PSYCLE__CPU_COST__CALCULATE(idletime, amount);
					song().cpu_idle(idletime);
					song()._sampCount += amount;
					if((_playing) && (_recording))
					{
						float* pL(song()._pMachine[MASTER_INDEX]->_pSamplesL);
						float* pR(song()._pMachine[MASTER_INDEX]->_pSamplesR);
						int i;
						switch(Global::pConfig->_pOutputDriver->_channelmode)
						{
						case 0: // mono mix
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample(((*pL++)+(*pR++))/2) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						case 1: // mono L
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample((*pL++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						case 2: // mono R
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample((*pR++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						default: // stereo
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteStereoSample((*pL++),(*pR++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						}
					}
					Master::_pMasterSamples += amount * 2;
					numSamplex -= amount;
				}
				if(_playing) _samplesRemaining -= amount;
			} while(numSamplex>0); ///\todo this is strange. <JosepMa> It is not strange. Simply numSamples doesn't need anymore to be passed as reference.
			return _pBuffer;
		}

		void Player::StartRecording(std::string psFilename, int bitdepth, int samplerate, int channelmode)
		{
			if(!_recording)
			{
				//\todo: Upgrade all the playing functions to use m_SampleRate instead of pOutputdriver->samplesPerSec
				backup_rate = Global::pConfig->_pOutputDriver->_samplesPerSec;
				backup_bits = Global::pConfig->_pOutputDriver->_bitDepth;
				backup_channelmode = Global::pConfig->_pOutputDriver->_channelmode;
				if(samplerate > 0) { SampleRate(samplerate); Global::pConfig->_pOutputDriver->_samplesPerSec = samplerate; }
				if(bitdepth > 0) Global::pConfig->_pOutputDriver->_bitDepth = bitdepth;
				if(channelmode >= 0) Global::pConfig->_pOutputDriver->_channelmode = channelmode;
				int channels = 2;
				if(Global::pConfig->_pOutputDriver->_channelmode != 3) channels = 1;
				Stop();
				if(_outputWaveFile.OpenForWrite(psFilename.c_str(), Global::pConfig->_pOutputDriver->_samplesPerSec, Global::pConfig->_pOutputDriver->_bitDepth, channels) == DDC_SUCCESS)
					_recording = true;
				else
				{
					StopRecording(false);
				}
			}
		}

		void Player::StopRecording(bool bOk)
		{
			if(_recording)
			{
				Global::pConfig->_pOutputDriver->_samplesPerSec = backup_rate;
				SampleRate(backup_rate);
				Global::pConfig->_pOutputDriver->_bitDepth = backup_bits;
				Global::pConfig->_pOutputDriver->_channelmode = backup_channelmode;
				_outputWaveFile.Close();
				_recording = false;
				if(!bOk)
				{
					MessageBox(0, "Wav recording failed.", "ERROR", MB_OK);
				}
			}
		}
	}
}
