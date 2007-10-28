///\file
///\brief interface file for psy::core::Player.

/***************************************************************************
*   Copyright (C) 2007 Psycledelics, Josep Maria Antolín Segura, D.W. Aley
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

#pragma once

#include <psycle/audiodrivers/audiodriver.h> ///\todo doesn't belong in psycore

#include "song.h"
#include "dither.h"
#include "machine.h"
#include "riff.h"

namespace psy
{
	namespace core
	{
		/// schedules the processing of machines, sends signal buffers and sequence events to them, ...
		class Player : public MachineCallbacks
		{
			public:
				Player();
				~Player();

				static Player* Instance() {
					// note: keep sure a player instance is created from the gui
					// before starting audiothread
					// or use single threaded only
					static Player s;
					return &s; 
				}

			public:
				/// used by the plugins to indicate that they need redraw.
				///\todo private access
				bool Tweaker;

			///\name audio driver
			///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
			///\{
				public:
					///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
					void setDriver(  const AudioDriver & driver );
					///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
					AudioDriver & driver() { return *driver_; }
					///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
					AudioDriver const & driver() const { return *driver_; }
				private:
					///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
					AudioDriver* driver_;
			///\}

			public:
				void SampleRate(const int sampleRate);

			///\name recording
			///\todo missplaced?
			///\{
				public:
					void setAutoRecording( bool on ) { autoRecord_ = on; }
				private:
					bool autoRecord_;
			///\}

			///\name callback
			///\{
				public:
					/// entrance for the callback function (audiodriver)
					static float * Work(void* context, int & samples) { return reinterpret_cast<Player*>(context)->Work(samples); }
				private:
					/// entrance for the callback function (audiodriver)
					float * Work(int samples);
			///\}
				
			///\name song
			///\{
				public:
					Song const & song() const { return *song_; }
					Song & song() { return *song_; }
					void song(Song * song) { song_ = song; }
				private:
					Song * song_;
			///\}

			///\name secondary output device, write to a file
			///\todo maybe this shouldn't be in player either.
			///\{
				public:
					/// starts the recording output device.
					void startRecording( );
					/// stops the recording output device.
					void stopRecording( );
					/// wether the recording device has been started.
					bool recording() const { return recording_; }
					/// for wave render set the filename
					void setFileName( const std::string & fileName) { fileName_ = fileName; }
					/// gets the wave to render filename
					const std::string fileName() const { return fileName_; }
				private:
					/// wether the recording device has been started.
					bool recording_;
					/// whether to apply dither to recording
					bool _doDither;
					/// wave render filename
					std::string fileName_;
					/// file to which to output signal.
					WaveFile _outputWaveFile;
					void writeSamplesToFile( int amount );
			///\}

			///\name time info
			///\{
				public:
					 PlayerTimeInfo & timeInfo() { return timeInfo_; }
				private:
					PlayerTimeInfo timeInfo_;
			///\}

			///\name time info ... play position
			///\{
				public:
					/// sets the current play position
					void setPlayPos( double pos ) { timeInfo_.setPlayBeatPos( pos ); }
					/// the current play position
					double playPos() const { return timeInfo_.playBeatPos(); }
			///\}
			
			///\name time info ... bpm
			///\{
				public:
					void setBpm( double bpm ) { timeInfo_.setBpm( bpm ); }
					double bpm() const { return timeInfo_.bpm(); }
			///\}
			
			///\name start/stop
			///\{
				public:
					/// starts to play.
					void start( double pos = 0.0 );
					/// stops playing.
					void stop();
					/// is the player in playmode.
					bool playing() const { return _playing; }
				private:
					bool _playing;
			///\}
			
			///\name loop
			///\{
				public:
					void setLoopSong( bool setit ) { loopSong_ = setit; }
					bool loopSong() const { return loopSong_; }

					void setLoopSequenceEntry( SequenceEntry *seqEntry ) { loopSequenceEntry_ = seqEntry; }
					SequenceEntry *loopSequenceEntry() const { return loopSequenceEntry_; }
				private:
					bool loopSong_;
					SequenceEntry *loopSequenceEntry_;
			///\}

			///\name auto stop
			///\{
				public:
					bool autoStopMachines() const { return autoStopMachines_; }
				private:
					bool autoStopMachines_;
			///\}
			
			///\name multithreading locking
			///\{
				public:
					void lock();
					void unlock();
				private:
					///\todo here we need some real mutexes
					bool lock_;
					///\todo here we need some real mutexes
					bool inWork_;
			///\}

			private:
				/// Final Loop. Read new line for notes to send to the Machines
				void ExecuteNotes( double beatOffset , PatternLine & line );
				void ProcessGlobalEvent(const GlobalEvent & event);
				void Process(int nsamples);

				/// Stores which machine played last in each track. this allows you to not specify the machine number everytime in the pattern.
				Machine::id_type prevMachines[MAX_TRACKS];
				/// Stores the samplerate of playback when recording to wave offline (non-realtime), since it can be changed.
				int backup_rate;
				/// Stores the bitdepth of playback when recording to wave offline (non-realtime), since it can be changed.
				int backup_bits;
				/// Stores the channel mode (mono/stereo) of playback when recording to wave offline (non-realtime), since it can be changed.
				int backup_channelmode;
				/// Temporary buffer to get all the audio from Master (which work in small chunks), and send it to the soundcard after converting it to float.
				float _pBuffer[MAX_DELAY_BUFFER];

				/// dither handler
				dsp::Dither dither;

				/// Contains the number of samples until a line change comes in.
				int _samplesRemaining;
		};
	}
}
