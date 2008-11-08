///\file
///\brief implementation file for psycle::host::Player.
#include <psycle/project.private.hpp>
#include "player.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "Configuration.hpp"

#if !defined WINAMP_PLUGIN
	#include "MidiInput.hpp"
	#include "InputHandler.hpp"
#endif //!defined WINAMP_PLUGIN

#include <seib-vsthost/CVSTHost.Seib.hpp> // Included to interact directly with the host.
#include "global.hpp"
namespace psycle
{
	namespace host
	{
		using namespace seib::vst;
		Player::Player()
		{
			_playing = false;
			_playBlock = false;
			_recording = false;
			_clipboardrecording = false;
			_isWorking = false;
			Tweaker = false;
			_samplesRemaining=0;
			_lineCounter=0;
			_lineStop=-1;
			_loopSong=true;
			_patternjump=-1;
			_linejump=-1;
			_loop_count=0;
			_loop_line=0;
			m_SampleRate=44100;
			SetBPM(125,4);
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i]=255;
		}

		Player::~Player()
		{
#if !defined WINAMP_PLUGIN
			if(_recording) _outputWaveFile.Close();
#endif //!defined WINAMP_PLUGIN
		}

		void Player::Start(int pos, int lineStart, int lineStop, bool initialize)
		{
			_lineStop = lineStop;
			Start(pos, lineStart, initialize);
		}

		void Player::Start(int pos, int line, bool initialize)
		{
			CSingleLock crit(&Global::_pSong->door, TRUE);
			if (initialize)
			{
				Stop(); // This causes all machines to reset, and samplesperRow to init.				
				Work(this,256);
				((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->_clip = false;
				((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->sampleCount = 0;
			}
			_lineChanged = true;
			_lineCounter = line;
			_SPRChanged = false;
			_playPosition= pos;
			_playPattern = Global::_pSong->playOrder[_playPosition];
			if (initialize)
			{
				_playTime = 0;
				_playTimem = 0;
			}
			_loop_count =0;
			_loop_line = 0;
			if (initialize)
			{
				SetBPM(Global::_pSong->BeatsPerMin(),Global::_pSong->LinesPerBeat());
				SampleRate(Global::pConfig->_pOutputDriver->_samplesPerSec);
				for(int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 255;
				_playing = true;
			}
			CVSTHost::vstTimeInfo.flags |= kVstTransportPlaying;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
			ExecuteLine();
			_samplesRemaining = SamplesPerRow();
		}

		void Player::Stop(void)
		{
			CSingleLock crit(&Global::_pSong->door, TRUE);

			if (_playing == true)
				_lineStop = -1;

			// Stop song enviroment
			_playing = false;
			_playBlock = false;			
			for(int i=0; i<MAX_MACHINES; i++)
			{
				if(Global::_pSong->_pMachine[i])
				{
					Global::_pSong->_pMachine[i]->Stop();
					for(int c = 0; c < MAX_TRACKS; c++) Global::_pSong->_pMachine[i]->TriggerDelay[c]._cmd = 0;
				}
			}
			SetBPM(Global::_pSong->BeatsPerMin(),Global::_pSong->LinesPerBeat());
			SampleRate(Global::pConfig->_pOutputDriver->_samplesPerSec);
			CVSTHost::vstTimeInfo.flags &= ~kVstTransportPlaying;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
		}

		void Player::SampleRate(const int sampleRate)
		{
			///\todo update the source code of the plugins...
#if PSYCLE__CONFIGURATION__RMS_VUS
			helpers::dsp::numRMSSamples=sampleRate*0.05f;
#endif
			if(m_SampleRate != sampleRate)
			{
				m_SampleRate = sampleRate;
				RecalcSPR();
				CVSTHost::pHost->SetSampleRate(sampleRate);
				for(int i(0) ; i < MAX_MACHINES; ++i)
				{
					if(Global::_pSong->_pMachine[i]) Global::_pSong->_pMachine[i]->SetSampleRate(sampleRate);
				}
			}
		}
		void Player::SetBPM(int _bpm,int _tpb)
		{
			if ( _tpb != 0) tpb=_tpb;
			if ( _bpm != 0) bpm=_bpm;
			RecalcSPR();
			CVSTHost::vstTimeInfo.tempo = bpm;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
			CVSTHost::vstTimeInfo.flags |= kVstTempoValid;
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
			Song* pSong = Global::_pSong;
			_patternjump = -1;
			_linejump = -1;
			int mIndex = 0;
			unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);

			for(int track=0; track<pSong->SONGTRACKS; track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
				if(pEntry->_note < notecommands::tweak || pEntry->_note == 255) // If This isn't a tweak (twk/tws/mcm) then do
				{
					switch(pEntry->_cmd)
					{
					case PatternCmd::SET_TEMPO:
						if(pEntry->_parameter != 0)
						{	///\todo: implement the Tempo slide
							// SET_SONG_TEMPO=			20, // T0x Slide tempo down . T1x slide tempo up
							SetBPM(pEntry->_parameter);
						}
						break;
					case PatternCmd::EXTENDED:
						if(pEntry->_parameter != 0)
						{
							if ( (pEntry->_parameter&0xE0) == 0 ) // range from 0 to 1F for LinesPerBeat.
							{
								SetBPM(0,pEntry->_parameter);
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_BYPASS )
							{
								mIndex = pEntry->_mach;
								if ( mIndex < MAX_MACHINES && pSong->_pMachine[mIndex] && pSong->_pMachine[mIndex]->_mode == MACHMODE_FX )
								{
									if ( pEntry->_parameter&0x0F )
										pSong->_pMachine[mIndex]->Bypass(true);
									else
										pSong->_pMachine[mIndex]->Bypass(false);
								}
							}

							else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_MUTE )
							{
								mIndex = pEntry->_mach;
								if ( mIndex < MAX_MACHINES && pSong->_pMachine[mIndex] && pSong->_pMachine[mIndex]->_mode != MACHMODE_MASTER )
								{
									if ( pEntry->_parameter&0x0F )
										pSong->_pMachine[mIndex]->_mute = true;
									else
										pSong->_pMachine[mIndex]->_mute = false;
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
						if ( pEntry->_parameter < pSong->playLength ){
							_patternjump=pEntry->_parameter;
							_linejump=0;
						}
						break;
					case PatternCmd::BREAK_TO_LINE:
						if (_patternjump ==-1) 
						{
							_patternjump=(_playPosition+1>=pSong->playLength)?0:_playPosition+1;
						}
						if ( pEntry->_parameter >= pSong->patternLines[_patternjump])
						{
							_linejump = pSong->patternLines[_patternjump];
						} else { _linejump= pEntry->_parameter; }
						break;
					case PatternCmd::SET_VOLUME:
						if(pEntry->_mach == 255)
						{
							((Master*)(pSong->_pMachine[MASTER_INDEX]))->_outDry = pEntry->_parameter;
						}
						else 
						{
							int mIndex = pEntry->_mach;
							if(mIndex < MAX_MACHINES)
							{
								if(pSong->_pMachine[mIndex]) pSong->_pMachine[mIndex]->SetDestWireVolume(pSong,mIndex,pEntry->_inst, helpers::CValueMapper::Map_255_1(pEntry->_parameter));
							}
						}
						break;
					case  PatternCmd::SET_PANNING:
						mIndex = pEntry->_mach;
						if(mIndex < MAX_MACHINES)
						{
							if(pSong->_pMachine[mIndex]) pSong->_pMachine[mIndex]->SetPan(pEntry->_parameter>>1);
						}

						break;
					}
				}
				// Check For Tweak or MIDI CC
				else if(!pSong->_trackMuted[track])
				{
					int mac = pEntry->_mach;
					if((mac != 255) || (prevMachines[track] != 255))
					{
						if(mac != 255) prevMachines[track] = mac;
						else mac = prevMachines[track];
						if(mac < MAX_MACHINES)
						{
							Machine *pMachine = pSong->_pMachine[mac];
							if(pMachine)
							{
								if(pEntry->_note == notecommands::midicc && pMachine->_type != MACH_VST && pMachine->_type != MACH_VSTFX)
								{
									// for native machines,
									// use the value in the "instrument" field of the event as a voice number
									int voice(pEntry->_inst);
									// make a copy of the pattern entry, because we're going to modify it.
									PatternEntry entry(*pEntry);
									entry._note = 255;
									entry._inst = 255;
									// check for out of range voice values (with the classic tracker way, it's the same as the pattern tracks)
									if(voice < pSong->SONGTRACKS)
									{
										pMachine->Tick(voice, &entry);
									}
									else if(voice == 0xff)
									{
										// special voice value which means we want to send the same command to all voices
										for(int voice(0) ; voice < pSong->SONGTRACKS ; ++voice)
										{
											pMachine->Tick(voice, &entry);
										}
									}
									else ; // probably an out of range voice value (with the classic tracker way, it's limited to the number of pattern tracks)
									pMachine->TriggerDelayCounter[track] = 0;
									pMachine->ArpeggioCount[track] = 0;
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
			Song* pSong = Global::_pSong;
			for(int tc=0; tc<MAX_MACHINES; tc++)
			{
				if(pSong->_pMachine[tc])
				{
					pSong->_pMachine[tc]->Tick();
					for(int c = 0; c < MAX_TRACKS; c++) pSong->_pMachine[tc]->TriggerDelay[c]._cmd = 0;
				}
			}

		}

		/// Final Loop. Read new line for notes to send to the Machines
		void Player::ExecuteNotes(void)
		{
			Song* pSong = Global::_pSong;
			unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);


			for(int track=0; track<pSong->SONGTRACKS; track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
				if(( !pSong->_trackMuted[track]) && (pEntry->_note < notecommands::tweak || pEntry->_note == 255)) // Is it not muted and is a note?
				{
					int mac = pEntry->_mach;
					if(mac != 255) prevMachines[track] = mac;
					else mac = prevMachines[track];
					if( mac != 255 && (pEntry->_note != 255 || pEntry->_cmd != 0 || pEntry->_parameter != 0) ) // is there a machine number and it is either a note or a command?
//					if( mac != 255 ) // is there a machine number and it is either a note or a command?
					{
						if(mac < MAX_MACHINES) //looks like a valid machine index?
						{
							Machine *pMachine = pSong->_pMachine[mac];
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
		}	


		void Player::AdvancePosition()
		{
			Song* pSong = Global::_pSong;
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
			if(_lineCounter >= pSong->patternLines[_playPattern] || _lineCounter==_lineStop)
			{
				_lineCounter = 0;
				if(!_playBlock)
					_playPosition++;
				else
				{
					_playPosition++;
					while(_playPosition< pSong->playLength && (!pSong->playOrderSel[_playPosition]))
						_playPosition++;
				}
			}
			if( _playPosition >= pSong->playLength)
			{	
				// Don't loop the recording
				if(_recording)
				{
					StopRecording();
				}
				if( _loopSong )
				{
					_playPosition = 0;
					if(( _playBlock) && (pSong->playOrderSel[_playPosition] == false))
					{
						while((!pSong->playOrderSel[_playPosition]) && ( _playPosition< pSong->playLength)) _playPosition++;
					}
				}
				else 
				{
					_playing = false;
					_playBlock =false;
				}
			}
			// this is outside the if, so that _patternjump works
			_playPattern = pSong->playOrder[_playPosition];
			_lineChanged = true;
		}

		float * Player::Work(void* context, int numSamples)
		{
			int amount;
			Player* pThis = (Player*)context;
			Song* pSong = Global::_pSong;
			Master::_pMasterSamples = pThis->_pBuffer;
			CSingleLock crit(&Global::_pSong->door, TRUE);
			do
			{
				if(numSamples > STREAM_SIZE) amount = STREAM_SIZE; else amount = numSamples;
				// Tick handler function
				if(amount >= pThis->_samplesRemaining) amount = pThis->_samplesRemaining;
				//if((pThis->_playing) && (amount >= pThis->_samplesRemaining)) amount = pThis->_samplesRemaining;
				// Song play
				if((pThis->_samplesRemaining <=0))
				{
					if (pThis->_playing)
					{
						// Advance position in the sequencer
						pThis->AdvancePosition();
						// Global commands are executed first so that the values for BPM and alike
						// are up-to-date when "NotifyNewLine()" is called.
						pThis->ExecuteGlobalCommands();
						pThis->NotifyNewLine();
						pThis->ExecuteNotes();
					}
					else
					{
						pThis->NotifyNewLine();
					}
					pThis->_samplesRemaining = pThis->SamplesPerRow();
				}
				// Processing plant
				if(amount > 0)
				{
					if( (int)pSong->_sampCount > Global::pConfig->_pOutputDriver->_samplesPerSec)
					{
						pSong->_sampCount =0;
						for(int c=0; c<MAX_MACHINES; c++)
						{
							if(pSong->_pMachine[c])
							{
								pSong->_pMachine[c]->_wireCost = 0;
								pSong->_pMachine[c]->_cpuCost = 0;
							}
						}
					}
					// Reset all machines
					for(int c=0; c<MAX_MACHINES; c++)
					{
						if(pSong->_pMachine[c]) pSong->_pMachine[c]->PreWork(amount);
					}

					//\todo: Sampler::DoPreviews( amount );
					pSong->DoPreviews( amount );

					CVSTHost::vstTimeInfo.samplePos = ((Master *) (pSong->_pMachine[MASTER_INDEX]))->sampleCount;

#if !defined WINAMP_PLUGIN
					// Inject Midi input data
					if(!CMidiInput::Instance()->InjectMIDI( amount ))
					{
						// if midi not enabled we just do the original tracker thing
						// Master machine initiates work
						pSong->_pMachine[MASTER_INDEX]->Work(amount);
					}

					pSong->_sampCount += amount;
					if((pThis->_playing) && (pThis->_recording))
					{
						float* pL(pSong->_pMachine[MASTER_INDEX]->_pSamplesL);
						float* pR(pSong->_pMachine[MASTER_INDEX]->_pSamplesR);
						if(pThis->_dodither)
						{
							pThis->dither.Process(pL, amount);
							pThis->dither.Process(pR, amount);
						}
						int i;
						if ( pThis->_clipboardrecording)
						{
							switch(Global::pConfig->_pOutputDriver->_channelmode)
							{
							case 0: // mono mix
								for(i=0; i<amount; i++)
								{
									if (!pThis->ClipboardWriteMono(((*pL++)+(*pR++))/2)) pThis->StopRecording(false);
								}
								break;
							case 1: // mono L
								for(i=0; i<amount; i++)
								{
									if (!pThis->ClipboardWriteMono(*pL++)) pThis->StopRecording(false);
								}
								break;
							case 2: // mono R
								for(i=0; i<amount; i++)
								{
									if (!pThis->ClipboardWriteMono(*pR++)) pThis->StopRecording(false);
								}
								break;
							default: // stereo
								for(i=0; i<amount; i++)
								{
									if (!pThis->ClipboardWriteStereo(*pL++,*pR++)) pThis->StopRecording(false);
								}
								break;
							}						}
						else switch(Global::pConfig->_pOutputDriver->_channelmode)
						{
						case 0: // mono mix
							for(i=0; i<amount; i++)
							{
								//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
								if(pThis->_outputWaveFile.WriteMonoSample(((*pL++)+(*pR++))/2) != DDC_SUCCESS) pThis->StopRecording(false);
							}
							break;
						case 1: // mono L
							for(i=0; i<amount; i++)
							{
								if(pThis->_outputWaveFile.WriteMonoSample((*pL++)) != DDC_SUCCESS) pThis->StopRecording(false);
							}
							break;
						case 2: // mono R
							for(i=0; i<amount; i++)
							{
								if(pThis->_outputWaveFile.WriteMonoSample((*pR++)) != DDC_SUCCESS) pThis->StopRecording(false);
							}
							break;
						default: // stereo
							for(i=0; i<amount; i++)
							{
								if(pThis->_outputWaveFile.WriteStereoSample((*pL++),(*pR++)) != DDC_SUCCESS) pThis->StopRecording(false);
							}
							break;
						}
					}
#else
					pSong->_pMachine[MASTER_INDEX]->Work(amount);
					pSong->_sampCount += amount;
#endif //!defined WINAMP_PLUGIN

					Master::_pMasterSamples += amount * 2;
					numSamples -= amount;
				}
				 pThis->_samplesRemaining -= amount;
				 CVSTHost::vstTimeInfo.flags &= ~kVstTransportChanged;
			} while(numSamples>0);
			return pThis->_pBuffer;
		}
		bool Player::ClipboardWriteMono(float sample)
		{
			int *length = reinterpret_cast<int*>((*pClipboardmem)[0]);
			int pos = *length%1000000;
			int endpos = pos;
			
			switch( Global::pConfig->_pOutputDriver->_bitDepth)
			{
			case 8: endpos+=1; break;
			case 16: endpos+=2; break;
			case 24: endpos+=3; break;
			case 32: endpos+=4; break;
			}

			int d(0);
			if(sample > 32767.0f) sample = 32767.0f;
			else if(sample < -32768.0f) sample = -32768.0f;
			switch( Global::pConfig->_pOutputDriver->_bitDepth)
			{
			case 8:
				d = int(sample/256.0f);
				d += 128;
				(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
				*length+=1;
				break;
			case 16:
				d = static_cast<int>(sample);
				(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
				(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
				*length+=2;
				break;
			case 24:
				d = int(sample * 256.0f);
				if ( endpos < 1000000 )
				{
					(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
					(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
					(*pClipboardmem)[clipbufferindex][pos+2]=*(reinterpret_cast<char*>(&d)+2);
					*length+=3;
				}
				break;
			case 32:
				d = int(sample * 65536.0f);
				(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
				(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
				(*pClipboardmem)[clipbufferindex][pos+2]=*(reinterpret_cast<char*>(&d)+2);
				(*pClipboardmem)[clipbufferindex][pos+3]=*(reinterpret_cast<char*>(&d)+3);
				*length+=4;
				break;
			default:
				break;
			}

			if ( endpos >= 1000000 )
			{
				clipbufferindex++;
				char *newbuf = new char[1000000];
				if (!newbuf) return false;
				pClipboardmem->push_back(newbuf);
				// bitdepth == 24 is the only "odd" value, since it uses 3 chars each, nondivisible by 1000000
				if ( Global::pConfig->_pOutputDriver->_bitDepth == 24)
				{
					clipbufferindex--;
					(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
					if ( ++pos = 1000000) { pos = 0; clipbufferindex++; }
					(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
					if ( ++pos = 1000000) { pos = 0; clipbufferindex++; }
					(*pClipboardmem)[clipbufferindex][pos+2]=*(reinterpret_cast<char*>(&d)+2);
					if ( ++pos = 1000000) { pos = 0;  clipbufferindex++; }
					*length+=3;
				}
			}
			return true;
		}
		bool Player::ClipboardWriteStereo(float left, float right)
		{
			if (!ClipboardWriteMono(left)) return false;
			return ClipboardWriteMono(right);
		}

		void Player::StartRecording(std::string psFilename, int bitdepth, int samplerate, int channelmode, bool isFloat, bool dodither, int ditherpdf, int noiseshape, std::vector<char*> *clipboardmem)
		{
#if !defined WINAMP_PLUGIN
			if(!_recording)
			{
				//\todo: Upgrade all the playing functions to use m_SampleRate instead of pOutputdriver->samplesPerSec
				//       ensure correct re/initialization of variables (concretely, player::m_SampleRate and AudioDriver::_samplesPerSec)
				backup_rate = Global::pConfig->_pOutputDriver->_samplesPerSec;
				backup_bits = Global::pConfig->_pOutputDriver->_bitDepth;
				backup_channelmode = Global::pConfig->_pOutputDriver->_channelmode;
				if(samplerate > 0) { SampleRate(samplerate); Global::pConfig->_pOutputDriver->_samplesPerSec = samplerate; }
				if(bitdepth > 0) Global::pConfig->_pOutputDriver->_bitDepth = bitdepth;
				if(channelmode >= 0) Global::pConfig->_pOutputDriver->_channelmode = channelmode;
				if(_dodither=dodither)	//(not a typo)
				{
					if(bitdepth>0)	dither.SetBitDepth(bitdepth);
					else			dither.SetBitDepth(Global::pConfig->_pOutputDriver->_bitDepth);
					dither.SetPdf((helpers::dsp::Dither::Pdf)ditherpdf);
					dither.SetNoiseShaping((helpers::dsp::Dither::NoiseShape)noiseshape);
				}
				int channels = 2;
				if(Global::pConfig->_pOutputDriver->_channelmode != 3) channels = 1;
				Stop();
				if (!psFilename.empty())
				{
					if(_outputWaveFile.OpenForWrite(psFilename.c_str(), Global::pConfig->_pOutputDriver->_samplesPerSec, Global::pConfig->_pOutputDriver->_bitDepth, channels, isFloat) == DDC_SUCCESS)
						_recording = true;
					else
					{
						_recording = true;
						StopRecording(false);
					}
				}
				else
				{
					char *newbuf = new char[1000000];
					if ( newbuf)
					{
						pClipboardmem = clipboardmem;
						pClipboardmem->push_back(newbuf);
						_clipboardrecording = true;
						clipbufferindex = 1;
						_recording = true;
					}
					else {
						_recording = true;
						StopRecording(false);
					}
				}
			}
#endif //!defined WINAMP_PLUGIN
		}

		void Player::StopRecording(bool bOk)
		{
#if !defined WINAMP_PLUGIN
			if(_recording)
			{
				Global::pConfig->_pOutputDriver->_samplesPerSec = backup_rate;
				SampleRate(backup_rate);
				Global::pConfig->_pOutputDriver->_bitDepth = backup_bits;
				Global::pConfig->_pOutputDriver->_channelmode = backup_channelmode;
				if (!_clipboardrecording)
					_outputWaveFile.Close();
				_recording = false;
				_clipboardrecording =false;
				if(!bOk)
				{
					MessageBox(0, "Wav recording failed.", "ERROR", MB_OK);
				}
			}
#endif //!defined WINAMP_PLUGIN
		}
	}
}
