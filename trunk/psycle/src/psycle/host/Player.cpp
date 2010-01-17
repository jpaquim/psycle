///\file
///\brief implementation file for psycle::host::Player.
#include "configuration_options.hpp"

#if !PSYCLE__CONFIGURATION__USE_PSYCORE // popped at eof

#include "Global.hpp"
#include "Player.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "Sampler.hpp"
#include "Configuration.hpp"

#if !defined WINAMP_PLUGIN
	#include "MidiInput.hpp"
	#include "InputHandler.hpp"
#endif //!defined WINAMP_PLUGIN

#include <seib-vsthost/CVSTHost.Seib.hpp> // Included to interact directly with the host.
#include <psycle/helpers/value_mapper.hpp>

namespace psycle { namespace host {

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
			CSingleLock crit(&Global::song().door, TRUE);
			if (initialize)
			{
				stop(); // This causes all machines to reset, and samplesperRow to init.				
				Work(this,psycle::plugin_interface::MAX_BUFFER_LENGTH);
				((Master*)(Global::song().machine(MASTER_INDEX)))->_clip = false;
				((Master*)(Global::song().machine(MASTER_INDEX)))->sampleCount = 0;
			}
			_lineChanged = true;
			_lineCounter = line;
			_SPRChanged = false;
			_sequencePosition= pos;
			_playPattern = Global::song().playOrder[_sequencePosition];
			if (initialize)
			{
				_playTime = 0;
				_playTimem = 0;
			}
			_loop_count =0;
			_loop_line = 0;
			if (initialize)
			{
				SetBPM(Global::song().BeatsPerMin(),Global::song().LinesPerBeat());
				SampleRate(Global::pConfig->_pOutputDriver->_samplesPerSec);
				for(int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 255;
				_playing = true;
			}
			CVSTHost::vstTimeInfo.flags |= kVstTransportPlaying;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
			ExecuteLine();
			_samplesRemaining = SamplesPerRow();
		}

		void Player::stop(void)
		{
			CSingleLock crit(&Global::song().door, TRUE);

			if (_playing == true)
				_lineStop = -1;

			// Stop song enviroment
			_playing = false;
			_playBlock = false;			
			for(int i=0; i<MAX_MACHINES; i++)
			{
				if(Global::song().machine(i))
				{
					Global::song().machine(i)->Stop();
					for(int c = 0; c < MAX_TRACKS; c++) Global::song().machine(i)->TriggerDelay[c]._cmd = 0;
				}
			}
			SetBPM(Global::song().BeatsPerMin(),Global::song().LinesPerBeat());
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
					if(Global::song().machine(i)) Global::song().machine(i)->SetSampleRate(sampleRate);
				}
			}
		}
		void Player::SetBPM(int _bpm,int _tpb)
		{
			if ( _tpb != 0) tpb_=_tpb;
			if ( _bpm != 0) bpm_=_bpm;
			RecalcSPR();
			CVSTHost::vstTimeInfo.tempo = bpm_;
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
			Song* pSong = &Global::song();
			_patternjump = -1;
			_linejump = -1;
			int mIndex = 0;
			unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);

			for(int track=0; track<pSong->tracks(); track++)
			{
				PatternEvent* pEntry = (PatternEvent*)(plineOffset + track*EVENT_SIZE);
				if(pEntry->note() < notecommands::tweak || pEntry->note() == 255) // If This isn't a tweak (twk/tws/mcm) then do
				{
					switch(pEntry->command())
					{
					case PatternCmd::SET_TEMPO:
						if(pEntry->parameter() != 0)
						{	///\todo: implement the Tempo slide
							// SET_SONG_TEMPO=			20, // T0x Slide tempo down . T1x slide tempo up
							SetBPM(pEntry->parameter());
						}
						break;
					case PatternCmd::EXTENDED:
						if(pEntry->parameter() != 0)
						{
							if ( (pEntry->parameter()&0xE0) == 0 ) // range from 0 to 1F for LinesPerBeat.
							{
								SetBPM(0,pEntry->parameter());
							}
							else if ( (pEntry->parameter()&0xF0) == PatternCmd::SET_BYPASS )
							{
								mIndex = pEntry->machine();
								if ( mIndex < MAX_MACHINES && pSong->machine(mIndex) && pSong->machine(mIndex)->_mode == MACHMODE_FX )
								{
									if ( pEntry->parameter()&0x0F )
										pSong->machine(mIndex)->Bypass(true);
									else
										pSong->machine(mIndex)->Bypass(false);
								}
							}

							else if ( (pEntry->parameter()&0xF0) == PatternCmd::SET_MUTE )
							{
								mIndex = pEntry->machine();
								if ( mIndex < MAX_MACHINES && pSong->machine(mIndex) && pSong->machine(mIndex)->_mode != MACHMODE_MASTER )
								{
									if ( pEntry->parameter()&0x0F )
										pSong->machine(mIndex)->_mute = true;
									else
										pSong->machine(mIndex)->_mute = false;
								}
							}
							else if ( (pEntry->parameter()&0xF0) == PatternCmd::PATTERN_DELAY )
							{
								SamplesPerRow(SamplesPerRow()*(1+(pEntry->parameter()&0x0F)));
								_SPRChanged=true;
							}
							else if ( (pEntry->parameter()&0xF0) == PatternCmd::FINE_PATTERN_DELAY)
							{
								SamplesPerRow(SamplesPerRow()*(1.0f+((pEntry->parameter()&0x0F)*tpb()/24.0f)));
								_SPRChanged=true;
							}
							else if ( (pEntry->parameter()&0xF0) == PatternCmd::PATTERN_LOOP)
							{
								int value = pEntry->parameter()&0x0F;
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
						if ( pEntry->parameter() < pSong->playLength ){
							_patternjump=pEntry->parameter();
							_linejump=0;
						}
						break;
					case PatternCmd::BREAK_TO_LINE:
						if (_patternjump ==-1) 
						{
							_patternjump=(_sequencePosition+1>=pSong->playLength)?0:_sequencePosition+1;
						}
						if ( pEntry->parameter() >= pSong->patternLines[_patternjump])
						{
							_linejump = pSong->patternLines[_patternjump];
						} else { _linejump= pEntry->parameter(); }
						break;
					case PatternCmd::SET_VOLUME:
						if(pEntry->machine() == 255)
						{
							((Master*)(pSong->machine(MASTER_INDEX)))->_outDry = pEntry->parameter();
						}
						else 
						{
							int mIndex = pEntry->machine();
							if(mIndex < MAX_MACHINES)
							{
								if(pSong->machine(mIndex)) pSong->machine(mIndex)->SetDestWireVolume(pSong,mIndex,pEntry->instrument(), value_mapper::map_255_1(pEntry->parameter()));
							}
						}
						break;
					case  PatternCmd::SET_PANNING:
						mIndex = pEntry->machine();
						if(mIndex < MAX_MACHINES)
						{
							if(pSong->machine(mIndex)) pSong->machine(mIndex)->SetPan(pEntry->parameter()>>1);
						}

						break;
					}
				}
				// Check For Tweak or MIDI CC
				else if(!pSong->_trackMuted[track])
				{
					int mac = pEntry->machine();
					if((mac != 255) || (prevMachines[track] != 255))
					{
						if(mac != 255) prevMachines[track] = mac;
						else mac = prevMachines[track];
						if(mac < MAX_MACHINES)
						{
							Machine *pMachine = pSong->machine(mac);
							if(pMachine)
							{
								if(pEntry->note() == notecommands::midicc && pMachine->_type != MACH_VST && pMachine->_type != MACH_VSTFX)
								{
									// for native machines,
									// use the value in the "instrument" field of the event as a voice number
									int voice(pEntry->instrument());
									// make a copy of the pattern entry, because we're going to modify it.
									PatternEvent entry(*pEntry);
									entry._note = 255;
									entry._inst = 255;
									// check for out of range voice values (with the classic tracker way, it's the same as the pattern tracks)
									if(voice < pSong->tracks())
									{
										pMachine->Tick(voice, &entry);
									}
									else if(voice == 0xff)
									{
										// special voice value which means we want to send the same command to all voices
										for(int voice(0) ; voice < pSong->tracks() ; ++voice)
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
			Song* pSong = &Global::song();
			for(int tc=0; tc<MAX_MACHINES; tc++)
			{
				if(pSong->machine(tc))
				{
					pSong->machine(tc)->Tick();
					for(int c = 0; c < MAX_TRACKS; c++) pSong->machine(tc)->TriggerDelay[c]._cmd = 0;
				}
			}

		}

		/// Final Loop. Read new line for notes to send to the Machines
		void Player::ExecuteNotes(void)
		{
			Song* pSong = &Global::song();
			unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);


			for(int track=0; track<pSong->tracks(); track++)
			{
				PatternEvent* pEntry = (PatternEvent*)(plineOffset + track*EVENT_SIZE);
				if(( !pSong->_trackMuted[track]) && (pEntry->note() < notecommands::tweak || pEntry->note() == 255)) // Is it not muted and is a note?
				{
					int mac = pEntry->machine();
					if(mac != 255) prevMachines[track] = mac;
					else mac = prevMachines[track];
					if( mac != 255 && (pEntry->note() != 255 || pEntry->command() != 0 || pEntry->parameter() != 0) ) // is there a machine number and it is either a note or a command?
//					if( mac != 255 ) // is there a machine number and it is either a note or a command?
					{
						if(mac < MAX_MACHINES) //looks like a valid machine index?
						{
							Machine *pMachine = pSong->machine(mac);
							if(pMachine && !(pMachine->_mute)) // Does this machine really exist and is not muted?
							{
								if(pEntry->command() == PatternCmd::NOTE_DELAY)
								{
									// delay
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEvent));
									pMachine->TriggerDelayCounter[track] = ((pEntry->parameter()+1)*SamplesPerRow())/256;
								}
								else if(pEntry->command() == PatternCmd::RETRIGGER)
								{
									// retrigger
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEvent));
									pMachine->RetriggerRate[track] = (pEntry->parameter()+1);
									pMachine->TriggerDelayCounter[track] = 0;
								}
								else if(pEntry->command() == PatternCmd::RETR_CONT)
								{
									// retrigger continue
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEvent));
									if(pEntry->parameter()&0xf0) pMachine->RetriggerRate[track] = (pEntry->parameter()&0xf0);
								}
								else if (pEntry->command() == PatternCmd::ARPEGGIO)
								{
									// arpeggio
									//\todo : Add Memory.
									//\todo : This won't work... What about sampler's NNA's?
									if (pEntry->parameter())
									{
										memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEvent));
										pMachine->ArpeggioCount[track] = 1;
									}
									pMachine->RetriggerRate[track] = SamplesPerRow()*tpb()/24;
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
			Song* pSong = &Global::song();
			if ( _patternjump!=-1 ) _sequencePosition= _patternjump;
			if ( _SPRChanged ) { RecalcSPR(); _SPRChanged = true; }
			if ( _linejump!=-1 ) _lineCounter=_linejump;
			else _lineCounter++;
			_playTime += 60 / float (bpm_ * tpb_);
			if(_playTime>60)
			{
				_playTime-=60;
				_playTimem++;
			}
			if(_lineCounter >= pSong->patternLines[_playPattern] || _lineCounter==_lineStop)
			{
				_lineCounter = 0;
				if(!_playBlock)
					_sequencePosition++;
				else
				{
					_sequencePosition++;
					while(_sequencePosition< pSong->playLength && (!pSong->playOrderSel[_sequencePosition]))
						_sequencePosition++;
				}
			}
			if( _sequencePosition >= pSong->playLength)
			{	
				// Don't loop the recording
				if(_recording)
				{
					stopRecording();
				}
				if( _loopSong )
				{
					_sequencePosition = 0;
					if(( _playBlock) && (pSong->playOrderSel[_sequencePosition] == false))
					{
						while((!pSong->playOrderSel[_sequencePosition]) && ( _sequencePosition< pSong->playLength)) _sequencePosition++;
					}
				}
				else 
				{
					_playing = false;
					_playBlock =false;
				}
			}
			// this is outside the if, so that _patternjump works
			_playPattern = pSong->playOrder[_sequencePosition];
			_lineChanged = true;
		}

		float * Player::Work(void* context, int numSamples)
		{
			int amount;
			Player* pThis = (Player*)context;
			Song* pSong = &Global::song();
			Master::_pMasterSamples = pThis->_pBuffer;
			CSingleLock crit(&Global::song().door, TRUE);
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
							if(pSong->machine(c))
							{
								pSong->machine(c)->_wireCost = 0;
								pSong->machine(c)->_cpuCost = 0;
							}
						}
					}
					// Reset all machines
					for(int c=0; c<MAX_MACHINES; c++)
					{
						if(pSong->machine(c)) pSong->machine(c)->PreWork(amount);
					}

					Sampler::DoPreviews( pSong->machine(MASTER_INDEX)->_pSamplesL, pSong->machine(MASTER_INDEX)->_pSamplesR, amount );

					CVSTHost::vstTimeInfo.samplePos = ((Master *) (pSong->machine(MASTER_INDEX)))->sampleCount;

					#if !defined WINAMP_PLUGIN
					// Inject Midi input data
					if(!CMidiInput::Instance()->InjectMIDI( amount ))
					{
						// if midi not enabled we just do the original tracker thing
						// Master machine initiates work
						pSong->machine(MASTER_INDEX)->Work(amount);
					}

					pSong->_sampCount += amount;
					if((pThis->_playing) && (pThis->_recording))
					{
						float* pL(pSong->machine(MASTER_INDEX)->_pSamplesL);
						float* pR(pSong->machine(MASTER_INDEX)->_pSamplesR);
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
									if (!pThis->ClipboardWriteMono(((*pL++)+(*pR++))/2)) pThis->stopRecording(false);
								}
								break;
							case 1: // mono L
								for(i=0; i<amount; i++)
								{
									if (!pThis->ClipboardWriteMono(*pL++)) pThis->stopRecording(false);
								}
								break;
							case 2: // mono R
								for(i=0; i<amount; i++)
								{
									if (!pThis->ClipboardWriteMono(*pR++)) pThis->stopRecording(false);
								}
								break;
							default: // stereo
								for(i=0; i<amount; i++)
								{
									if (!pThis->ClipboardWriteStereo(*pL++,*pR++)) pThis->stopRecording(false);
								}
								break;
							}						}
						else switch(Global::pConfig->_pOutputDriver->_channelmode)
						{
						case 0: // mono mix
							for(i=0; i<amount; i++)
							{
								//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
								if(pThis->_outputWaveFile.WriteMonoSample(((*pL++)+(*pR++))/2) != DDC_SUCCESS) pThis->stopRecording(false);
							}
							break;
						case 1: // mono L
							for(i=0; i<amount; i++)
							{
								if(pThis->_outputWaveFile.WriteMonoSample((*pL++)) != DDC_SUCCESS) pThis->stopRecording(false);
							}
							break;
						case 2: // mono R
							for(i=0; i<amount; i++)
							{
								if(pThis->_outputWaveFile.WriteMonoSample((*pR++)) != DDC_SUCCESS) pThis->stopRecording(false);
							}
							break;
						default: // stereo
							for(i=0; i<amount; i++)
							{
								if(pThis->_outputWaveFile.WriteStereoSample((*pL++),(*pR++)) != DDC_SUCCESS) pThis->stopRecording(false);
							}
							break;
						}
					}
					#else
					pSong->machine(MASTER_INDEX)->Work(amount);
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
					dither.SetPdf(Dither::Pdf::type(ditherpdf));
					dither.SetNoiseShaping(Dither::NoiseShape::type(noiseshape));
				}
				int channels = 2;
				if(Global::pConfig->_pOutputDriver->_channelmode != 3) channels = 1;
				stop();
				if (!psFilename.empty())
				{
					if(_outputWaveFile.OpenForWrite(psFilename.c_str(), Global::pConfig->_pOutputDriver->_samplesPerSec, Global::pConfig->_pOutputDriver->_bitDepth, channels, isFloat) == DDC_SUCCESS)
						_recording = true;
					else
					{
						_recording = true;
						stopRecording(false);
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
						stopRecording(false);
					}
				}
			}
			#endif //!defined WINAMP_PLUGIN
		}

		void Player::stopRecording(bool bOk)
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
#endif //#if !PSYCLE__CONFIGURATION__USE_PSYCORE
