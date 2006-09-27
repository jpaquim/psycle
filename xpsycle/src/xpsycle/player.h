///\file
///\brief interface file for psycle::host::Player.
/// schedule the processing of machines, sends signal buffers and sequence events to them, ...

#pragma once
#include "song.h"
#include "dither.h"
#include "machine.h"
#include "riff.h"
#include "audiodriver.h"
#include "playertimeinfo.h"

namespace psycle
{
	namespace host
	{

		class Player
		{
		// Singleton Pattern
		private:
	  	Player();          
  		~Player();
			Player( Player const & );
  		Player& operator=(Player const&);

		public:
			static Player* Instance() {
					// note keep sure a player instance is created from the gui
					// before starting audiothread
					// or use single threaded only
					static Player s;
 					return &s; 
			}
		// Singleton pattern end

			void setDriver(  const AudioDriver & driver );
			AudioDriver & driver();

			Song inline & song() {
				return *song_;
			}

			void inline song(Song * song) {
				song_ = song;
			}

			// starts to play.
			void start( double pos = 0.0 );

			// stops playing.
			void stop();

			// is the player in playmode.
			bool playing() const;

			// set the actualPlaypos
			void setPlayPos( double pos );

			// the current playPos
			double playPos() const;

			// for wave render set the filename
			void setFileName( const std::string & fileName);
			
			// gets the wave to render filename
			const std::string fileName() const;

			void setAutoRecording( bool on );
			void startRecording( );
			void stopRecording( );
			bool recording() const;

			const PlayerTimeInfo & timeInfo() const;

			void setBpm( int bpm );
			float bpm() const;

			void SampleRate(const int sampleRate);

			bool autoStopMachines;

			// work function. (Entrance for the callback function (audiodriver)
			static float * Work(void* context, int& nsamples);
			
			// used by the plugins to indicate that they need redraw.
			bool Tweaker;

			void lock();
			void unlock();

			

		private:

			PlayerTimeInfo timeInfo_;

			Song * song_;
			std::string fileName_;
			AudioDriver* driver_;

			bool _playing;
			bool autoRecord_;
			bool recording_;

			// Final Loop. Read new line for notes to send to the Machines
			void ExecuteNotes( double beatOffset , PatternLine & line );
			void ProcessGlobalEvent(const GlobalEvent & event);
			void Process(int nsamples);

			float * Work(int nsamples);

			// Stores which machine played last in each track. this allows you to not specify the machine number everytime in the pattern.
			Machine::id_type prevMachines[MAX_TRACKS];
			// Stores the samplerate of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_rate;
			// Stores the bitdepth of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_bits;
			// Stores the channel mode (mono/stereo) of playback when recording to wave offline (non-realtime), since it can be changed.
			int backup_channelmode;
			// Temporary buffer to get all the audio from Master (which work in small chunks), and send it to the soundcard after converting it to float.
			float _pBuffer[MAX_DELAY_BUFFER];
			// file to which to output signal.
			WaveFile _outputWaveFile;
			// dither handler
			dsp::Dither dither;

			void writeSamplesToFile( int amount );
			bool _doDither;

			// here we need some real mutexes			
			bool lock_;
			bool inWork_;
			// Contains the number of samples until a line change comes in.
			int _samplesRemaining;
		
		};
	}
}
