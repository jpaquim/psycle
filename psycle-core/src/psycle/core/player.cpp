// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\implementation psy::core::Player.

#include <psycle/core/config.private.hpp>
#include "player.h"

#include "internal_machines.h"
#include "machine.h"
#include "sampler.h"
#include "song.h"
#include <psycle/audiodrivers/audiodriver.h>

///\todo needs link against libuniversalis
//#include <universalis/operating_system/cpu_affinity.hpp>

#include <boost/bind.hpp>
#include <iostream> // only for debug output

namespace psy { namespace core {

	using namespace psycle::helpers;

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
	for(int i(0); i < MAX_TRACKS; ++i) prev_machines_[i] = 255;

	///\todo starting the threads needs to be done elsewhere?
	start_threads();
}

void Player::start_threads() {
	std::cout << "psycle: core: player: starting scheduler threads\n";
	//if(loggers::information()()) loggers::information()("starting scheduler threads on graph " + graph().underlying().name() + " ...", UNIVERSALIS__COMPILER__LOCATION);
	if(threads_.size()) {
		//if(loggers::information()()) loggers::information()("scheduler threads are already running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}

	// normally, we would compute the scheduling plan here,
	// but we haven't yet implemented signals that notifies when connections are changed or nodes added/removed.
	// so it's actually done every time in the processing loop
	//compute_plan();

	stop_requested_ = suspend_requested_ = false;
	processed_node_count_ = suspended_ = 0;

	///\todo needs link against libuniversalis
	//thread_count_ = universalis::operating_system::cpu_affinity::cpu_count();
	thread_count_ = 0;
	{ // thread count env var
		char const * const env(std::getenv("PSYCLE_THREADS"));
		if(env) {
			std::stringstream s;
			s << env;
			s >> thread_count_;
		}
	}
	
	if(!thread_count_) return; // don't create any thread, will use a single-threaded, recursive processing

	try {
		// start the scheduling threads
		std::cout << "psycle: core: player: using " << thread_count_ << " threads\n";
		#if 0
		if(loggers::information()()) {
			std::ostringstream s;
			s << "using " << thread_count_ << " threads";
			loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		#endif
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

	std::cout << "psycle: core: player: starting\n";
	if(!song_) {
		std::cerr << "psycle: core: player: no song to play\n";
		return;
	}
	if(!driver_) {
		std::cerr << "psycle: core: player: no audio driver to output the song to\n";
		return;
	}

	if(autoRecord_) startRecording();

	Master & master(static_cast<Master&>(*song().machine(MASTER_INDEX)));
	master._clip = false;
	master.sampleCount = 0;
	
	for(int i(0); i < MAX_TRACKS; ++i) prev_machines_[i] = 255;
	playing_ = true;
	timeInfo_.setPlayBeatPos(pos);
	timeInfo_.setTicksSpeed(song().ticksSpeed(), song().isTicks());
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
		while(suspended_ != threads_.size()) condition_.wait(lock);
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
		int const amount(std::min(remaining_samples, STREAM_SIZE));
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
				while(processed_node_count_ != graph_size_) condition_.wait(lock);
			}
		}
		// write samples to file
		///\josepma: I don't understand this check. autoRecord doesn't play a role here
		if(recording_ && (playing_ || !autoRecord_)) writeSamplesToFile(amount); 
		// move the pointer forward for the next Master::Work() iteration.
		Master::_pMasterSamples += amount * 2;
		remaining_samples -= amount;
		// increase the timeInfo playBeatPos by the number of beats corresponding to the amount of samples we processed
		timeInfo_.setPlayBeatPos(timeInfo_.playBeatPos() + amount / timeInfo_.samplesPerBeat());
	}
}

void Player::thread_function(std::size_t thread_number) {
	{ scoped_lock lock(mutex_);
		std::cout << "psycle: core: player: scheduler thread #" << thread_number << " started\n";
	}

	#if 0
	if(loggers::information()()) loggers::information()("scheduler thread started on graph " + graph().underlying().name(), UNIVERSALIS__COMPILER__LOCATION);
	
	universalis::operating_system::thread_name thread_name;
	{ // set thread name
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << '#' << graph().underlying().name() << '#' << thread_number;
		thread_name.set(s.str());
	}

	// install cpu/os exception handler/translator
	universalis::processor::exception::install_handler_in_thread();
	#endif

	try {
		try {
			process_loop();
		} catch(...) {
			//loggers::exception()("caught exception in scheduler thread", UNIVERSALIS__COMPILER__LOCATION);
			throw;
		}
	} catch(std::exception const & e) {
		#if 0
		if(loggers::exception()()) {
			std::ostringstream s;
			s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		#endif
		throw;
	} catch(...) {
		#if 0
		if(loggers::exception()()) {
			std::ostringstream s;
			s << "exception: " << universalis::compiler::exceptions::ellipsis();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		#endif
		throw;
	}
	//loggers::information()("scheduler thread on graph " + graph().underlying().name() + " terminated", UNIVERSALIS__COMPILER__LOCATION);
	{ scoped_lock lock(mutex_);
		std::cout << "psycle: core: player: scheduler thread #" << thread_number << " terminated\n";
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
					condition_.notify_all();
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
	
		unsigned int notify(0);
		{ scoped_lock lock(mutex_);
			node.processed_by_multithreaded_scheduler_ = true;
			// check whether all nodes have been processed
			if(++processed_node_count_ == graph_size_) notify = 2; // wake up the main processing loop
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
	#if 0
	{ scoped_lock lock(mutex_);
		std::cout << "psycle: core: player: processing node: " << node.GetEditName() << '\n';
	}
	#endif

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
	std::cout << "psycle: core: player: stopping\n";
	if(!song_ || !driver_) return;
	playing_ = false;
	for(int i(0); i < MAX_MACHINES; ++i) if(song().machine(i)) {
		song().machine(i)->Stop();
		for(int c(0); c < MAX_TRACKS; ++c) song().machine(i)->TriggerDelay[c].setCommand(0);
	}
	setBpm(song().bpm());
	timeInfo_.setTicksSpeed(song().ticksSpeed(), song().isTicks());
	samples_per_second(driver_->settings().samplesPerSec());
	// Disabled, because in psyclemfc, autorecording is used sometimes to record live, and pressing stop
	// is a way to make autorecording ignore the song length and keep recording until intentionally stopping
	// the recording (not the playback)
	//if(autoRecord_) stopRecording();
}

Player::~Player() {
	///\todo stopping the threads needs to be done elsewhere?
	stop_threads();
}

void Player::stop_threads() {
	//if(loggers::information()()) loggers::information()("terminating and joining scheduler threads ...", UNIVERSALIS__COMPILER__LOCATION);
	if(!threads_.size()) {
		//if(loggers::information()()) loggers::information()("scheduler threads were not running", UNIVERSALIS__COMPILER__LOCATION);
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
	//if(loggers::information()()) loggers::information()("scheduler threads joined", UNIVERSALIS__COMPILER__LOCATION);
	threads_.clear();
	clear_plan();
}

void Player::samples_per_second(int samples_per_second) {
	timeInfo_.setSampleRate(samples_per_second);
	if(!song_) return;
	///\todo update the source code of the plugins...
	for(int i(0) ; i < MAX_MACHINES; ++i) if(song().machine(i)) song().machine(i)->SetSampleRate(samples_per_second);
}

void Player::process_global_event(GlobalEvent const & event) {
	Machine::id_type mIndex;
	switch(event.type()) {
		case GlobalEvent::BPM_CHANGE:
			setBpm(event.parameter());
			std::cout << "psycle: core: player: bpm change event found. position: " << timeInfo_.playBeatPos() << ", new bpm: " << event.parameter() << '\n';
			break;
		case GlobalEvent::JUMP_TO:
			timeInfo_.setPlayBeatPos(event.parameter());
			break;
		case GlobalEvent::SET_BYPASS:
			mIndex = event.target();
			if(mIndex < MAX_MACHINES && song().machine(mIndex) && song().machine(mIndex)->acceptsConnections()) //i.e. Effect
				song().machine(mIndex)->_bypass = true;
			break;
		case GlobalEvent::UNSET_BYPASS:
			mIndex = event.target();
			if(mIndex < MAX_MACHINES && song().machine(mIndex) && song().machine(mIndex)->acceptsConnections()) // i.e. Effect
				song().machine(mIndex)->_bypass = false;
			break;
		case GlobalEvent::SET_MUTE:
			mIndex = event.target();
			if(mIndex < MAX_MACHINES && song().machine(mIndex))
				song().machine(mIndex)->_mute = true;
			break;
		case GlobalEvent::UNSET_MUTE:
			mIndex = event.target();
			if(mIndex < MAX_MACHINES && song().machine(mIndex))
				song().machine(mIndex)->_mute = false;
			break;
		case GlobalEvent::SET_VOLUME:
			if(event.target() == 255) {
				Master & master(static_cast<Master&>(*song().machine(MASTER_INDEX)));
				master._outDry = static_cast<int>(event.parameter());
			} else {
				mIndex = event.target();
				if(mIndex < MAX_MACHINES && song().machine(mIndex)) {
					Wire::id_type wire(event.target2());
					song().machine(mIndex)->SetDestWireVolume(mIndex, wire,
						CValueMapper::Map_255_1(static_cast<int>(event.parameter()))
					);
				}
			}
		case GlobalEvent::SET_PANNING:
			mIndex = event.target();
			if(mIndex < MAX_MACHINES && song().machine(mIndex))
				song().machine(mIndex)->SetPan(static_cast<int>( event.parameter()));
			break;
		default: ;
	}
}

/// Final Loop. Read new line for notes to send to the Machines
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

float * Player::Work(int numSamples) {
	if(!song_) return buffer_;

	scoped_lock lock(work_mutex());

	// Prepare the buffer that the Master Machine writes to. It is done here because process() can be called several times.
	Master::_pMasterSamples = buffer_;
	double beatsToWork = numSamples / static_cast<double>(timeInfo_.samplesPerBeat());
	
	///\todo CSingleLock crit(&song().door, true);

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
		
		std::vector<GlobalEvent*> globals;

		// processing of each buffer is subdivided into chunks, determined by the placement of any global events.
		
		// end beat position of the current chunk-- i.e., the next global event's position.
		double chunkBeatEnd;
		// number of beats in the chunk.
		double chunkBeatSize;
		// number of samples needed to process for this chunk.
		int chunkSampleSize;
		// this is used to counter rounding errors of sample/beat conversions
		int processedSamples(0);
		// whether this is the first time through the loop.  this is passed to GetNextGlobalEvents()
		// to specify that we're including events at exactly playPos -only- on the first iteration--
		// otherwise we'll get the first event over and over again.
		bool bFirst(true);
		do {
			// get the next round of global events.  we need to repopulate the list of globals and patternlines
			// each time through the loop because global events can potentially move the song's beatposition elsewhere.
			globals.clear();
			chunkBeatEnd = song().patternSequence().GetNextGlobalEvents(timeInfo_.playBeatPos(), beatsToWork, globals, bFirst);
			if(loopEnabled()) {
				// Don't go further than the sequenceEnd.
				if(chunkBeatEnd >= timeInfo_.cycleEndPos())
					chunkBeatEnd = timeInfo_.cycleEndPos();
			}

			// determine chunk length in beats and samples.
			chunkBeatSize = chunkBeatEnd - timeInfo_.playBeatPos();
			if(globals.empty())
				chunkSampleSize = numSamples - processedSamples;
			else
				chunkSampleSize = static_cast<int>(chunkBeatSize * timeInfo_.samplesPerBeat());

			// get all patternlines occuring before the next global event, execute them, and process
			sequencer_.set_time_info(&timeInfo_);
			sequencer_.Work(numSamples);
			if(chunkSampleSize > 0) {
				process(chunkSampleSize);
				processedSamples += chunkSampleSize;
			}
			
			beatsToWork -= chunkBeatSize;

			// execute this batch of global events
			for(
				std::vector<GlobalEvent*>::iterator globIt = globals.begin();
				globIt!=globals.end();
				++globIt
			) process_global_event(**globIt);

			bFirst = false;
		} while(!globals.empty()); // if globals is empty, then we've processed through to the end of the buffer.
	}
	return buffer_;
}

void Player::setDriver(AudioDriver & driver) {
	std::cout << "psycle: core: player: setting audio driver to: " << driver.info().name() << '\n';

	if(driver_) driver_->Enable(false);

	if(!driver.Initialized()) {
		driver.Initialize(Work, this);
		std::cout << "psycle: core: player: audio driver initialized\n";
	}
	
	if(!driver.Configured()) {
		std::cout << "psycle: core: player: asking audio driver to configure itself\n";
		driver.Configure();
	}
	std::cout << "psycle: core: player: audio driver configured\n";
	
	if(driver.Enable(true)) {
		std::cout << "psycle: core: player: audio driver enabled: " << driver.info().name() << '\n';
		samples_per_second(driver.settings().samplesPerSec());
		driver_ = &driver;
	} else {
		std::cerr << "psycle: core: player: audio driver failed to enable. setting back previous driver\n";
		if(driver_) driver_->Enable(true);
	}
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
	switch(driver_->settings().channelMode()) {
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
	if(recording_) return;
	if(!song_ && !driver_) return;
	int channels(2);
	if(driver_->settings().channelMode() != 3) channels = 1;
	recording_ =( DDC_SUCCESS == _outputWaveFile.OpenForWrite(fileName().c_str(), driver_->settings().samplesPerSec(), driver_->settings().bitDepth(), channels));
	recording_with_dither_ = dodither;
	if (dodither) {
		dither.SetBitDepth(driver_->settings().bitDepth());
		dither.SetPdf(ditherpdf);
		dither.SetNoiseShaping(noiseshaping);
	}
}

void Player::stopRecording() {
	if(!recording_) return;
	_outputWaveFile.Close();
	recording_ = false;
}

}}
