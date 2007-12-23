///\file
///\brief implementation file for psy::core::Player

/***************************************************************************
*   Copyright (C) 2007 Psycledelics, Josep Maria Antolin Segura, D.W. Aley
*   psycle.sf.net   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include <psycle/core/psycleCorePch.hpp>

#include "player.h"

#include "internal_machines.h"
#include "machine.h"
#include "sampler.h"
#include "song.h"
#include <psycle/audiodrivers/audiodriver.h>

#include <iostream> // only for debug output

#if defined __unix__ || defined __APPLE__
	#include <unistd.h> // for OpenBSD usleep()
#endif

namespace psy
{
	namespace core
	{

		Player::Player()
		:
			song_(),
			driver_(),
			_playing(),
			Tweaker(),
			loopSequenceEntry_(),
			_doDither(),
			autoRecord_(),
			recording_(),
			autoStopMachines_(),
			lock_(),
			inWork_()
		{
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i]=255;
		}

		Player::~Player()
		{
			if ( driver_ ) delete driver_; ///\todo [bohan] i don't see why the player owns the driver.
		}

		void Player::start( double pos )
		{
			if(!song_)
			{
				std::cerr << "psycle: core: player: no song to play\n";
				return;
			}
			if(!driver_)
			{
				std::cerr << "psycle: core: player: no audio driver to output the song to\n";
				return;
			}
			stop(); // This causes all machines to reset, and samplesperRow to init.
			if (autoRecord_) startRecording();

			((Master*)(song().machine(MASTER_INDEX)))->_clip = false;
			((Master*)(song().machine(MASTER_INDEX)))->sampleCount = 0;
			
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 255;
			_playing = true;
			timeInfo_.setPlayBeatPos( pos );
			timeInfo_.setTicksSpeed( song().ticksSpeed(), song().isTicks() );
		}

		void Player::stop( )
		{
			if ( !song_) return;
			if ( !driver_ ) return;

			// Stop song enviroment
			_playing = false;
			for(int i=0; i<MAX_MACHINES; i++)
			{
				if(song().machine(i))
				{
					song().machine(i)->Stop();
					for(int c = 0; c < MAX_TRACKS; c++) song().machine(i)->TriggerDelay[c].setCommand( 0 );
				}
			}
			setBpm( song().bpm() );
			timeInfo_.setTicksSpeed( song().ticksSpeed(), song().isTicks() );
			SampleRate( driver_->settings().samplesPerSec() );
			if (autoRecord_) stopRecording();
			//printf("stop\n");
		}

		void Player::SampleRate(const int sampleRate)
		{
			if ( !song_ ) return;

			///\todo update the source code of the plugins...

			timeInfo_.setSampleRate( sampleRate );

			for(int i(0) ; i < MAX_MACHINES; ++i)
			{
				if(song().machine(i)) song().machine(i)->SetSampleRate( sampleRate );
			}
		}
		
		void Player::ProcessGlobalEvent(const GlobalEvent & event)
		{
			Machine::id_type mIndex;
			switch ( event.type() )
			{
				case GlobalEvent::BPM_CHANGE:
					setBpm ( event.parameter() );
					std::cout << "psycle: player: bpm change event found. position: " << timeInfo_.playBeatPos() << ", new bpm: " << event.parameter() << "\n";
					break;
				case GlobalEvent::JUMP_TO:
					timeInfo_.setPlayBeatPos( event.parameter() );
					break;
				case GlobalEvent::SET_BYPASS:
					mIndex = event.target();
					if ( mIndex < MAX_MACHINES && song().machine(mIndex) && song().machine(mIndex)->mode() == MACHMODE_FX )
						song().machine(mIndex)->_bypass = true;
					break;
				case GlobalEvent::UNSET_BYPASS:
					mIndex = event.target();
					if ( mIndex < MAX_MACHINES && song().machine(mIndex) && song().machine(mIndex)->mode() == MACHMODE_FX )
						song().machine(mIndex)->_bypass = false;
					break;
				case GlobalEvent::SET_MUTE:
					mIndex = event.target();
					if ( mIndex < MAX_MACHINES && song().machine(mIndex) )
						song().machine(mIndex)->_mute = true;
					break;
				case GlobalEvent::UNSET_MUTE:
					mIndex = event.target();
					if ( mIndex < MAX_MACHINES && song().machine(mIndex) )
						song().machine(mIndex)->_mute = false;
					break;
				case GlobalEvent::SET_VOLUME:
					if(event.target() == 255)
					{
						((Master*)(song().machine(MASTER_INDEX)))->_outDry = static_cast<int>( event.parameter() );
					}
					else 
					{
						mIndex = event.target();
						if(mIndex < MAX_MACHINES)
						{
							Wire::id_type wire( event.target2() );
							if(song().machine(mIndex)) song().machine(mIndex)->SetDestWireVolume(mIndex,wire,CValueMapper::Map_255_1( static_cast<int>( event.parameter() )));
						}
					}
				case GlobalEvent::SET_PANNING:
					mIndex = event.target();
					if(mIndex < MAX_MACHINES)
						if(song().machine(mIndex)) song().machine(mIndex)->SetPan( static_cast<int>( event.parameter() ) );
					break;
				default:
					break;
				}
		}

		/// Final Loop. Read new line for notes to send to the Machines
		void Player::ExecuteNotes(  double beatOffset , PatternLine & line )
		{
			/// WARNING!!! In this function, the events inside the patterline are assumed to be temporary! (thus, modificable)

			/// Step 1: Process all tweaks.
			std::map<int, PatternEvent>::iterator trackItr = line.tweaks().begin();
			for ( ; trackItr != line.tweaks().end() ; ++trackItr) {
				PatternEvent & entry(trackItr->second);
				int track = trackItr->first;
				int mac = entry.machine();
				if(mac < MAX_MACHINES) //looks like a valid machine index?
				{
					if ( song().machine(mac) )
					{
						Machine &machine = *song().machine(mac);
						if ( entry.note() == commands::tweak_slide )
						{
							const int delay(64);
							int delaysamples(0), origin(machine.GetParamValue(entry.instrument()));
							float rate(0.0f), increment(origin);
							int previous(0);
							
							rate = (((entry.command()<<16) | entry.parameter()) - origin) / (timeInfo().samplesPerTick()/64.0f);

							entry.setNote(commands::tweak);
							entry.setCommand(origin>>8);
							entry.setParameter(origin&0xFF);
							machine.AddEvent(beatOffset+ ((double)delaysamples)/timeInfo().samplesPerBeat(), line.sequenceTrack()*1024+track, entry);
							previous= origin;
							delaysamples+=delay;
							while (delaysamples < timeInfo().samplesPerTick())
							{
								increment+=rate;
								if (static_cast<int>(increment) != previous)
								{
									origin= static_cast<int>(increment);
									entry.setCommand(origin>>8);
									entry.setParameter(origin&0xFF);
									machine.AddEvent(beatOffset+ ((double)delaysamples)/timeInfo().samplesPerBeat(), line.sequenceTrack()*1024+track, entry);
									previous = origin;
								}
								delaysamples+=delay;
							}
						}
						else
						{
							machine.AddEvent(beatOffset, line.sequenceTrack()*1024+track, entry);
						}
					}
				}
			}

			/// Step 2: Process all notes.
			trackItr = line.notes().begin();
			for ( ; trackItr != line.notes().end() ; ++trackItr) {
				PatternEvent entry = trackItr->second;
				int track = trackItr->first;
				if(( !song().patternSequence()->trackMuted(track)) && (entry.note() < psy::core::commands::tweak || entry.note() == 255)) // Is it not muted and is a note?
				{
					int mac = entry.machine();
					if(mac != 255) prevMachines[track] = mac;
					else mac = prevMachines[track];
					//if( mac != 255 && (pEntry->_note != 255 || pEntry->_cmd != 0x00) ) // is there a machine number and it is either a note or a command?
					if( mac != 255 ) // is there a machine number and it is either a note or a command?
					{
						if(mac < MAX_MACHINES) //looks like a valid machine index?
						{
							if(song().machine(mac) && !(song().machine(mac)->_mute)) // Does this machine really exist and is not muted?
							{
								Machine &machine = *song().machine(mac);
								if(entry.command() == PatternCmd::NOTE_DELAY)
								{
									double delayoffset(entry.parameter()/256.0);
									// At least Plucked String works erroneously if the command is not ommited.
									entry.setCommand(0); entry.setParameter(0);
									machine.AddEvent(beatOffset+delayoffset, line.sequenceTrack()*1024+track, entry);
								}
								else if(entry.command() == PatternCmd::RETRIGGER)
								{
									//\todo: delaysamples and rate should be memorized (for RETR_CONT command ). Then set delaysamples to zero in this function.
									int delaysamples(0), rate(0), delay(0);
									rate = entry.parameter()+1;
									delay = (rate*static_cast<int>(timeInfo().samplesPerTick())) >> 8; // x/256
									entry.setCommand(0); entry.setParameter(0);
									machine.AddEvent(beatOffset, line.sequenceTrack()*1024+track, entry);

									delaysamples+=delay;
									while (delaysamples < timeInfo().samplesPerTick())
									{
										machine.AddEvent(beatOffset+ ((double)delaysamples)/timeInfo().samplesPerBeat(), line.sequenceTrack()*1024+track, entry);
										delaysamples+=delay;
									}
								}
								else if(entry.command() == PatternCmd::RETR_CONT)
								{
									///\todo: delaysamples and rate should be memorized, do not reinit delaysamples.
									///\todo: verify that using ints for rate and variation is enough, or has to be float.
									int delaysamples(0), rate(0), delay(0), variation(0);
									int parameter = entry.parameter()&0x0f;
									variation = (parameter < 9) ? (4*parameter) : (-1*(2*(16-parameter)));

									if(entry.parameter()&0xf0) rate = (entry.parameter()&0xf0);
									delay = (rate*static_cast<int>(timeInfo().samplesPerTick())) >> 8; // x/256
									entry.setCommand(0); entry.setParameter(0);
									machine.AddEvent(beatOffset+ ((double)delaysamples)/timeInfo().samplesPerBeat(), line.sequenceTrack()*1024+track, entry);
									delaysamples+=delay;
									while (delaysamples < timeInfo().samplesPerTick())
									{
										machine.AddEvent(beatOffset+ ((double)delaysamples)/timeInfo().samplesPerBeat(), line.sequenceTrack()*1024+track, entry);

										rate+=variation;
										if (rate < 16) rate = 16;
										delay = (rate*static_cast<int>(timeInfo().samplesPerTick())) >> 8; // x/256
										delaysamples+=delay;
									}
								}
/*
								else if (entry.command() == PatternCmd::ARPEGGIO)
								{
									// arpeggio
									///\todo : Add Memory.
									///\todo : This won't work... What about sampler's NNA's?
									if (entry.parameter())
									{
										machine.TriggerDelay[track] = entry;
										machine.ArpeggioCount[track] = 1;
									}
									machine.RetriggerRate[track] = static_cast<int>( timeInfo_.samplesPerTick()* timeInfo_.linesPerBeat() / 24 );
								}
*/
								else
								{
									machine.TriggerDelay[track].setCommand( 0 );
									machine.AddEvent(beatOffset, line.sequenceTrack()*1024+track, entry);
									machine.TriggerDelayCounter[track] = 0;
									machine.ArpeggioCount[track] = 0;
								}
							}
						}
					}
				}
			}
		}

		float * Player::Work(int numSamples)
		{
			if ( !song_ ) return _pBuffer;
			if ( lock_ ) return _pBuffer;

			inWork_ = true;

			// Prepare the buffer that the Master Machine writes to.It is done here because Process() can be called several times.
			Master::_pMasterSamples = _pBuffer;
			double beatsToWork = numSamples/(double) timeInfo_.samplesPerBeat();
			//CSingleLock crit(&song().door, true);

			if (autoRecord_ && timeInfo_.playBeatPos() >= song().patternSequence()->tickLength()) {
				stopRecording();
			}

			if (_playing)
			{
				if ( loopSequenceEntry() ) {
					if ( timeInfo_.playBeatPos() >= loopSequenceEntry()->tickEndPosition()
							|| timeInfo_.playBeatPos() <= loopSequenceEntry()->tickPosition() ) {
						setPlayPos( loopSequenceEntry()->tickPosition() );
					}
				} else if ( loopSong() && timeInfo_.playBeatPos() >= song().patternSequence()->tickLength()) {
					setPlayPos( 0.0 );
				}
				std::multimap<double, PatternLine> events;
				std::vector<GlobalEvent*> globals;

				//processing of each buffer is subdivided into chunks, determined by the placement of any global events.
				
				/// end beat position of the current chunk-- i.e., the next global event's position.
				double chunkBeatEnd;
				/// number of beats in the chunk.
				double chunkBeatSize;
				/// number of samples needed to process for this chunk.
				int chunkSampleSize;
				/// this is used to counter rounding errors of sample/beat conversions
				int processedSamples(0);
				/// whether this is the first time through the loop.  this is passed to GetNextGlobalEvents()
				/// to specify that we're including events at exactly playPos -only- on the first iteration--
				/// otherwise we'll get the first event over and over again.
				bool bFirst(true);
				do {
					//get the next round of global events.  we need to repopulate the list of globals and patternlines
					//each time through the loop because global events can potentially move the song's beatposition elsewhere.
					globals.clear();
					chunkBeatEnd = song().patternSequence()->GetNextGlobalEvents(timeInfo_.playBeatPos(), beatsToWork, globals, bFirst);

					//determine chunk length in beats and samples.
					chunkBeatSize = chunkBeatEnd - timeInfo_.playBeatPos();
					if(globals.empty())
						chunkSampleSize = numSamples - processedSamples;
					else
						chunkSampleSize = static_cast<int>( chunkBeatSize * timeInfo_.samplesPerBeat() );

					//get all patternlines occuring before the next global event, execute them, and process
					events.clear();
					///\todo: Need to add the events coming from the MIDI device. (Of course, first we need the MIDI device)
					song().patternSequence()->GetLinesInRange(timeInfo_.playBeatPos(), chunkBeatSize, events);
					for( std::multimap<double, PatternLine>::iterator lineIt=events.begin()
						; lineIt!= events.end()
						; ++lineIt) {
						ExecuteNotes(lineIt->first - timeInfo_.playBeatPos(), lineIt->second);
					}

					if(chunkSampleSize>0)
					{
						Process(chunkSampleSize);
						processedSamples+=chunkSampleSize;
					}
					
					beatsToWork-=chunkBeatSize;

					//execute this batch of global events
					for( std::vector<GlobalEvent*>::iterator globIt = globals.begin()
						; globIt!=globals.end()
						; ++globIt)
					{
						ProcessGlobalEvent(*(*globIt));
					}

					bFirst=false;
				} while(!globals.empty()); // if globals is empty, then we've processed through to the end of the buffer.
			}
			else
			{
				///\todo: Need to add the events coming from the MIDI device. (Of course, first we need the MIDI device)
				Process(numSamples);
				//playPos+=beatLength;
				//if (playPos> "signumerator") playPos-=signumerator;
			}
			
			//printf("\rBeat: %.02f",timeInfo().playBeatPos());
			//fflush(stdout);
			
			inWork_ = false;
			return _pBuffer;
		}

		void Player::Process(int numSamples)
		{
			int remainingsamples = numSamples;
			while (remainingsamples)
			{
				int amount = std::min(remainingsamples,STREAM_SIZE);
				// Reset all machine buffers
				for(int c=0; c<MAX_MACHINES; c++)
				{
					if(song().machine(c)) song().machine(c)->PreWork(amount);
				}

				Sampler::DoPreviews( amount ,song().machine(MASTER_INDEX)->_pSamplesL, song().machine(MASTER_INDEX)->_pSamplesR);
				song().machine(MASTER_INDEX)->Work(amount );

				if ( (recording_ && !autoRecord_) || // controlled by record button
						(recording_ && _playing && autoRecord_) ) { // controlled by play
					// write sample to file
					writeSamplesToFile( amount);
				}

				//Move the pointer forward for the next Master::Work() iteration.
				Master::_pMasterSamples += amount * 2;
				remainingsamples -= amount;
				//increase playPos
				timeInfo_.setPlayBeatPos( timeInfo_.playBeatPos()+ (amount/ timeInfo_.samplesPerBeat()) );
			}
		}

		// buffer to wav_file methods

		void Player::writeSamplesToFile( int amount ) {

			if (!song_ || !driver_) return;

			float* pL(song().machine(MASTER_INDEX)->_pSamplesL);
			float* pR(song().machine(MASTER_INDEX)->_pSamplesR);
			if(_doDither) {
				dither.Process(pL, amount);
				dither.Process(pR, amount);
			}
			int i;
			switch( driver_->settings().channelMode() )
			{
				case 0: // mono mix
					for( i = 0; i < amount; i++)
					{
						//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
						if(_outputWaveFile.WriteMonoSample(((*pL++)+(*pR++))/2) != DDC_SUCCESS) stopRecording();
					}
					break;
				case 1: // mono L
					for( i = 0; i < amount; i++ )
					{
						if(_outputWaveFile.WriteMonoSample((*pL++)) != DDC_SUCCESS) stopRecording();
					}
					break;
				case 2: // mono R
					for( i = 0; i < amount; i++)
					{
						if(_outputWaveFile.WriteMonoSample((*pR++)) != DDC_SUCCESS) stopRecording();
					}
					break;
				default: // stereo
					for( i = 0; i < amount; i++)
					{
						if(_outputWaveFile.WriteStereoSample((*pL++),(*pR++)) != DDC_SUCCESS) stopRecording();
					}
					break;
			}
		}

		void Player::startRecording( ) {

			if ( !song_ && !driver_ ) return;

			int channels = 2;
			if ( driver_->settings().channelMode() != 3 ) channels = 1;

			if(_outputWaveFile.OpenForWrite(fileName().c_str(), driver_->settings().samplesPerSec(), driver_->settings().bitDepth(), channels ) == DDC_SUCCESS)
				recording_ = true;
			else
				recording_ = false;
		}

		void Player::stopRecording( ) {
			if (recording_) {
				_outputWaveFile.Close();
				recording_ = false;
			}
		}

		void Player::setDriver(  const AudioDriver & driver ) {
			std::cout << "entering player::setaudio" << std::endl;
			if ( driver_) {
				driver_->Enable( false );
				delete driver_;
			}
			///\todo: This is a dangerous thing. It's scheduled to be changed
			driver_ = driver.clone();
			std::cout << "cloned driver " << std::endl;
			if (!driver_->Initialized())
			{
				driver_->Initialize( Work, this );
			}
			std::cout << " driver initialized" << std::endl;
			if (!driver_->Configured())
			{
				std::cout << "asking driver to configure itself" << std::endl;
				driver_->Configure();
				//SampleRate(driver_->_samplesPerSec);
				//_outputActive = true;
			}
			std::cout << " driver configured" << std::endl;
			if (driver_->Enable(true))
			{
				std::cout << "driver enabled " << driver_->info().name() << std::endl;
				//_outputActive = true;
			} else {
				std::cout << "driver failed to enable. setting null driver " << std::endl;
				if (driver_) delete driver_;
				driver_ = new AudioDriver();
			}
			SampleRate(driver_->settings().samplesPerSec());
		}

		void psy::core::Player::lock( )
		{
			///\todo this is bad
			lock_ = true;
			#if defined __unix__ || defined __APPLE__
				while ( inWork_) usleep( 200 );
			#endif
		}

		void psy::core::Player::unlock( )
		{
			///\todo this is bad
			lock_ = false;
		}
	}
}
