///\file
///\brief implementation file for psycle::host::Player.
#include <project.private.hpp>
#include "player.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "Configuration.hpp"
#include "MidiInput.hpp"
#include "InputHandler.hpp"
namespace psycle
{
	namespace host
	{
		Player::Player()
		{
			_playing = false;
			_playBlock = false;
			_recording = false;
			Tweaker = false;
			_ticksRemaining=0;
			_lineCounter=0;
			_loopSong=true;
			_patternjump=-1;
			_linejump=-1;
			m_SampleRate=44100;
			m_SamplesPerRow=(44100*60)/(125*4);
			tpb=4;
			bpm=125;
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i]=255;
		}

		Player::~Player()
		{
			if(_recording) _outputWaveFile.Close();
		}

		void Player::Start(int pos, int line)
		{
			Stop(); // This causes all machines to reset, and samplesperRow to init.
			((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->_clip = false;
			((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->sampleCount = 0;
			_lineChanged = true;
			_lineCounter = line;
			_playPosition= pos;
			_playPattern = Global::_pSong->playOrder[_playPosition];
			_playTime = 0;
			_playTimem = 0;
			bpm=Global::_pSong->BeatsPerMin();
			tpb=Global::_pSong->LinesPerBeat();
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
				if(Global::_pSong->_pMachine[i])
				{
					Global::_pSong->_pMachine[i]->Stop();
					for(int c = 0; c < MAX_TRACKS; c++) Global::_pSong->_pMachine[i]->TriggerDelay[c]._cmd = 0;
				}
			}
			SamplesPerRow((Global::pConfig->_pOutputDriver->_samplesPerSec*60)/(Global::_pSong->BeatsPerMin()*Global::_pSong->LinesPerBeat()));
		}

		void Player::SampleRate(const int sampleRate){
			SamplesPerRow((sampleRate*60)/(bpm*tpb));
			///\todo update the source code of the plugins...
			if(m_SampleRate != sampleRate)
			{
				for(int i(0) ; i < MAX_MACHINES; ++i)
				{
					if(Global::_pSong->_pMachine[i]) Global::_pSong->_pMachine[i]->SetSampleRate(sampleRate);
				}
			}
			m_SampleRate = sampleRate;
		}
		void Player::SetBPM(int _bpm,int _tpb)
		{
			tpb=_tpb;
			bpm=_bpm;
			RecalcSPR();
			//\todo : Find out if we should notify the plugins of this change.
		}

		void Player::ExecuteLine(void)
		{
			Song* pSong = Global::_pSong;
			_lineChanged = true;
			_patternjump = -1;
			_linejump = -1;
			unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);

			// Initial Loop. Check for Tracker Commands.
			for(int track=0; track<pSong->SONGTRACKS; track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
				if(pEntry->_note < cdefTweakM || pEntry->_note == 255) // If This isn't a tweak (twk/tws/mcm) then do
				{
					switch(pEntry->_cmd)
					{
					case CMD::SET_TEMPO:
						if(pEntry->_parameter != 0)
						{	//\todo: implement the Tempo slide
							// SET_SONG_TEMPO=			20, // T0x Slide tempo down . T1x slide tempo up
							bpm = pEntry->_parameter;
							RecalcSPR();
						}
						break;
					case CMD::EXTENDED:
						if(pEntry->_parameter != 0)
						{
							if ( (pEntry->_parameter&0xF0) == CMD::PATTERN_DELAY )
							{
							//\todo: finish the implementation of these two commands.
							}
							else if ( (pEntry->_parameter&0xF0) == CMD::FINE_PATTERN_DELAY)
							{
							}
							else if ( (pEntry->_parameter&0xF0) == CMD::PATTERN_LOOP)
							{
							}
							else if ( (pEntry->_parameter&0xE0) == 0 ) // range from 0 to 1F for LinesPerBeat.
							{
								tpb=pEntry->_parameter;
								RecalcSPR();
							}
						}
						break;
					case CMD::JUMP_TO_ORDER:
						if ( pEntry->_parameter < pSong->playLength ){
							_patternjump=pEntry->_parameter;
							_linejump=0;
						}
						break;
					case CMD::BREAK_TO_LINE:
						if (_patternjump ==-1) 
						{
							_patternjump=(_playPosition+1>=pSong->playLength)?0:_playPosition+1;
						}
						if ( pEntry->_parameter >= pSong->patternLines[_patternjump])
						{
							_linejump = pSong->patternLines[_patternjump];
						} else { _linejump= pEntry->_parameter; }
					case CMD::SET_VOLUME:
						if(pEntry->_mach == 255)
						{
							((Master*)(pSong->_pMachine[MASTER_INDEX]))->_outDry = pEntry->_parameter;
						}
						else 
						{
							int mIndex = pEntry->_mach;
							if(mIndex < MAX_MACHINES)
							{
								if(pSong->_pMachine[mIndex]) pSong->_pMachine[mIndex]->SetDestWireVolume(mIndex,pEntry->_inst,CValueMapper::Map_255_1(pEntry->_parameter));
							}
						}
						break;
					case  CMD::SET_PANNING:
						int mIndex = pEntry->_mach;
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
							if(pMachine) pMachine->Tick(track, pEntry);
						}
					}
				}
			}

			// Notify all machines that a new Tick() comes.
			for(int tc=0; tc<MAX_MACHINES; tc++)
			{
				if(pSong->_pMachine[tc])
				{
					pSong->_pMachine[tc]->Tick();
					for(int c = 0; c < MAX_TRACKS; c++) pSong->_pMachine[tc]->TriggerDelay[c]._cmd = 0;
				}
			}

			// Finally, loop again checking for Notes
			for(int track=0; track<pSong->SONGTRACKS; track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*5);
				if(( !pSong->_trackMuted[track]) && (pEntry->_note < cdefTweakM || pEntry->_note == 255)) // Is it not muted and is a note?
				{
					int mac = pEntry->_mach;
					if(mac != 255) prevMachines[track] = mac;
					else mac = prevMachines[track];
					if( mac != 255 && (pEntry->_note != 255 || pEntry->_cmd != 0x00) ) // is there a machine number and it is either a note or a command?
					{
						if(mac < MAX_MACHINES) //looks like a valid machine index?
						{
							Machine *pMachine = pSong->_pMachine[mac];
							if(pMachine && !(pMachine->_mute)) // Does this machine really exist and is not muted?
							{
								if(pEntry->_cmd == CMD::NOTE_DELAY)
								{
									// delay
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									pMachine->TriggerDelayCounter[track] = ((pEntry->_parameter+1)*SamplesPerRow())/256;
								}
								else if(pEntry->_cmd == CMD::RETRIGGER)
								{
									// retrigger
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									pMachine->RetriggerRate[track] = (pEntry->_parameter+1);
									pMachine->TriggerDelayCounter[track] = 0;
								}
								else if(pEntry->_cmd == CMD::RETR_CONT)
								{
									// retrigger continue
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									if(pEntry->_parameter&0xf0) pMachine->RetriggerRate[track] = (pEntry->_parameter&0xf0);
								}
								else if (pEntry->_cmd == CMD::ARPEGGIO)
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
			_ticksRemaining = SamplesPerRow();
		}	


		void Player::AdvancePosition()
		{
			Song* pSong = Global::_pSong;
			if ( _patternjump!=-1 ) _playPosition= _patternjump;
			if ( _linejump!=-1 ) _lineCounter=_linejump;
			else _lineCounter++;
			_playTime += 60 / float (bpm * tpb);
			if(_playTime>60)
			{
				_playTime-=60;
				_playTimem++;
			}
			if(_lineCounter >= pSong->patternLines[_playPattern])
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
		}

		float * Player::Work(void* context, int & numSamples)
		{
			int amount;
			Player* pThis = (Player*)context;
			Song* pSong = Global::_pSong;
			Master::_pMasterSamples = pThis->_pBuffer;
			int numSamplex = numSamples;
			do
			{
				if(numSamplex > STREAM_SIZE) amount = STREAM_SIZE; else amount = numSamplex;
				// Tick handler function
				if((pThis->_playing) && (amount >= pThis->_ticksRemaining)) amount = pThis->_ticksRemaining;
				// Song play
				if((pThis->_ticksRemaining <=0) && (pThis->_playing))
				{
					// Advance position in the sequencer
					pThis->AdvancePosition();
					if (pThis->_playing) pThis->ExecuteLine();
				}
				// Processing plant
				if(amount > 0)
				{
					CSingleLock crit(&Global::_pSong->door, TRUE);
					CPUCOST_INIT(idletime);
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
					if(pSong->PW_Stage == 1)
					{
						// Mixing preview WAV
						pSong->PW_Work(pSong->_pMachine[MASTER_INDEX]->_pSamplesL,pSong->_pMachine[MASTER_INDEX]->_pSamplesR, amount);
					}
					// Inject Midi input data
					if(!CMidiInput::Instance()->InjectMIDI( amount ))
					{
						// if midi not enabled we just do the original tracker thing
						// Master machine initiates work
						pSong->_pMachine[MASTER_INDEX]->Work(amount);
					}
					CPUCOST_CALC(idletime, amount);
					pSong->cpuIdle = idletime;
					pSong->_sampCount += amount;
					if((pThis->_playing) && (pThis->_recording))
					{
						//float* pData(pThis->_pBuffer); ///\todo <- this was fuxxxxing up
						float* pL(pSong->_pMachine[MASTER_INDEX]->_pSamplesL);
						float* pR(pSong->_pMachine[MASTER_INDEX]->_pSamplesR);
						int i;
						switch(Global::pConfig->_pOutputDriver->_channelmode)
						{
						case 0: // mono mix
							for(i=0; i<amount; i++)
							{
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
					Master::_pMasterSamples += amount * 2;
					numSamplex -= amount;
				}
				if(pThis->_playing) pThis->_ticksRemaining -= amount;
			} while(numSamplex>0); ///\todo this is strange
			return pThis->_pBuffer;
		}

		void Player::StartRecording(std::string psFilename, int bitdepth, int samplerate, int channelmode)
		{
			if(!_recording)
			{
				//\todo: Upgrade all the playing functions to use m_SampleRate instead of pOutputdriver->samplesPerSec
				backup_rate = Global::pConfig->_pOutputDriver->_samplesPerSec;
				backup_bits = Global::pConfig->_pOutputDriver->_bitDepth;
				backup_channelmode = Global::pConfig->_pOutputDriver->_channelmode;
				if(samplerate > 0) Global::pConfig->_pOutputDriver->_samplesPerSec = samplerate;
				if(bitdepth > 0) Global::pConfig->_pOutputDriver->_bitDepth = bitdepth;
				if(channelmode >= 0) Global::pConfig->_pOutputDriver->_channelmode = channelmode;
				int channels = 2;
				if(Global::pConfig->_pOutputDriver->_channelmode != 3) channels = 1;
				SamplesPerRow((Global::pConfig->_pOutputDriver->_samplesPerSec*60)/(Global::pPlayer->bpm*Global::pPlayer->tpb));
				Stop();
				if(_outputWaveFile.OpenForWrite(psFilename.c_str(), Global::pConfig->_pOutputDriver->_samplesPerSec, Global::pConfig->_pOutputDriver->_bitDepth, channels) == DDC_SUCCESS)
					_recording = true;
				else
				{
					StopRecording();
					::MessageBox(0, psFilename.c_str(), "FAILED", 0);
				}
			}
		}

		void Player::StopRecording(bool bOk)
		{
			if(_recording)
			{
				Global::pConfig->_pOutputDriver->_samplesPerSec = backup_rate;
				Global::pConfig->_pOutputDriver->_bitDepth = backup_bits;
				Global::pConfig->_pOutputDriver->_channelmode = backup_channelmode;
				SamplesPerRow((Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb));
				_outputWaveFile.Close();
				_recording = false;
				if(!bOk)
				{
					::MessageBox(0, "Wav recording failed.", "ERROR", MB_OK);
				}
			}
		}
	}
}
