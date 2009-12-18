// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\interface psycle::core::Player.

#ifndef PSYCLE__CORE__PLAYER__INCLUDED
#define PSYCLE__CORE__PLAYER__INCLUDED
#pragma once

#include "song.h"
#include "machine.h"
#include "sequencer.h"

#include <psycle/helpers/dither.hpp>
#include <psycle/helpers/riff.hpp>

#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>
#include <universalis/stdlib/date_time.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <list>
#include <stdexcept>

namespace psycle { namespace core {

class AudioDriver; ///\todo doesn't belong in psycore
using namespace psycle::helpers;

/// schedules the processing of machines, sends signal buffers and sequence events to them, ...
class PSYCLE__CORE__DECL Player : public MachineCallbacks, private boost::noncopyable {
	private:
		Player();
		~Player();

	public:
		Player static & singleton();

	public:
		/// used by the plugins to indicate that they need redraw.
		///\todo private access
		bool Tweaker;

	///\name audio driver
	///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
	///\{
		public:
			///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
			AudioDriver & driver() { return *driver_; }
			///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
			AudioDriver const & driver() const { return *driver_; }
			///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
			void setDriver(AudioDriver & driver);
		private:
			///\todo player should not need to know about the audio driver, it should export a callback to generate audio instead.
			AudioDriver * driver_;
	///\}

	///\name sample rate
	///\{
		public:
			/// samples per second
			void samples_per_second(int samples_per_second);
	///\}

	///\name recording
	///\todo missplaced?
	///\{
		public:
			void setAutoRecording(bool on) { autoRecord_ = on; }
		private:
			bool autoRecord_;
	///\}

	///\name callback
	///\{
		public:
			/// entrance for the callback function (audiodriver)
			static float * Work(void * context, int samples) { return reinterpret_cast<Player*>(context)->Work(samples); }
		private:
			/// entrance for the callback function (audiodriver)
			float * Work(int samples);
	///\}

	///\name song
	///\{
		public:
			CoreSong const & song() const { return *song_; }
			CoreSong & song() { return *song_; }
			void song(CoreSong & song) { song_ = &song; sequencer_.set_song(&song); }
		private:
			CoreSong * song_;
	///\}

	///\name secondary output device, write to a file
	///\todo maybe this shouldn't be in player either.
	///\{
		public:
			/// starts the recording output device.
			void startRecording(bool dodither=false , 
				psycle::helpers::dsp::Dither::Pdf::type ditherpdf=psycle::helpers::dsp::Dither::Pdf::triangular,
				psycle::helpers::dsp::Dither::NoiseShape::type noiseshaping=psycle::helpers::dsp::Dither::NoiseShape::none);
			/// stops the recording output device.
			void stopRecording( );
			/// wether the recording device has been started.
			bool recording() const { return recording_; }
			/// for wave render set the filename
			void setFileName(std::string const & fileName) { fileName_ = fileName; }
			/// gets the wave to render filename
			const std::string fileName() const { return fileName_; }
		private:
			/// wether the recording device has been started.
			bool recording_;
			/// whether to apply dither to recording
			bool recording_with_dither_;
			/// wave render filename
			std::string fileName_;
			/// file to which to output signal.
			WaveFile _outputWaveFile;
			void writeSamplesToFile(int amount);
	///\}

	///\name time info
	///\{
		public:
			PlayerTimeInfo & timeInfo() throw() { return timeInfo_; }
			PlayerTimeInfo const & timeInfo() const throw() { return timeInfo_; }
		private:
			PlayerTimeInfo timeInfo_;
	///\}

	///\name time info ... play position
	///\{
		public:
			/// the current play position
			double playPos() const { return timeInfo_.playBeatPos(); }
			/// sets the current play position
			void setPlayPos(double pos) { timeInfo_.setPlayBeatPos(pos); }
	///\}

	///\name time info ... bpm
	///\{
		public:
			double bpm() const { return timeInfo_.bpm(); }
			void setBpm(double bpm) { timeInfo_.setBpm(bpm); }
	///\}

	///\name start/stop
	///\{
		public:
			/// starts to play.
			void start(double pos = 0);
			/// stops playing.
			void stop();
			void skip(double beats);
			void skipTo(double beatpos);
			/// is the player in playmode.
			bool playing() const { return playing_; }
		private:
			bool playing_;
	///\}

	///\name loop
	///\{
		public:
			bool loopEnabled() const { return timeInfo_.cycleEnabled(); }
			void UnsetLoop() {
				timeInfo_.setCycleStartPos(0.0);
				timeInfo_.setCycleEndPos(0.0);
			}
			void setLoopSong() { 
				timeInfo_.setCycleStartPos(0.0);
				timeInfo_.setCycleEndPos(song().patternSequence().tickLength());
			}
			void setLoopSequenceEntry(SequenceEntry * seqEntry ) {
				timeInfo_.setCycleStartPos(seqEntry->tickPosition());
				timeInfo_.setCycleEndPos(seqEntry->tickEndPosition());
			}
			void setLoopRange(double loopStart, double loopEnd) { 
				timeInfo_.setCycleStartPos(loopStart);
				timeInfo_.setCycleEndPos(loopEnd);
			}
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
			typedef std::scoped_lock<std::mutex> scoped_lock;
			std::mutex & work_mutex() { return mutex_; }
		private:
			std::mutex mutable work_mutex_;
	///\}

	private:
		/// Final Loop. Read new line for notes to send to the Machines
#if 0
		//replaced by sequencer
		void execute_notes(double beat_offset, PatternEvent& line);
#endif
		void process_global_event(const GlobalEvent & event);
		void process(int samples);

		/// stores which machine played last in each track. this allows you to not specify the machine number everytime in the pattern.
		Machine::id_type prev_machines_[MAX_TRACKS];
		/// temporary buffer to get all the audio from master (which work in small chunks), and send it to the soundcard after converting it to float.
		float buffer_[MAX_DELAY_BUFFER];

		/// dither handler
		dsp::Dither dither;

	///\name multithreaded scheduler
	///\{
		private:
			void start_threads();
			void stop_threads();

			typedef Machine node;

			std::size_t thread_count_;
			typedef std::list<std::thread *> threads_type;
			threads_type threads_;
			void thread_function(std::size_t thread_number);

			std::mutex mutable mutex_;
			std::condition<scoped_lock> mutable condition_;

			bool stop_requested_;
			bool suspend_requested_;
			std::size_t suspended_;

			typedef std::list<node*> nodes_queue_type;
			/// nodes with no dependency.
			nodes_queue_type terminal_nodes_;
			/// nodes ready to be processed, just waiting for a free thread
			nodes_queue_type nodes_queue_;

			std::size_t graph_size_, processed_node_count_;

			void suspend_and_compute_plan();
			void compute_plan();
			void clear_plan();

			void process_loop() throw(std::exception);
			void process(node &) throw(std::exception);
			int samples_to_process_;

			Sequencer sequencer_;
	///\}
};

}}
#endif
