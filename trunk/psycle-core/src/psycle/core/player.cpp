// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "player.h"

#include "internal_machines.h"
#include "machine.h"
#include "sampler.h"
#include "song.h"

#include <psycle/audiodrivers/audiodriver.h>
#include <psycle/helpers/value_mapper.hpp>
#include <universalis/os/loggers.hpp>
#include <universalis/os/cpu_affinity.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/cpu/exception.hpp>
#include <universalis/os/aligned_memory_alloc.hpp>
#include <boost/bind.hpp>

namespace psycle { namespace core {

using namespace helpers;
namespace loggers = universalis::os::loggers;

namespace {
	static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE bool this_thread_suspended_ = false;
}

Player & Player::singleton() {
	// note: keep sure a player instance is created from the gui
	// before starting audiothread
	// or use single threaded only
	static Player player;
	return player; 
}

Player::Player()
:
	Tweaker(),
	driver_(),
	autoRecord_(),
	song_(),
	recording_(),
	recording_with_dither_(),
	playing_(),
	autoStopMachines_()
{
	universalis::os::aligned_memory_alloc(16, buffer_, MAX_SAMPLES_WORKFN);
	for(int i(0); i < MAX_TRACKS; ++i) prev_machines_[i] = 255;
	start_threads();
}

void Player::start_threads() {
	if(loggers::trace()) loggers::trace()("psycle: core: player: starting scheduler threads", UNIVERSALIS__COMPILER__LOCATION);
	if(threads_.size()) {
		if(loggers::trace()) loggers::trace()("psycle: core: player: scheduler threads are already running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}

	// normally, we would compute the scheduling plan here,
	// but we haven't yet implemented signals that notifies when connections are changed or nodes added/removed.
	// so it's actually done every time in the processing loop
	//compute_plan();

	stop_requested_ = suspend_requested_ = false;
	processed_node_count_ = suspended_ = 0;

	thread_count_ = universalis::os::cpu_affinity::cpu_count();
	thread_count_ = 0; // multithreading disabled by default
	{ // thread count env var
		char const * const env(std::getenv("PSYCLE_THREADS"));
		if(env) {
			std::stringstream s;
			s << env;
			s >> thread_count_;
		}
	}
	
	if(loggers::information()) {
		std::ostringstream s;
		s << "psycle: core: player: using " << thread_count_ << " threads";
		loggers::information()(s.str());
	}

	if(!thread_count_) return; // don't create any thread, will use a single-threaded, recursive processing

	try {
		// start the scheduling threads
		for(std::size_t i(0); i < thread_count_; ++i)
			threads_.push_back(new std::thread(boost::bind(&Player::thread_function, this, i)));
	} catch(...) {
		{ scoped_lock lock(mutex_);
			stop_requested_ = true;
		}
		condition_.notify_all();
		for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
			(**i).join();
			delete *i;
		}
		threads_.clear();
		clear_plan();
		throw;
	}
}

void Player::start(double pos) {
	stop(); // This causes all machines to reset, and samplesperRow to init.

	if(loggers::information()) loggers::information()("psycle: core: player: starting");
	if(!song_) {
		if(loggers::warning()) loggers::warning()("psycle: core: player: no song to play");
		return;
	}
	if(!driver_) {
		if(loggers::warning()) loggers::warning()("psycle: core: player: no audio driver to output the song to");
		return;
	}

	scoped_lock lock(song().Mutex());
	if(autoRecord_) startRecording();

	Master & master(static_cast<Master&>(*song().machine(MASTER_INDEX)));
	master._clip = false;
	master.sampleCount = 0;
	
	for(int i(0); i < MAX_TRACKS; ++i) prev_machines_[i] = 255;
	playing_ = true;
	timeInfo_.setPlayBeatPos(pos);
	timeInfo_.setTicksSpeed(song().ticksSpeed(), song().isTicks());

	driver_->set_started(true);
}

void Player::skip(double beats) {
	scoped_lock lock(song().Mutex());
	if (!playing_) 
		return;

	skipTo(timeInfo_.playBeatPos()+beats);
	
}
void Player::skipTo(double beatpos) {
	scoped_lock lock(song().Mutex());
	if (!playing_) 
		return;

	timeInfo_.setPlayBeatPos(beatpos);
	Master & master(static_cast<Master&>(*song().machine(MASTER_INDEX)));
	master.sampleCount = 60 * beatpos / bpm();

}

void Player::suspend_and_compute_plan() {
	if(!playing_) {
		// no threads running, so no nothing to suspend; simply compute the plan and return.
		compute_plan();
		return;
	}
	// before we compute the plan, we need to suspend all the threads.
	{ scoped_lock lock(mutex_);
		suspend_requested_ = true;
	}
	condition_.notify_all();  // notify all threads they must suspend
	{ scoped_lock lock(mutex_);
		// wait until all threads are suspended
		while(suspended_ != threads_.size()) main_condition_.wait(lock);
		compute_plan();
		suspend_requested_ = false;
	}
	condition_.notify_all(); // notify all threads they can resume
}

void Player::compute_plan() {
	graph_size_ = 0;
	terminal_nodes_.clear();

	// iterate over all the nodes
	for(int m(0); m < MAX_MACHINES; ++m) if(song().machine(m)) {
		++graph_size_;
		node & n(*song().machine(m));
		// find the terminal nodes in the graph (nodes with no connected input ports)
		if(!n._connectedInputs) terminal_nodes_.push_back(&n);
	}

	// copy the initial processing queue
	nodes_queue_ = terminal_nodes_;
}

void Player::clear_plan() {
	nodes_queue_.clear();
	terminal_nodes_.clear();
}

void Player::process(int samples) {
	int remaining_samples = samples;
	while(remaining_samples) {
		int const amount(std::min(remaining_samples, MAX_BUFFER_LENGTH));
		// reset all machine buffers
		for(int c(0); c < MAX_MACHINES; ++c) if(song().machine(c)) song().machine(c)->PreWork(amount);
		Sampler::DoPreviews(amount, song().machine(MASTER_INDEX)->_pSamplesL, song().machine(MASTER_INDEX)->_pSamplesR);
		if(!threads_.size()) // single-threaded, recursive processing
			song().machine(MASTER_INDEX)->Work(amount);
		else { // multi-threaded scheduling
			// we push all the terminal nodes to the processing queue
			{ scoped_lock lock(mutex_);
				compute_plan(); // it's overkill, but we haven't yet implemented signals that notifies when connections are changed or nodes added/removed.
				processed_node_count_ = 0;
				samples_to_process_ = amount;
			}
			condition_.notify_all(); // notify all threads that we added nodes to the queue
			// wait until all nodes have been processed
			{ scoped_lock lock(mutex_);
				while(processed_node_count_ != graph_size_) main_condition_.wait(lock);
			}
		}
		// write samples to file
		///\josepma: I don't understand this check. autoRecord doesn't play a role here
		if(recording_ && (playing_ || !autoRecord_)) writeSamplesToFile(amount); 
		// move the pointer forward for the next Master::Work() iteration.
		((Master*)song().machine(MASTER_INDEX))->_pMasterSamples += amount * 2;
		remaining_samples -= amount;
		// increase the timeInfo playBeatPos by the number of beats corresponding to the amount of samples we processed
		timeInfo_.setPlayBeatPos(timeInfo_.playBeatPos() + amount / timeInfo_.samplesPerBeat());
		timeInfo_.setSamplePos(((Master*)song().machine(MASTER_INDEX))->sampleCount);
	}
}

void Player::thread_function(std::size_t thread_number) {
	if(loggers::trace()) {
		scoped_lock lock(mutex_);
		std::ostringstream s;
		s << "psycle: core: player: scheduler thread #" << thread_number << " started";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	universalis::os::thread_name thread_name;
	{ // set thread name
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << '#' << thread_number;
		thread_name.set(s.str());
	}

	// install cpu/os exception handler/translator
	universalis::cpu::exception::install_handler_in_thread();

	try {
		try {
			process_loop();
		} catch(...) {
			loggers::exception()("caught exception in scheduler thread", UNIVERSALIS__COMPILER__LOCATION);
			throw;
		}
	} catch(std::exception const & e) {
		e;
		if(loggers::exception()) {
			std::ostringstream s;
			s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	} catch(...) {
		if(loggers::exception()) {
			std::ostringstream s;
			s << "exception: " << universalis::compiler::exceptions::ellipsis();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	}

	if(loggers::trace()) {
		scoped_lock lock(mutex_);
		std::ostringstream s;
		s << "psycle: core: player: scheduler thread #" << thread_number << " terminated";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void Player::process_loop() throw(std::exception) {
	while(true) {
		Player::node * node_;
		{ scoped_lock lock(mutex_);
			while(
				!nodes_queue_.size() &&
				!suspend_requested_ &&
				!stop_requested_
			) condition_.wait(lock);

			if(stop_requested_) break;

			if(suspend_requested_) {
				if(!this_thread_suspended_) {
					this_thread_suspended_ = true;
					++suspended_;
					main_condition_.notify_one();
				}
				continue;
			}
			if(this_thread_suspended_) {
				this_thread_suspended_ = false;
				--suspended_;
			}
			// There are nodes waiting in the queue. We pop the first one.
			node_ = nodes_queue_.front();
			nodes_queue_.pop_front();
		}
		Player::node & node(*node_);

		process(node);
	
		int notify(0);
		{ scoped_lock lock(mutex_);
			node.processed_by_multithreaded_scheduler_ = true;
			// check whether all nodes have been processed
			if(++processed_node_count_ == graph_size_) notify = -1; // wake up the main processing loop
			else // check whether successors of the node we processed are now ready.
				// iterate over all the outputs of the node we processed
				if(node._connectedOutputs) for(int c(0); c < MAX_CONNECTIONS; ++c) if(node._connection[c]) {
					Player::node & output_node(*song().machine(node._outputMachines[c]));
					bool output_node_ready(true);
					// iterate over all the inputs connected to our output
					if(output_node._connectedInputs) for(int c(0); c < MAX_CONNECTIONS; ++c) if(output_node._inputCon[c]) {
						Player::node & input_node(*song().machine(output_node._inputMachines[c]));
						if(!input_node.processed_by_multithreaded_scheduler_) {
							output_node_ready = false;
							break;
						}
					}
					if(output_node_ready) {
						// All the dependencies of the node have been processed.
						// We add the node to the processing queue.
						nodes_queue_.push_back(&output_node);
						++notify;
					}
				}
		}
		switch(notify) {
			case -1:
				main_condition_.notify_one(); // wake up the main processing loop
			case 0:
				// no successor ready
			break;
			case 1:
				// If there's only one successor ready, we don't notify since it can be processed in the same thread.
			break;
			case 2:
				condition_.notify_one(); // notify one thread that we added nodes to the queue
			break;
			default:
				condition_.notify_all(); // notify all threads that we added nodes to the queue
		}
	}
}

void Player::process(Player::node & node) throw(std::exception) {
	if(loggers::trace()) {
		scoped_lock lock(mutex_);
		std::ostringstream s;
		s << "psycle: core: player: processing node: " << node.GetEditName();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	///\todo the mixer machine needs this to be set to false
	bool const mix(true);

	if(node._connectedInputs) for(int i(0); i < MAX_CONNECTIONS; ++i) if(node._inputCon[i]) {
		Player::node & input_node(*song().machine(node._inputMachines[i]));
		if(!input_node.Standby()) node.Standby(false);
		if(!node._mute && !node.Standby() && mix) {
			dsp::Add(input_node._pSamplesL, node._pSamplesL, samples_to_process_, input_node.lVol() * node._inputConVol[i]);
			dsp::Add(input_node._pSamplesR, node._pSamplesR, samples_to_process_, input_node.rVol() * node._inputConVol[i]);
		}
	}
	dsp::Undenormalize(node._pSamplesL, node._pSamplesR, samples_to_process_);
	node.GenerateAudio(samples_to_process_);
}

void Player::stop() {
	if(loggers::information()) loggers::information()("psycle: core: player: stopping");
	if(!song_ || !driver_) return;
	
	scoped_lock lock(song().Mutex());
	playing_ = false;
	for(int i(0); i < MAX_MACHINES; ++i) if(song().machine(i)) {
		song().machine(i)->Stop();
		for(int c(0); c < MAX_TRACKS; ++c) song().machine(i)->TriggerDelay[c].setCommand(0);
	}
	setBpm(song().bpm());
	timeInfo_.setTicksSpeed(song().ticksSpeed(), song().isTicks());
	samples_per_second(driver_->playbackSettings().samplesPerSec());
	// Disabled, because in psyclemfc, autorecording is used sometimes to record live, and pressing stop
	// is a way to make autorecording ignore the song length and keep recording until intentionally stopping
	// the recording (not the playback)
	//if(autoRecord_) stopRecording();
}

Player::~Player() {
	stop_threads();
	universalis::os::aligned_memory_dealloc(buffer_);
}

void Player::stop_threads() {
	if(loggers::trace()) loggers::trace()("terminating and joining scheduler threads ...", UNIVERSALIS__COMPILER__LOCATION);
	if(!threads_.size()) {
		if(loggers::trace()) loggers::trace()("scheduler threads were not running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
	}
	condition_.notify_all();
	for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
		(**i).join();
		delete *i;
	}
	if(loggers::trace()) loggers::trace()("scheduler threads joined", UNIVERSALIS__COMPILER__LOCATION);
	threads_.clear();
	clear_plan();
}

void Player::samples_per_second(int samples_per_second) {
	timeInfo_.setSampleRate(samples_per_second);
	if(!song_) return;
	///\todo update the source code of the plugins...
	for(int i(0) ; i < MAX_MACHINES; ++i) if(song().machine(i)) song().machine(i)->SetSampleRate(samples_per_second);
}


/// Final Loop. Read new line for notes to send to the Machines
#if 0
		//replaced by sequencer
void Player::execute_notes(double beat_offset, PatternEvent& entry) {
	// WARNING!!! In this function, the events inside the patterline are assumed to be temporary! (thus, modifiable)

	int track = entry.track();
	int sequence_track = entry.sequence_track();

	// step 1: process all tweaks.
	{
		int mac = entry.machine();

		// not a valid machine id?
		if(mac >= MAX_MACHINES || !song().machine(mac))
			return;
			
		Machine & machine = *song().machine(mac);
		
		switch(entry.note()) {
			case notetypes::tweak_slide: {
				int const delay(64);
				int delaysamples(0), origin(machine.GetParamValue(entry.instrument()));
				float increment(origin);
				int previous(0);
				float rate = (((entry.command() << 16 ) | entry.parameter()) - origin) / (timeInfo().samplesPerTick() / 64.0f);
					entry.setNote(notetypes::tweak);
					entry.setCommand(origin >> 8);
					entry.setParameter(origin & 0xff);
					machine.AddEvent(
						beat_offset + static_cast<double>(delaysamples) / timeInfo().samplesPerBeat(),
						sequence_track * 1024 + track, entry
					);
					previous = origin;
					delaysamples += delay;
					while(delaysamples < timeInfo().samplesPerTick()) {
						increment += rate;
						if(static_cast<int>(increment) != previous) {
							origin = static_cast<int>(increment);
							entry.setCommand(origin >> 8);
							entry.setParameter(origin & 0xff);
							machine.AddEvent(
								beat_offset + static_cast<double>(delaysamples) / timeInfo().samplesPerBeat(),
								sequence_track * 1024 + track, entry
							);
							previous = origin;
						}
						delaysamples += delay;
					}
			} break;
			case notetypes::tweak:
				machine.AddEvent(beat_offset, sequence_track * 1024 + track, entry);
			break;
			default: 
				;
		}
	}
	
	// step 2: collect note
	{
		// track muted?
		if(song().patternSequence().trackMuted(track)) return;

		// not a note ?
		if(entry.note() >= notetypes::tweak && entry.note() != 255) return;

		int mac = entry.machine();
		if(mac != 255) prev_machines_[track] = mac;
		else mac = prev_machines_[track];

		// not a valid machine id?
		if(mac == 255 || mac >= MAX_MACHINES) return;
			
		// no machine with this id?
		if(!song().machine(mac)) return;
		
		Machine& machine = *song().machine(mac);

		// machine muted?
		if(machine._mute) return;

		switch(entry.command()) {
			case commandtypes::NOTE_DELAY: {
				double delayoffset(entry.parameter() / 256.0);
				// At least Plucked String works erroneously if the command is not ommited.
				entry.setCommand(0); entry.setParameter(0);
				machine.AddEvent(beat_offset + delayoffset, sequence_track * 1024 + track, entry);
			} break;
			case commandtypes::RETRIGGER: {
				///\todo: delaysamples and rate should be memorized (for RETR_CONT command ). Then set delaysamples to zero in this function.
				int delaysamples(0);
				int rate = entry.parameter() + 1;
				int delay = (rate * static_cast<int>(timeInfo().samplesPerTick())) >> 8;
				entry.setCommand(0); entry.setParameter(0);
				machine.AddEvent(beat_offset, sequence_track * 1024 + track, entry);
				delaysamples += delay;
				while(delaysamples < timeInfo().samplesPerTick()) {
					machine.AddEvent(
					beat_offset + static_cast<double>(delaysamples) / timeInfo().samplesPerBeat(),
					sequence_track * 1024 + track, entry
				);
				delaysamples += delay;
			}
		} break;
		case commandtypes::RETR_CONT: {
			///\todo: delaysamples and rate should be memorized, do not reinit delaysamples.
			///\todo: verify that using ints for rate and variation is enough, or has to be float.
			int delaysamples(0), rate(0), delay(0), variation(0);
			int parameter = entry.parameter() & 0x0f;
			variation = (parameter < 9) ? (4 * parameter) : (-2 * (16 - parameter));
			if(entry.parameter() & 0xf0) rate = entry.parameter() & 0xf0;
			delay = (rate * static_cast<int>(timeInfo().samplesPerTick())) >> 8;
			entry.setCommand(0); entry.setParameter(0);
			machine.AddEvent(
				beat_offset + static_cast<double>(delaysamples) / timeInfo().samplesPerBeat(),
				sequence_track * 1024 + track, entry
			);
			delaysamples += delay;
			while(delaysamples < timeInfo().samplesPerTick()) {
				machine.AddEvent(
					beat_offset + static_cast<double>(delaysamples) / timeInfo().samplesPerBeat(),
					sequence_track * 1024 + track, entry
				);
				rate += variation;
				if(rate < 16) rate = 16;
				delay = (rate * static_cast<int>(timeInfo().samplesPerTick())) >> 8;
				delaysamples += delay;
			}
		} break;
		case commandtypes::ARPEGGIO: {
			///\todo : Add Memory.
			///\todo : This won't work... What about sampler's NNA's?
			#if 0
				if(entry.parameter()) {
					machine.TriggerDelay[track] = entry;
					machine.ArpeggioCount[track] = 1;
				}
				machine.RetriggerRate[track] = static_cast<int>(timeInfo_.samplesPerTick() * timeInfo_.linesPerBeat() / 24);
			#endif
		} break;
		default:
			machine.TriggerDelay[track].setCommand(0);
			machine.AddEvent(beat_offset, sequence_track * 1024 + track, entry);
			machine.TriggerDelayCounter[track] = 0;
			machine.ArpeggioCount[track] = 0;
		}
	}
}
#endif

float * Player::Work(int numSamples) {
	assert(numSamples < MAX_SAMPLES_WORKFN);

	if(!song_) return buffer_;

	scoped_lock lock(song().Mutex());
	if (!song().IsReady()) {
		dsp::Clear(buffer_, numSamples);
		return buffer_;
	}

	// Prepare the buffer that the Master Machine writes to. It is done here because process() can be called several times.
	///\todo: The buffer has to be served by the audiodriver, since the audiodriver knows the size it needs to have.
	((Master*)song().machine(MASTER_INDEX))->_pMasterSamples = buffer_;
	
	if(autoRecord_ && timeInfo_.playBeatPos() >= song().patternSequence().tickLength()) stopRecording();

	if(!playing_) {
		///\todo: Need to add the events coming from the MIDI device. (Of course, first we need the MIDI device)
		process(numSamples);
		//playPos += beatLength;
		//if(playPos > "signumerator") playPos -= signumerator;
	} else {
		if(loopEnabled()) {
			// Maintain the cursor inside the loop sequence
			if(
				timeInfo_.playBeatPos() >= timeInfo_.cycleEndPos() ||
				timeInfo_.playBeatPos() < timeInfo_.cycleStartPos()
			) setPlayPos(timeInfo_.cycleStartPos());
		}
		sequencer_.set_player(*this); // for a callback to process()
		sequencer_.set_time_info(&timeInfo_);
		sequencer_.Work(numSamples);
	}
	return buffer_;
}

void Player::setDriver(AudioDriver & driver) {
	if(loggers::trace()) {
		std::ostringstream s;
		s << "psycle: core: player: setting audio driver to: " << driver.info().name();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	bool was_opened(driver_ ? driver_->opened() : false);
	bool was_started(driver_ ? driver_->started() : false);

	if(&driver == driver_) {
		// same driver instance
		driver.set_started(false);
	} else {
		// different driver instance
		if(driver_) driver_->set_opened(false);
		driver_ = &driver;
	}

	driver.set_callback(Work, this);
	
	if(was_started && loggers::trace()) {
		std::ostringstream s;
		s << "psycle: core: player: starting audio driver: " << driver.info().name();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	driver.set_opened(was_opened);
	driver.set_started(was_started);

	samples_per_second(driver.playbackSettings().samplesPerSec());
}

/*****************************************************************************/
// buffer to riff wav file methods

void Player::writeSamplesToFile(int amount) {
	if(!song_ || !driver_) return;
	float * pL(song().machine(MASTER_INDEX)->_pSamplesL);
	float * pR(song().machine(MASTER_INDEX)->_pSamplesR);
	if(recording_with_dither_) {
		dither.Process(pL, amount);
		dither.Process(pR, amount);
	}
	switch(driver_->playbackSettings().channelMode()) {
		case 0: // mono mix
			for(int i(0); i < amount; ++i)
				//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
				if(_outputWaveFile.WriteMonoSample((*pL++ + *pR++) / 2) != DDC_SUCCESS) stopRecording();
			break;
		case 1: // mono L
			for(int i(0); i < amount; ++i)
				if(_outputWaveFile.WriteMonoSample(*pL++) != DDC_SUCCESS) stopRecording();
			break;
		case 2: // mono R
			for(int i(0); i < amount; ++i)
				if(_outputWaveFile.WriteMonoSample(*pR++) != DDC_SUCCESS) stopRecording();
			break;
		default: // stereo
			for(int i(0); i < amount; ++i)
				if(_outputWaveFile.WriteStereoSample(*pL++, *pR++) != DDC_SUCCESS) stopRecording();
			break;
	}
}

void Player::startRecording(bool dodither , dsp::Dither::Pdf::type ditherpdf, dsp::Dither::NoiseShape::type noiseshaping)
{
	if(!song_ && !driver_) return;
	
	scoped_lock lock(song().Mutex());
	if(recording_) return;
	int channels(driver_->playbackSettings().numChannels());
	recording_ =( DDC_SUCCESS == _outputWaveFile.OpenForWrite(fileName().c_str(), driver_->playbackSettings().samplesPerSec(), driver_->playbackSettings().bitDepth(), channels));
	recording_with_dither_ = dodither;
	if (dodither) {
		dither.SetBitDepth(driver_->playbackSettings().bitDepth());
		dither.SetPdf(ditherpdf);
		dither.SetNoiseShaping(noiseshaping);
	}
}

void Player::stopRecording() {

	scoped_lock lock(song().Mutex());
	if(!recording_) return;
	_outputWaveFile.Close();
	recording_ = false;
}

}}
