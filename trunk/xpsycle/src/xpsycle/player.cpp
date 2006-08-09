///\file
///\brief implementation file for psycle::host::Player. based on psycle revision 2689
//#include <packageneric/pre-compiled.private.hpp>
//#include PACKAGENERIC
#include "player.h"
#include "song.h"
#include "machine.h"
#include "internal_machines.h"
#include "configuration.h"
#include "plugin_interface.h"
//#include "MidiInput.h"
#include "inputhandler.h"
namespace psycle
{
	namespace host
	{
		Player::Player()
		:
			song_(0),
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
			m_SamplesPerBeat((44100*60/125)),
			tpb(4),
			bpm(125),
			playPos(0.0)
		{
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i]=255;
		}

		Player::~Player() throw()
		{
			if(_recording) _outputWaveFile.Close();
		}

		void Player::Start(double pos)
		{
			Stop(); // This causes all machines to reset, and samplesperRow to init.
			((Master*)(song()._pMachine[MASTER_INDEX]))->_clip = false;
			((Master*)(song()._pMachine[MASTER_INDEX]))->sampleCount = 0;
			_lineChanged = true;
			_lineCounter = 0; //line;
			_SPRChanged = false;
			_playPosition= 0; //pos;
			_playTime = 0;
			_playTimem = 0;
			_loop_count =0;
			_loop_line = 0;
			SetBPM(song().bpm(), song().LinesPerBeat());
			SampleRate(Global::configuration()._pOutputDriver->_samplesPerSec);
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 255;
			_playing = true;
			playPos = pos;
			ExecuteLine();
		}

		void Player::Stop(void)
		{
			// Stop song enviroment
			_playing = false;
			_playBlock = false;
			for(int i=0; i<MAX_MACHINES; i++)
			{
				if(song()._pMachine[i])
				{
					song()._pMachine[i]->Stop();
					for(int c = 0; c < MAX_TRACKS; c++) song()._pMachine[i]->TriggerDelay[c]._cmd = 0;
				}
			}
			SetBPM(song().bpm(),song().LinesPerBeat());
			SampleRate(Global::configuration()._pOutputDriver->_samplesPerSec);
		}

		void Player::SampleRate(const int sampleRate)
		{
			///\todo update the source code of the plugins...
			if(m_SampleRate != sampleRate)
			{
				m_SampleRate = sampleRate;
				RecalcSPR();
				RecalcSPB();
				for(int i(0) ; i < MAX_MACHINES; ++i)
				{
					if(song()._pMachine[i]) song()._pMachine[i]->SetSampleRate(sampleRate);
				}
			}
		}
		void Player::SetBPM(float _bpm,int _tpb)
		{
			if ( _tpb != 0) tpb=_tpb;
			if ( _bpm != 0) bpm=_bpm;
			RecalcSPR();
			RecalcSPB();
			///\todo : Find out if we should notify the plugins of this change.
		}

		void Player::ExecuteLine(void)
		{
//			ExecuteGlobalCommands();
			NotifyNewLine();
//			ExecuteNotes();
		}
		// Initial Loop. Read new line and Interpret the Global commands.
/*		void Player::ExecuteGlobalCommands( std::list<PatternLine*> & tempPlayLines )
		{
			_patternjump = -1;
			_linejump = -1;
			int mIndex = 0;
			std::list<PatternLine*>::iterator lineItr = tempPlayLines.begin();
			for ( ; lineItr < tempPlayLines.end(); lineItr++);
			{
				PatternLine* line = *lineItr;
				std::map<int, PatternEvent>::iterator trackItr = line->begin();
				for ( ; trackItr != line->end() ; trackItr++) {
					PatternEvent dataEntry = trackItr->second;
					int trackNumber = trackItr->first;
			
				if(pEntry->_note < cdefTweakM || pEntry->_note == 255) // If This isn't a tweak (twk/tws/mcm) then do
				{
					switch(pEntry->_cmd)
					{
					case PatternCmd::SET_TEMPO:
						if(pEntry->_parameter != 0)
						{	///\todo: implement the Tempo slide
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
		}*/

		// Notify all machines that a new Tick() comes.
		void Player::NotifyNewLine( )
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
		void Player::ProcessGlobalEvent(const GlobalEvent & event)
		{
			Machine::id_type mIndex;
			switch (event.type())
			{
			case GlobalEvent::BPM_CHANGE:
				SetBPM(event.parameter() );
std::cout<<"bpm change event found. position: "<<playPos<<", new bpm: "<<event.parameter() <<std::endl;
				break;
			case GlobalEvent::JUMP_TO:
				this->SetPlayPos( event.parameter() );
				break;
			case GlobalEvent::SET_BYPASS:
				mIndex = event.target();
				if ( mIndex < MAX_MACHINES && song()._pMachine[mIndex] && song()._pMachine[mIndex]->_mode == MACHMODE_FX )
					song()._pMachine[mIndex]->_bypass = true;
				break;
			case GlobalEvent::UNSET_BYPASS:
				mIndex = event.target();
				if ( mIndex < MAX_MACHINES && song()._pMachine[mIndex] && song()._pMachine[mIndex]->_mode == MACHMODE_FX )
					song()._pMachine[mIndex]->_bypass = false;
				break;
			case GlobalEvent::SET_MUTE:
				mIndex = event.target();
				if ( mIndex < MAX_MACHINES && song()._pMachine[mIndex] )
					song()._pMachine[mIndex]->_mute = true;
				break;
			case GlobalEvent::UNSET_MUTE:
				mIndex = event.target();
				if ( mIndex < MAX_MACHINES && song()._pMachine[mIndex] )
					song()._pMachine[mIndex]->_mute = false;
				break;
			case GlobalEvent::SET_VOLUME:
				if(event.target() == 255)
				{
					((Master*)(song()._pMachine[MASTER_INDEX]))->_outDry = event.parameter();
				}
				else 
				{
					mIndex = event.target();
					if(mIndex < MAX_MACHINES)
					{
						Wire::id_type wire( event.target2() );
						if(song()._pMachine[mIndex]) song()._pMachine[mIndex]->SetDestWireVolume(mIndex,wire,CValueMapper::Map_255_1(event.parameter()));
					}
				}
			case GlobalEvent::SET_PANNING:
				mIndex = event.target();
				if(mIndex < MAX_MACHINES)
					if(song()._pMachine[mIndex]) song()._pMachine[mIndex]->SetPan( event.parameter() );
				break;

			default:
				break;
			}
		}

		/// Final Loop. Read new line for notes to send to the Machines
		void Player::ExecuteNotes(  double beatOffset , PatternLine & line )
		{
			PatternLine::iterator trackItr = line.begin();
			for ( ; trackItr != line.end() ; ++trackItr) {

				PatternEvent entry = trackItr->second;
				int track = trackItr->first;
				if(( !song()._trackMuted[track]) && (entry.note() < cdefTweakM || entry.note() == 255)) // Is it not muted and is a note?
				{
					int mac = entry.machine();
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
								if(entry.command() == PatternCmd::NOTE_DELAY)
								{
									// delay
									memcpy(&pMachine->TriggerDelay[track], entry.entry(), sizeof(PatternEntry));
									pMachine->TriggerDelayCounter[track] = ((entry.parameter()+1)*SamplesPerRow())/256;
								}
								else if(entry.command() == PatternCmd::RETRIGGER)
								{
									// retrigger
									memcpy(&pMachine->TriggerDelay[track], entry.entry(), sizeof(PatternEntry));
									pMachine->RetriggerRate[track] = (entry.parameter()+1);
									pMachine->TriggerDelayCounter[track] = 0;
								}
								else if(entry.command() == PatternCmd::RETR_CONT)
								{
									// retrigger continue
									memcpy(&pMachine->TriggerDelay[track], entry.entry(), sizeof(PatternEntry));
									if(entry.parameter()&0xf0) pMachine->RetriggerRate[track] = (entry.parameter()&0xf0);
								}
								else if (entry.command() == PatternCmd::ARPEGGIO)
								{
									// arpeggio
									///\todo : Add Memory.
									///\todo : This won't work... What about sampler's NNA's?
									if (entry.parameter())
									{
										memcpy(&pMachine->TriggerDelay[track], entry.entry(), sizeof(PatternEntry));
										pMachine->ArpeggioCount[track] = 1;
									}
									pMachine->RetriggerRate[track] = SamplesPerRow()*tpb/24;
								}
								else
								{
									pMachine->TriggerDelay[track]._cmd = 0;
									pMachine->AddEvent(beatOffset, track, entry);
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

		float * Player::Work(void* context, int & numSamples)
		{
			return reinterpret_cast<Player*>(context)->Work(numSamples);
		}

		float * Player::Work(int numSamples)
		{
			// Prepare the buffer that the Master Machine writes to.It is done here because Process() can be called several times.
			Master::_pMasterSamples = _pBuffer;
			double beatLength = numSamples/(double) SamplesPerBeat();
//			CSingleLock crit(&song().door, true);
			if (_playing)
			{
				std::multimap<double, PatternLine> events;
				std::vector<GlobalEvent*> globals;

				//processing of each buffer is subdivided into chunks, determined by the placement of any global events.
				double chunkBeatEnd;	//end beat position of the current chunk-- i.e., the next global event's position.
				double chunkBeatSize;	//number of beats in the chunk.
				int chunkSampleSize;	//number of samples needed to process for this chunk.
				int processedSamples(0);//this is used to counter rounding errors of sample/beat conversions
				bool bFirst(true);	//whether this is the first time through the loop.  this is passed to GetNextGlobalEvents()
						//to specify that we're including events at exactly playPos -only- on the first iteration--
						//otherwise we'll get the first event over and over again.
				do {
					//get the next round of global events.  we need to repopulate the list of globals and patternlines
					//each time through the loop because global events can potentially move the song's beatposition elsewhere.
					globals.clear();
					chunkBeatEnd = song().patternSequence()->GetNextGlobalEvents(playPos, beatLength, globals, bFirst);

					//determine chunk length in beats and samples.
					chunkBeatSize = chunkBeatEnd-playPos;	
					if(globals.empty())
						chunkSampleSize = numSamples - processedSamples;
					else
						chunkSampleSize = chunkBeatSize * SamplesPerBeat();

					//get all patternlines occuring before the next global event, execute them, and process
					events.clear();
					///\todo: Need to add the events coming from the MIDI device. (Of course, first we need the MIDI device)
					song().patternSequence()->GetLinesInRange(playPos, chunkBeatSize, events);
					for( std::multimap<double, PatternLine>::iterator lineIt=events.begin()
					   ; lineIt!= events.end()
					   ; ++lineIt)
						ExecuteNotes(lineIt->first - playPos, lineIt->second);

					if(chunkSampleSize>0)
					{
						Process(chunkSampleSize);
						processedSamples+=chunkSampleSize;
					}
					
					//increase playPos prior to executing globals, in case one of the globals needs it or wants to change it.
					playPos=chunkBeatEnd;
					beatLength-=chunkBeatSize;

					//execute this batch of global events
					for( std::vector<GlobalEvent*>::iterator globIt = globals.begin()
					   ; globIt!=globals.end()
					   ; ++globIt)
					{
						ProcessGlobalEvent(*(*globIt));
					}

					bFirst=false;
				} while(!globals.empty());  //if globals is empty, then we've processed through to the end of the buffer.
			}
			else
			{
				///\todo: Need to add the events coming from the MIDI device. (Of course, first we need the MIDI device)
				playPos=beatLength;
//				if (playPos> "signumerator") playPos-=signumerator;
				Process(numSamples);
			}
			return _pBuffer;
		}

		void Player::Process(int numSamples)
		{
			int remainingsamles = numSamples;
			while (remainingsamples)
			{
				int amount = std::min(remainingsamples,STREAM_SIZE);
//				if( (int)song()._sampCount > Global::configuration()._pOutputDriver->_samplesPerSec)
//				{
//					song()._sampCount =0;
//					for(int c=0; c<MAX_MACHINES; c++)
//					{
//						if(song()._pMachine[c])
//						{
//							song()._pMachine[c]->wire_cpu_cost(0);
//							song()._pMachine[c]->work_cpu_cost(0);
//						}
//					}
//				}
				// Reset all machine buffers
				for(int c=0; c<MAX_MACHINES; c++)
				{
					if(song()._pMachine[c]) song()._pMachine[c]->PreWork(amount);
				}

				song().DoPreviews( amount );
				song()._pMachine[MASTER_INDEX]->Work(amount);

				//Move the pointer forward for the next Master::Work() iteration.
				Master::_pMasterSamples += amount * 2;
				remainingsamples -= amount;
			}
		}
  }
}




