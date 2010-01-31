// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "player.h"

#include "internal_machines.h"
#include "machine.h"
#include "sampler.h"
#include "song.h"
#if defined _WIN32 || defined _WIN64 
	#include "vsthost.h"
	#include "machinefactory.h"
#endif


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
using namespace psycle::audiodrivers;

namespace loggers = universalis::os::loggers;

namespace {
	static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE bool this_thread_suspended_ = false;
}

namespace { bool const ultra_trace(false); }

Player::Player()
:
	Tweaker(),
	driver_(),
	autoRecord_(),
	song_(),
	recording_(),
	recording_with_dither_(),
	playing_(),
	autoStopMachines_(),
	autostop_(true) {
	driver_ = default_driver_ = new DummyDriver();
	universalis::os::aligned_memory_alloc(16, buffer_, MAX_SAMPLES_WORKFN);
	for(int i(0); i < MAX_TRACKS; ++i) prev_machines_[i] = 255;
	start_threads();
}

Player::~Player() {
	stop_threads();
	delete default_driver_;
	universalis::os::aligned_memory_dealloc(buffer_);
}


void Player::start_threads() {
	if(loggers::trace()) loggers::trace()("psycle: core: player: starting scheduler threads", UNIVERSALIS__COMPILER__LOCATION);
	if(!threads_.empty()) {
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

	if(thread_count_ < 2) return; // don't create any thread, will use a single-threaded, recursive processing

	try {
		// start the scheduling threads
		for(std::size_t i(0); i < thread_count_; ++i) {
			std::thread* newthread = new std::thread(boost::bind(&Player::thread_function, this, i));
#ifdef DIVERSALIS__OS__MICROSOFT
			newthread->applyPriorityWindows(std::thread::HIGH);
#endif
			threads_.push_back(newthread);
		}
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
	{
		scoped_lock lock(song());
		if(autoRecord_) startRecording();

		Master & master(static_cast<Master&>(*song().machine(MASTER_INDEX)));
		master._clip = false;
		master.sampleCount = 0;

		for(int i(0); i < MAX_TRACKS; ++i) prev_machines_[i] = 255;
		playing_ = true;
		timeInfo_.setPlayBeatPos(pos);
		timeInfo_.setTicksSpeed(song().ticksSpeed(), song().isTicks());
	}
}

void Player::skip(double beats) {
	skipTo(timeInfo_.playBeatPos() + beats);
}
void Player::skipTo(double beatpos) {
	if (!playing_) return;

	timeInfo_.setPlayBeatPos(beatpos);

	scoped_lock lock(song());
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
		Machine & n(*song().machine(m));
		// find the terminal nodes in the graph (nodes with no connected input ports)
		if(n.sched_inputs().empty()) terminal_nodes_.push_back(&n);
	}
	if(ultra_trace && loggers::trace()) {
		std::ostringstream s;
		s << "psycle: core: player: terminal nodes:";
		for(nodes_queue_type::const_iterator i(terminal_nodes_.begin()), e(terminal_nodes_.end()); i != e; ++i) {
			Machine & node(**i);
			s << "\n\t" << node.GetEditName();
		}
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
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
		if(threads_.empty()) // single-threaded, recursive processing
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
#ifdef DIVERSALIS__OS__POSIX
	this_thread::applyPriority(std::thread::HIGH);
#endif
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
		Machine * node_;
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
		Machine & node(*node_);

		if(ultra_trace && loggers::trace()) {
			scoped_lock lock(mutex_);
			std::ostringstream s;
			s << "psycle: core: player: processing node: " << node.GetEditName();
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}

		bool const done = node.sched_process(samples_to_process_);
	
		if(ultra_trace && loggers::trace()) {
			scoped_lock lock(mutex_);
			std::ostringstream s;
			s << "psycle: core: player: processed node: " << node.GetEditName() << ", done: " << done;
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}

		int notify(0);
		{ scoped_lock lock(mutex_);
			node.sched_processed_ = done;
			//If not done, it means it needs to be reprocessed somewhere.
			if (!done) ++graph_size_;

			// check whether all nodes have been processed
			if(++processed_node_count_ == graph_size_) notify = -1; // wake up the main processing loop
			else {
				// check whether successors of the node we processed are now ready.
				// iterate over all the outputs of the node we processed
				Machine::sched_deps output_nodes(node.sched_outputs());
				for(Machine::sched_deps::const_iterator i(output_nodes.begin()), e(output_nodes.end()); i != e; ++i) {
					Machine & output_node(*const_cast<Machine*>(*i));
					bool output_node_ready(true);
					// iterate over all the inputs connected to our output
					Machine::sched_deps input_nodes(output_node.sched_inputs());
					for(Machine::sched_deps::const_iterator i(input_nodes.begin()), e(input_nodes.end()); i != e; ++i) {
						const Machine & input_node(**i);
						if(&input_node == &node) continue;
						if(ultra_trace && loggers::trace()) {
							std::ostringstream s;
							s << "psycle: core: player: node: " << node.GetEditName()
								<< ", output: " << output_node.GetEditName()
								<< ", input: " << input_node.GetEditName()
								<< ", ready: " << input_node.sched_processed_;
							loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						if(!input_node.sched_processed_) {
							output_node_ready = false;
							break;
						}
					}
					if(ultra_trace && loggers::trace()) {
						std::ostringstream s;
						s << "psycle: core: player: node: " << node.GetEditName()
							<< ", output: " << output_node.GetEditName() << ", ready: " << output_node_ready;
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(output_node_ready) {
						// All the dependencies of the node have been processed.
						// We add the node to the processing queue.
						nodes_queue_.push_back(&output_node);
						++notify;
					}
				}
			}
		}
		switch(notify) {
			case -1: main_condition_.notify_one(); break; // wake up the main processing loop
			case 0: break; // no successor ready
			case 1: break; // If there's only one successor ready, we don't notify since it can be processed in the same thread.
			case 2: condition_.notify_one(); break; // notify one thread that we added nodes to the queue
			default: condition_.notify_all(); // notify all threads that we added nodes to the queue
		}
	}
}

void Player::stop() {
	if(!song_) return;

	if(loggers::information()) loggers::information()("psycle: core: player: stopping");
	
	scoped_lock lock(song());
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

void Player::stop_threads() {
	if(loggers::trace()) loggers::trace()("terminating and joining scheduler threads ...", UNIVERSALIS__COMPILER__LOCATION);
	if(threads_.empty()) {
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
#if defined _WIN32 || defined _WIN64 
	//this updates the vstTimeInfo. The plugins themselves are still informed via the setsamplerate call
	((vst::host*)MachineFactory::getInstance().getHosts()[Hosts::VST])->ChangeSampleRate(samples_per_second);
#endif

	if(!song_) return;
	///\todo update the source code of the plugins...
	for(int i(0) ; i < MAX_MACHINES; ++i) if(song().machine(i)) song().machine(i)->SetSampleRate(samples_per_second);
}

float * Player::Work(int numSamples) {
	assert(numSamples < MAX_SAMPLES_WORKFN);

	if(!song_) return buffer_;

	scoped_lock lock(song());
	if (!song().IsReady()) {
		dsp::Clear(buffer_, numSamples);
		return buffer_;
	}

	// Prepare the buffer that the Master Machine writes to. It is done here because process() can be called several times.
	///\todo: The buffer has to be served by the audiodriver, since the audiodriver knows the size it needs to have.
	((Master*)song().machine(MASTER_INDEX))->_pMasterSamples = buffer_;
	
	if(autoRecord_ && timeInfo_.playBeatPos() >= song().sequence().tickLength())
		stopRecording();

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
		} else {
			// autostop
			if (timeInfo_.playBeatPos() >= song().sequence().max_beats()) {
				stop();
			}
		}
		sequencer_.set_player(*this); // for a callback to process()
		sequencer_.set_time_info(timeInfo_);
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

	bool was_opened = driver_->opened();
	bool was_started = driver_->started();

	if(&driver == driver_) {
		// same driver instance
		driver.set_started(false);
	} else {
		// different driver instance
		driver_->set_opened(false);
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
	if(!song_) return;

	float * pL(song().machine(MASTER_INDEX)->_pSamplesL);
	float * pR(song().machine(MASTER_INDEX)->_pSamplesR);
	if(recording_with_dither_) {
		dither_.Process(pL, amount);
		dither_.Process(pR, amount);
	}
	switch(driver_->playbackSettings().channelMode()) {
		case 0: // mono mix
			for(int i(0); i < amount; ++i)
				//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
				if(outputWaveFile_.WriteMonoSample((*pL++ + *pR++) / 2) != DDC_SUCCESS) stopRecording();
			break;
		case 1: // mono L
			for(int i(0); i < amount; ++i)
				if(outputWaveFile_.WriteMonoSample(*pL++) != DDC_SUCCESS) stopRecording();
			break;
		case 2: // mono R
			for(int i(0); i < amount; ++i)
				if(outputWaveFile_.WriteMonoSample(*pR++) != DDC_SUCCESS) stopRecording();
			break;
		default: // stereo
			for(int i(0); i < amount; ++i)
				if(outputWaveFile_.WriteStereoSample(*pL++, *pR++) != DDC_SUCCESS) stopRecording();
			break;
	}
}

void Player::startRecording(bool do_dither, dsp::Dither::Pdf::type ditherpdf, dsp::Dither::NoiseShape::type noiseshaping) {
	if(recording_) return;
	if(!song_) return;

	scoped_lock lock(song());
	int channels(driver_->playbackSettings().numChannels());
	recording_ = DDC_SUCCESS == outputWaveFile_.OpenForWrite(fileName().c_str(), driver_->playbackSettings().samplesPerSec(), driver_->playbackSettings().bitDepth(), channels);
	recording_with_dither_ = do_dither;
	if(do_dither) {
		dither_.SetBitDepth(driver_->playbackSettings().bitDepth());
		dither_.SetPdf(ditherpdf);
		dither_.SetNoiseShaping(noiseshaping);
	}
}

void Player::stopRecording() {
	if(!recording_) return;

	scoped_lock lock(song());
	outputWaveFile_.Close();
	recording_ = false;
}

}}