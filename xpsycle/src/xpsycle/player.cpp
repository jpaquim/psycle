///\file
///\brief implementation file for psycle::host::Player. based on psycle revision 2689
//#include <packageneric/pre-compiled.private.hpp>
//#include PACKAGENERIC
#include "player.h"
#include "song.h"
#include "machine.h"
#include "internal_machines.h"
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
			bpm(125)
		{
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i]=255;
			_doDither = false;
			autoRecord_ = false;
			driver_ = 0;
		}

		Player::~Player()
		{
			if ( driver_ ) delete driver_;
		}

		const PlayerTimeInfo & Player::timeInfo( ) const
		{
			return timeInfo_;
		}

		void Player::Start(double pos)
		{
      if ( !song_ && !driver_ ) return;

			Stop(); // This causes all machines to reset, and samplesperRow to init.
			if (autoRecord_) startRecording();

			((Master*)(song()._pMachine[MASTER_INDEX]))->_clip = false;
			((Master*)(song()._pMachine[MASTER_INDEX]))->sampleCount = 0;
			_lineChanged = true;
			_lineCounter = 0; //line;
			_SPRChanged = false;
			_playTime = 0;
			_playTimem = 0;
			_loop_count =0;
			_loop_line = 0;
			SetBPM(song().bpm(), song().LinesPerBeat());
			SampleRate( 44100 );
			for(int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 255;
			_playing = true;
			timeInfo_.setPlayBeatPos( pos );
		}

		void Player::Stop(void)
		{
      if ( !song_  && driver_ ) return;

			// Stop song enviroment
			_playing = false;
			_playBlock = false;
			for(int i=0; i<MAX_MACHINES; i++)
			{
				if(song()._pMachine[i])
				{
					song()._pMachine[i]->Stop();
					for(int c = 0; c < MAX_TRACKS; c++) song()._pMachine[i]->TriggerDelay[c].setCommand( 0 );
				}
			}
			SetBPM(song().bpm(),song().LinesPerBeat());
			SampleRate(driver_->_samplesPerSec);
			if (autoRecord_) stopRecording();
		}

		void Player::SampleRate(const int sampleRate)
		{
       if ( !song_ ) return;

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
			timeInfo_.setLinesPerBeat( _tpb );
			if ( _bpm != 0) bpm=_bpm;
			RecalcSPR();
			RecalcSPB();
			///\todo : Find out if we should notify the plugins of this change.
		}

		void Player::ProcessGlobalEvent(const GlobalEvent & event)
		{
			Machine::id_type mIndex;
			switch (event.type())
			{
			case GlobalEvent::BPM_CHANGE:
				SetBPM(event.parameter() );
std::cout<<"bpm change event found. position: "<<timeInfo_.playBeatPos()<<", new bpm: "<<event.parameter() <<std::endl;
				break;
			case GlobalEvent::JUMP_TO:
				timeInfo_.setPlayBeatPos( event.parameter() );
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
				if(( !song()._trackMuted[track]) && (entry.note() < notecommands::tweak || entry.note() == 255)) // Is it not muted and is a note?
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
									pMachine->TriggerDelay[track] = entry;
									pMachine->TriggerDelayCounter[track] = ((entry.parameter()+1)*SamplesPerRow())/256;
								}
								else if(entry.command() == PatternCmd::RETRIGGER)
								{
									// retrigger
									pMachine->TriggerDelay[track] = entry;
									pMachine->RetriggerRate[track] = (entry.parameter()+1);
									pMachine->TriggerDelayCounter[track] = 0;
								}
								else if(entry.command() == PatternCmd::RETR_CONT)
								{
									// retrigger continue
									pMachine->TriggerDelay[track] = entry;
									if(entry.parameter()&0xf0) pMachine->RetriggerRate[track] = (entry.parameter()&0xf0);
								}
								else if (entry.command() == PatternCmd::ARPEGGIO)
								{
									// arpeggio
									///\todo : Add Memory.
									///\todo : This won't work... What about sampler's NNA's?
									if (entry.parameter())
									{
										pMachine->TriggerDelay[track] = entry;
										pMachine->ArpeggioCount[track] = 1;
									}
									pMachine->RetriggerRate[track] = SamplesPerRow()* timeInfo_.linesPerBeat() / 24;
								}
								else
								{
									pMachine->TriggerDelay[track].setCommand( 0 );
									pMachine->AddEvent(beatOffset, line.sequenceTrack()*1024+track, entry);
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
			if ( !song_ ) return _pBuffer;

			// Prepare the buffer that the Master Machine writes to.It is done here because Process() can be called several times.
			Master::_pMasterSamples = _pBuffer;
			double beatLength = numSamples/(double) SamplesPerBeat();
//			CSingleLock crit(&song().door, true);

			if (autoRecord_ && timeInfo_.playBeatPos() >= song().patternSequence()->tickLength()) {
				stopRecording();
			}

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
					chunkBeatEnd = song().patternSequence()->GetNextGlobalEvents(timeInfo_.playBeatPos(), beatLength, globals, bFirst);

					//determine chunk length in beats and samples.
					chunkBeatSize = chunkBeatEnd - timeInfo_.playBeatPos();
					if(globals.empty())
						chunkSampleSize = numSamples - processedSamples;
					else
						chunkSampleSize = chunkBeatSize * SamplesPerBeat();

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
					
					//increase playPos prior to executing globals, in case one of the globals needs it or wants to change it.
					timeInfo_.setPlayBeatPos( chunkBeatEnd );
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
				Process(numSamples);
//				playPos+=beatLength;
//				if (playPos> "signumerator") playPos-=signumerator;
			}
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
					if(song()._pMachine[c]) song()._pMachine[c]->PreWork(amount);
				}

				song().DoPreviews( amount );
				song()._pMachine[MASTER_INDEX]->Work(amount, timeInfo_ );

				if ( (recording_ && !autoRecord_) || // controlled by record button
						(recording_ && _playing && autoRecord_) ) { // controlled by play
					// write sample to file
					writeSamplesToFile( amount);
				}

				//Move the pointer forward for the next Master::Work() iteration.
				Master::_pMasterSamples += amount * 2;
				remainingsamples -= amount;
			}
		}


		// buffer to wav_file methods

		void Player::setFileName( const std::string & fileName) {
			fileName_ = fileName;
		}

		const std::string Player::fileName() const {
			return fileName_;
		}

		void Player::writeSamplesToFile( int amount ) {

			if (!song_ && !driver_) return;

			float* pL(song()._pMachine[MASTER_INDEX]->_pSamplesL);
			float* pR(song()._pMachine[MASTER_INDEX]->_pSamplesR);
			if(_doDither) {
				dither.Process(pL, amount);
				dither.Process(pR, amount);
			}
			int i;
			switch( driver_->_channelmode )
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

		void Player::setAutoRecording( bool on ) {
			autoRecord_ = true;
		}

		void Player::startRecording( ) {

			if ( !song_ && !driver_ ) return;

			int channels = 2;
			if ( driver_->_channelmode != 3 ) channels = 1;

			if(_outputWaveFile.OpenForWrite(fileName().c_str(), driver_->_samplesPerSec, driver_->_bitDepth, channels ) == DDC_SUCCESS)
				recording_ = true;
			else
				recording_ = false;
		}

		void Player::stopRecording( ) {
			if (recording_) {
				_outputWaveFile.Close();
				recordStopped.emit();
			}
			recording_ = false;
		}

		void Player::setDriver(  const AudioDriver & driver ) {
			if ( !driver_) delete driver_;
			driver_ = driver.clone();
			if (!driver_->Initialized())
  		{
				driver_->Initialize( Work, this );
			}
  		if (!driver_->Configured())
  		{
      	driver_->Configure();
				SampleRate(driver_->_samplesPerSec);
  			//   _outputActive = true;
  		}
			if (driver_->Enable(true))
			{
			//   _outputActive = true;
			} else {
				if (driver_) delete driver_;
				driver_ = new AudioDriver();
			}
		}

	} // end of host namespace
}



 // end of psycle namespace
