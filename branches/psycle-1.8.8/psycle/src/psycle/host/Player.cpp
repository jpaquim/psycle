///\file
///\brief implementation file for psycle::host::Player.


#include "Player.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "Configuration.hpp"

#if !defined WINAMP_PLUGIN
	#include "MidiInput.hpp"
	#include "InputHandler.hpp"
#endif //!defined WINAMP_PLUGIN

#include "cpu_time_clock.hpp"
#include <universalis/os/thread_name.hpp>
#include <universalis/os/sched.hpp>
#include <universalis/os/aligned_alloc.hpp>
#include <seib-vsthost/CVSTHost.Seib.hpp> // Included to interact directly with the host.

namespace psycle
{
	namespace host
	{
		using namespace seib::vst;
		namespace {
			static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE bool this_thread_suspended_ = false;
		}
		Player::Player()
		{
			_playing = false;
			_playBlock = false;
			_recording = false;
			_clipboardrecording = false;
			_isWorking = false;
			_samplesRemaining=0;
			sampleCount=0;
			_lineCounter=0;
			_lineStop=-1;
			_loopSong=true;
			_patternjump=-1;
			_linejump=-1;
			_loop_count=0;
			_loop_line=0;
			measure_cpu_usage_=false;
			m_SampleRate=44100;
			SetBPM(125,4);
			for(int i=0;i<MAX_TRACKS;i++) {
				prevMachines[i]=255;
				prevInstrument[i]=255;
			}
			universalis::os::aligned_memory_alloc(16, _pBuffer, MAX_SAMPLES_WORKFN);
			start_threads();
		}

		Player::~Player()
		{
			stop_threads();
#if !defined WINAMP_PLUGIN
			if(_recording) _outputWaveFile.Close();
#endif //!defined WINAMP_PLUGIN
			universalis::os::aligned_memory_dealloc(_pBuffer);
		}

void Player::start_threads() {
	loggers::trace()("psycle: core: player: starting scheduler threads", UNIVERSALIS__COMPILER__LOCATION);
	if(!threads_.empty()) {
		loggers::trace()("psycle: core: player: scheduler threads are already running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}

	// normally, we would compute the scheduling plan here,
	// but we haven't yet implemented signals that notifies when connections are changed or nodes added/removed.
	// so it's actually done every time in the processing loop
	//compute_plan();

	stop_requested_ = suspend_requested_ = false;
	processed_node_count_ = suspended_ = 0;

	unsigned int thread_count = thread::hardware_concurrency();
	{ // thread count env var
		char const * const env(std::getenv("PSYCLE_THREADS"));
		if(env) {
			std::stringstream s;
			s << env;
			s >> thread_count;
		}
	}
	
	if(loggers::information()) {
		std::ostringstream s;
		s << "psycle: core: player: using " << thread_count << " threads";
		loggers::information()(s.str());
	}

	if(thread_count < 2) return; // don't create any thread, will use a single-threaded, recursive processing

	try {
		// start the scheduling threads
		for(std::size_t i(0); i < thread_count; ++i)
			threads_.push_back(new thread(boost::bind(&Player::thread_function, this, i)));
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


		void Player::Start(int pos, int lineStart, int lineStop, bool initialize)
		{
			_lineStop = lineStop;
			Start(pos, lineStart, initialize);
		}

		void Player::Start(int pos, int line, bool initialize)
		{
			CExclusiveLock lock(&Global::_pSong->semaphore, 2, true);
			if (initialize)
			{
				DoStop(); // This causes all machines to reset, and samplesperRow to init.				
				Work(256);
				((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->_clip = false;
			}
			_lineChanged = true;
			_lineCounter = line;
			_SPRChanged = false;
			_playPosition= pos;
			_playPattern = Global::_pSong->playOrder[_playPosition];
			if(pos != 0 || line != 0) {
				int songLength = 0;
				for (int i=0; i <Global::_pSong->playLength; i++)
				{
					int pattern = Global::_pSong->playOrder[i];
					// this should parse each line for ffxx commands if you want it to be truly accurate
					songLength += (Global::_pSong->patternLines[pattern] * 60/(tpb * bpm));
				}

				sampleCount=songLength*m_SampleRate;
			}
			else sampleCount=0;

			if (initialize)
			{
				_playTime = 0;
				_playTimem = 0;
			}
			_loop_count =0;
			_loop_line = 0;
			if (initialize)
			{
				SetBPM(Global::_pSong->BeatsPerMin(),Global::_pSong->LinesPerBeat());
				SampleRate(Global::pConfig->_pOutputDriver->GetSamplesPerSec());
				for(int i=0;i<MAX_TRACKS;i++) 
				{
					prevMachines[i] = 255;
					prevInstrument[i] = 255;
				}
				_playing = true;
			}
			CVSTHost::vstTimeInfo.flags |= kVstTransportPlaying;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
			ExecuteLine();
			_samplesRemaining = SamplesPerRow();
		}

		void Player::Stop(void)
		{
			CExclusiveLock lock(&Global::_pSong->semaphore, 2, true);
			DoStop();
		}
		void Player::DoStop(void)
		{
			if (_playing == true)
				_lineStop = -1;

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
			SetBPM(Global::_pSong->BeatsPerMin(),Global::_pSong->LinesPerBeat());
			SampleRate(Global::pConfig->_pOutputDriver->GetSamplesPerSec());
			CVSTHost::vstTimeInfo.flags &= ~kVstTransportPlaying;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
		}
		void Player::SetSampleRate(const int sampleRate) {
			CExclusiveLock lock(&Global::_pSong->semaphore, 2, true);
			SampleRate(sampleRate);
		}
		void Player::SampleRate(const int sampleRate)
		{
#if PSYCLE__CONFIGURATION__RMS_VUS
			helpers::dsp::numRMSSamples=sampleRate*0.05f;
#endif
			if(m_SampleRate != sampleRate)
			{
				m_SampleRate = sampleRate;
				RecalcSPR();
				CVSTHost::pHost->SetSampleRate(sampleRate);
				for(int i(0) ; i < MAX_MACHINES; ++i)
				{
					if(Global::_pSong->_pMachine[i]) Global::_pSong->_pMachine[i]->SetSampleRate(sampleRate);
				}
			}
		}
		void Player::SetBPM(int _bpm,int _tpb)
		{
			if ( _tpb != 0) tpb=_tpb;
			if ( _bpm != 0) bpm=_bpm;
			RecalcSPR();
			CVSTHost::vstTimeInfo.tempo = bpm;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
			CVSTHost::vstTimeInfo.flags |= kVstTempoValid;
			//\todo : Find out if we should notify the plugins of this change.
		}

void Player::suspend_and_compute_plan() {
	if(!_playing) {
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
	Song* pSong = Global::_pSong;
	for(int m(0); m < MAX_MACHINES; ++m) if(pSong->_pMachine[m]) {
		++graph_size_;
		Machine & n(*pSong->_pMachine[m]);
		// find the terminal nodes in the graph (nodes with no connected input ports)
		input_nodes_.clear(); n.sched_inputs(input_nodes_);
		if(input_nodes_.empty()) terminal_nodes_.push_back(&n);
	}

	// copy the initial processing queue
	nodes_queue_ = terminal_nodes_;
}

void Player::clear_plan() {
	nodes_queue_.clear();
	terminal_nodes_.clear();
}


		void Player::ExecuteLine(void)
		{
			ExecuteGlobalCommands();
			NotifyNewLine();
			ExecuteNotes();
		}
		// Initial Loop. Read new line and Interpret the Global commands.
		void Player::ExecuteGlobalCommands(void)
		{
			Song* pSong = Global::_pSong;
			_patternjump = -1;
			_linejump = -1;
			int mIndex = 0;
			unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);

			for(int track=0; track<pSong->SONGTRACKS; track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
				if(pEntry->_note < notecommands::tweak || pEntry->_note == 255) // If This isn't a tweak (twk/tws/mcm) then do
				{
					switch(pEntry->_cmd)
					{
					case PatternCmd::SET_TEMPO:
						if(pEntry->_parameter != 0)
						{	///\todo: implement the Tempo slide
							// SET_SONG_TEMPO=			20, // T0x Slide tempo down . T1x slide tempo up
							SetBPM(pEntry->_parameter);
						}
						break;
					case PatternCmd::EXTENDED:
						if(pEntry->_parameter != 0)
						{
							if ( (pEntry->_parameter&0xE0) == 0 ) // range from 0 to 1F for LinesPerBeat.
							{
								SetBPM(0,pEntry->_parameter);
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_BYPASS )
							{
								mIndex = pEntry->_mach;
								if ( mIndex < MAX_MACHINES && pSong->_pMachine[mIndex] && pSong->_pMachine[mIndex]->_mode == MACHMODE_FX )
								{
									if ( pEntry->_parameter&0x0F )
										pSong->_pMachine[mIndex]->Bypass(true);
									else
										pSong->_pMachine[mIndex]->Bypass(false);
								}
							}

							else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_MUTE )
							{
								mIndex = pEntry->_mach;
								if ( mIndex < MAX_MACHINES && pSong->_pMachine[mIndex] && pSong->_pMachine[mIndex]->_mode != MACHMODE_MASTER )
								{
									if ( pEntry->_parameter&0x0F )
										pSong->_pMachine[mIndex]->_mute = true;
									else
										pSong->_pMachine[mIndex]->_mute = false;
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
						if ( pEntry->_parameter < pSong->playLength ){
							_patternjump=pEntry->_parameter;
							_linejump=0;
						}
						break;
					case PatternCmd::BREAK_TO_LINE:
						if (_patternjump ==-1) 
						{
							_patternjump=(_playPosition+1>=pSong->playLength)?0:_playPosition+1;
						}
						if ( pEntry->_parameter >= pSong->patternLines[_patternjump])
						{
							_linejump = pSong->patternLines[_patternjump];
						} else { _linejump= pEntry->_parameter; }
						break;
					case PatternCmd::SET_VOLUME:
						if(pEntry->_mach == 255)
						{
							((Master*)(pSong->_pMachine[MASTER_INDEX]))->_outDry = pEntry->_parameter;
						}
						else 
						{
							int mIndex = pEntry->_mach;
							if(mIndex < MAX_MACHINES)
							{
								if(pSong->_pMachine[mIndex]) pSong->_pMachine[mIndex]->SetDestWireVolume(pSong,mIndex,pEntry->_inst, helpers::value_mapper::map_256_1(pEntry->_parameter));
							}
						}
						break;
					case  PatternCmd::SET_PANNING:
						mIndex = pEntry->_mach;
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
							if(pMachine)
							{
								// If the midi command uses a command less than 0x80, then interpret it as
								// send a tracker command to the specified track of the specified machine.
								if(pEntry->_note == notecommands::midicc && (pEntry->_inst < MAX_TRACKS || pEntry->_inst == 0xFF))
								{
									int voice(pEntry->_inst);
									// make a copy of the pattern entry, because we're going to modify it.
									PatternEntry entry(*pEntry);
									entry._note = 255;
									entry._inst = 255;
									// check for out of range voice values.
									if(voice < pSong->SONGTRACKS)
									{
										pMachine->Tick(voice, &entry);
									}
									else if(voice == 0xFF)
									{
										// special voice value which means we want to send the same command to all voices
										for(int voice(0) ; voice < pSong->SONGTRACKS ; ++voice)
										{
											pMachine->Tick(voice, &entry);
										}
									}
									else {}//Invalid index. do nothing.
									pMachine->TriggerDelayCounter[track] = 0;
									pMachine->ArpeggioCount[track] = 0;
								}
								// tweaks or midi cc
								else 
								{
									// classic tracking, use the track number as the channel/voice number
									pMachine->Tick(track, pEntry);
								}
							}
						}
					}
				}
			}
		}

			// Notify all machines that a new Tick() comes.
		void Player::NotifyNewLine(void)
		{
			Song* pSong = Global::_pSong;
			for(int tc=0; tc<MAX_MACHINES; tc++)
			{
				if(pSong->_pMachine[tc])
				{
					pSong->_pMachine[tc]->Tick();
					for(int c = 0; c < MAX_TRACKS; c++) pSong->_pMachine[tc]->TriggerDelay[c]._cmd = 0;
				}
			}

		}

		/// Final Loop. Read new line for notes to send to the Machines
		void Player::ExecuteNotes(void)
		{
			Song* pSong = Global::_pSong;
			unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);


			for(int track=0; track<pSong->SONGTRACKS; track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
				if(( !pSong->_trackMuted[track]) && (pEntry->_note < notecommands::tweak || pEntry->_note == 255)) // Is it not muted and is a note or command?
				{
					int mac = pEntry->_mach;
					if(mac != 255) prevMachines[track] = mac;
					else mac = prevMachines[track];
					if( mac != 255 && (pEntry->_note != 255 || pEntry->_cmd != 0 || pEntry->_parameter != 0) ) // is there a machine number and it is either a note or a command?
					{
						int ins = pEntry->_inst;
						if(pEntry->_inst != 255) prevInstrument[track] = pEntry->_inst;
						else ins = prevInstrument[track];

						if(mac < MAX_MACHINES) //looks like a valid machine index?
						{
							// make a copy of the pattern entry, because we're going to modify it.
							PatternEntry entry(*pEntry);
							entry._inst = ins;

							Machine *pMachine = pSong->_pMachine[mac];
							if(pMachine && !(pMachine->_mute)) // Does this machine really exist and is not muted?
							{
								if(entry._cmd == PatternCmd::NOTE_DELAY)
								{
									// delay
									memcpy(&pMachine->TriggerDelay[track], &entry, sizeof(PatternEntry));
									pMachine->TriggerDelayCounter[track] = ((entry._parameter+1)*SamplesPerRow())/256;
								}
								else if(entry._cmd == PatternCmd::RETRIGGER)
								{
									// retrigger
									memcpy(&pMachine->TriggerDelay[track], &entry, sizeof(PatternEntry));
									pMachine->RetriggerRate[track] = (entry._parameter+1);
									pMachine->TriggerDelayCounter[track] = 0;
								}
								else if(entry._cmd == PatternCmd::RETR_CONT)
								{
									// retrigger continue
									memcpy(&pMachine->TriggerDelay[track], &entry, sizeof(PatternEntry));
									if(entry._parameter&0xf0) pMachine->RetriggerRate[track] = (entry._parameter&0xf0);
								}
								else if (entry._cmd == PatternCmd::ARPEGGIO)
								{
									// arpeggio
									//\todo : Add Memory.
									//\todo : This won't work... What about sampler's NNA's?
									if (entry._parameter)
									{
										memcpy(&pMachine->TriggerDelay[track], &entry, sizeof(PatternEntry));
										pMachine->ArpeggioCount[track] = 1;
									}
									pMachine->RetriggerRate[track] = SamplesPerRow()*tpb/24;
								}
								else
								{
									pMachine->TriggerDelay[track]._cmd = 0;
									pMachine->Tick(track, &entry);
									pMachine->TriggerDelayCounter[track] = 0;
									pMachine->ArpeggioCount[track] = 0;
								}
							}
						}
					}
				}
			}
		}	


		void Player::AdvancePosition()
		{
			Song* pSong = Global::_pSong;
			if ( _patternjump!=-1 ) _playPosition= _patternjump;
			if ( _SPRChanged ) { RecalcSPR(); _SPRChanged = true; }
			if ( _linejump!=-1 ) _lineCounter=_linejump;
			else _lineCounter++;
			_playTime += 60 / float (bpm * tpb);
			if(_playTime>60)
			{
				_playTime-=60;
				_playTimem++;
			}
			if(_lineCounter >= pSong->patternLines[_playPattern] || _lineCounter==_lineStop)
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
			_lineChanged = true;
		}

void Player::thread_function(std::size_t thread_number) {
	if(loggers::trace()) {
		scoped_lock lock(mutex_);
		std::ostringstream s;
		s << "psycle: core: player: scheduler thread #" << thread_number << " started";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	// set thread name
	universalis::os::thread_name thread_name;
	{
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << " Engine thread #" << thread_number;
		thread_name.set(s.str());
	}

	// install cpu/os exception handler/translator
	universalis::cpu::exceptions::install_handler_in_thread();

	HANDLE hTask = NULL;
	{ // set thread priority and cpu affinity
		using universalis::os::exceptions::operation_not_permitted;
		using universalis::os::sched::thread;
		thread t;

		// set thread priority
		try {
			t.priority(thread::priorities::highest);
			// Ask MMCSS to temporarily boost the thread priority
			// to reduce glitches while the low-latency stream plays.
			if(Is_Vista_or_Later()) 
			{
				DWORD taskIndex = 0;
				hTask = Global::pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
			}
		} catch(operation_not_permitted e) {
			if(loggers::warning()) {
				std::ostringstream s; s << "no permission to set thread priority: " << e.what();
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}

		// set thread cpu affinity
		try {
			thread::affinity_mask_type const af(t.affinity_mask());
			if(af.active_count()) {
				unsigned int rotated = 0, cpu_index = 0;
				while(!af(cpu_index) || rotated++ != thread_number) cpu_index = (cpu_index + 1) % af.size();
				thread::affinity_mask_type new_af; new_af(cpu_index, true); t.affinity_mask(new_af);
			}
		} catch(operation_not_permitted e) {
			if(loggers::warning()) {
				std::ostringstream s; s << "no permission to set thread cpu affinity: " << e.what();
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	}

	try {
		try {
			process_loop(thread_number);
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
			s << "exception: " << universalis::compiler::exceptions::ellipsis_desc();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	}
	if (hTask != NULL) { Global::pAvRevertMmThreadCharacteristics(hTask); }

	if(loggers::trace()) {
		scoped_lock lock(mutex_);
		std::ostringstream s;
		s << "psycle: core: player: scheduler thread #" << thread_number << " terminated";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void Player::process_loop(std::size_t thread_number) throw(std::exception) {
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
					main_condition_.notify_all();
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

		bool const done(node.sched_process(samples_to_process_, measure_cpu_usage_));

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
				output_nodes_.clear(); node.sched_outputs(output_nodes_);

				for(Machine::sched_deps::const_iterator i(output_nodes_.begin()), e(output_nodes_.end()); i != e; ++i) {
					Machine & output_node(*const_cast<Machine*>(*i));
					bool output_node_ready(true);
					// iterate over all the inputs connected to our output
					input_nodes_.clear(); output_node.sched_inputs(input_nodes_);
					for(Machine::sched_deps::const_iterator i(input_nodes_.begin()), e(input_nodes_.end()); i != e; ++i) {
						const Machine & input_node(**i);
						if(&input_node == &node) continue;
						if(!input_node.sched_processed_) {
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
		}
		switch(notify) {
			case -1: main_condition_.notify_all(); break; // wake up the main processing loop
			case 0: break; // no successor ready
			case 1: break; // If there's only one successor ready, we don't notify since it can be processed in the same thread.
			case 2: condition_.notify_one(); break; // notify one thread that we added nodes to the queue
			default: condition_.notify_all(); // notify all threads that we added nodes to the queue
		}
	}
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

		float * Player::Work(void* context, int numSamples)
		{
			CSingleLock crit(&Global::_pSong->semaphore, FALSE);
			Player* pThis = (Player*)context;
			//Avoid possible deadlocks
			if(crit.Lock(100)) {
				pThis->Work(numSamples);
			}
			else {
				dsp::Clear(pThis->_pBuffer,numSamples*2);
			}
			return pThis->_pBuffer;
		}
		float * Player::Work(int numSamples)
		{
			int amount;
			Song* pSong = Global::_pSong;
			Master::_pMasterSamples = _pBuffer;
			nanoseconds const t0(cpu_time_clock());
			sampleOffset = 0;
			do
			{
				if(numSamples > STREAM_SIZE) amount = STREAM_SIZE; else amount = numSamples;
				// Tick handler function
				if(amount >= _samplesRemaining) amount = _samplesRemaining;
				// Song play
				if((_samplesRemaining <=0))
				{
					if (_playing)
					{
						// Advance position in the sequencer
						AdvancePosition();
						// Global commands are executed first so that the values for BPM and alike
						// are up-to-date when "NotifyNewLine()" is called.
						ExecuteGlobalCommands();
						NotifyNewLine();
						ExecuteNotes();
					}
					else
					{
						NotifyNewLine();
					}
					_samplesRemaining = SamplesPerRow();
				}
				// Processing plant
				if(amount > 0)
				{
					// Reset all machines
					for(int c=0; c<MAX_MACHINES; c++)
					{
						//Note: This should be scheduled if possible too. Also note that it increments
						// the routing_accumulator, which is is divided by numthreads in the infodlg.
						if(pSong->_pMachine[c]) pSong->_pMachine[c]->PreWork(amount, true, measure_cpu_usage_);
					}

					//\todo: Sampler::DoPreviews( amount );
					pSong->DoPreviews( amount );

					CVSTHost::vstTimeInfo.samplePos = sampleCount;

#if !defined WINAMP_PLUGIN
					// Inject Midi input data
					Global::midi().InjectMIDI( amount );
					if(threads_.empty()){ // single-threaded, recursive processing
						pSong->_pMachine[MASTER_INDEX]->recursive_process(amount, measure_cpu_usage_);
					} else { // multi-threaded scheduling
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
					sampleCount += amount;

					if((_playing) && (_recording))
					{
						float* pL(pSong->_pMachine[MASTER_INDEX]->_pSamplesL);
						float* pR(pSong->_pMachine[MASTER_INDEX]->_pSamplesR);
						if(_dodither)
						{
							dither.Process(pL, amount);
							dither.Process(pR, amount);
						}
						int i;
						if ( _clipboardrecording)
						{
							switch(Global::pConfig->_pOutputDriver->GetChannelMode())
							{
							case mono_mix: // mono mix
								for(i=0; i<amount; i++)
								{
									if (!ClipboardWriteMono(((*pL++)+(*pR++))/2)) StopRecording(false);
								}
								break;
							case mono_left: // mono L
								for(i=0; i<amount; i++)
								{
									if (!ClipboardWriteMono(*pL++)) StopRecording(false);
								}
								break;
							case mono_right: // mono R
								for(i=0; i<amount; i++)
								{
									if (!ClipboardWriteMono(*pR++)) StopRecording(false);
								}
								break;
							default: // stereo
								for(i=0; i<amount; i++)
								{
									if (!ClipboardWriteStereo(*pL++,*pR++)) StopRecording(false);
								}
								break;
							}						}
						else switch(Global::pConfig->_pOutputDriver->GetChannelMode())
						{
						case mono_mix: // mono mix
							for(i=0; i<amount; i++)
							{
								//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
								if(_outputWaveFile.WriteMonoSample(((*pL++)+(*pR++))/2) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						case mono_left: // mono L
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample((*pL++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						case mono_right: // mono R
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample((*pR++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						default: // stereo
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteStereoSample((*pL++),(*pR++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						}
					}
#else
					if(threads_.empty()){ // single-threaded, recursive processing
						pSong->_pMachine[MASTER_INDEX]->recursive_process(amount, _measure_cpu_usage_);
					} else { // multi-threaded scheduling
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
					sampleCount += amount;
#endif //!defined WINAMP_PLUGIN

					Master::_pMasterSamples += amount * 2;
					numSamples -= amount;
				}
				 _samplesRemaining -= amount;
				 sampleOffset += amount;
				 CVSTHost::vstTimeInfo.flags &= ~kVstTransportChanged;
			} while(numSamples>0);
			nanoseconds const t1(cpu_time_clock());
			pSong->accumulate_processing_time(t1 - t0);
			return _pBuffer;
		}
		bool Player::ClipboardWriteMono(float sample)
		{
			// right now the implementation does not support these two being different
			if (Global::pConfig->_pOutputDriver->GetSampleValidBits() !=
				Global::pConfig->_pOutputDriver->GetSampleBits()) {
					return false;
			}
			int *length = reinterpret_cast<int*>((*pClipboardmem)[0]);
			int pos = *length%1000000;
			int endpos = pos;
			
			switch( Global::pConfig->_pOutputDriver->GetSampleBits())
			{
			case 8: endpos+=1; break;
			case 16: endpos+=2; break;
			case 24: endpos+=3; break;
			case 32: endpos+=4; break;
			}

			int d(0);
			if(sample > 32767.0f) sample = 32767.0f;
			else if(sample < -32768.0f) sample = -32768.0f;
			switch( Global::pConfig->_pOutputDriver->GetSampleBits())
			{
			case 8:
				d = int(sample/256.0f);
				d += 128;
				(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
				*length+=1;
				break;
			case 16:
				d = static_cast<int>(sample);
				(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
				(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
				*length+=2;
				break;
			case 24:
				d = int(sample * 256.0f);
				if ( endpos < 1000000 )
				{
					(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
					(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
					(*pClipboardmem)[clipbufferindex][pos+2]=*(reinterpret_cast<char*>(&d)+2);
					*length+=3;
				}
				break;
			case 32:
				d = int(sample * 65536.0f);
				(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
				(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
				(*pClipboardmem)[clipbufferindex][pos+2]=*(reinterpret_cast<char*>(&d)+2);
				(*pClipboardmem)[clipbufferindex][pos+3]=*(reinterpret_cast<char*>(&d)+3);
				*length+=4;
				break;
			default:
				break;
			}

			if ( endpos >= 1000000 )
			{
				clipbufferindex++;
				char *newbuf = new char[1000000];
				if (!newbuf) return false;
				pClipboardmem->push_back(newbuf);
				// bitdepth == 24 is the only "odd" value, since it uses 3 chars each, nondivisible by 1000000
				if ( Global::pConfig->_pOutputDriver->GetSampleBits() == 24)
				{
					clipbufferindex--;
					(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
					if ( ++pos = 1000000) { pos = 0; clipbufferindex++; }
					(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
					if ( ++pos = 1000000) { pos = 0; clipbufferindex++; }
					(*pClipboardmem)[clipbufferindex][pos+2]=*(reinterpret_cast<char*>(&d)+2);
					if ( ++pos = 1000000) { pos = 0;  clipbufferindex++; }
					*length+=3;
				}
			}
			return true;
		}
		bool Player::ClipboardWriteStereo(float left, float right)
		{
			if (!ClipboardWriteMono(left)) return false;
			return ClipboardWriteMono(right);
		}

		void Player::StartRecording(std::string psFilename, int bitdepth, int samplerate, channel_mode channelmode, bool isFloat, bool dodither, int ditherpdf, int noiseshape, std::vector<char*> *clipboardmem)
		{
#if !defined WINAMP_PLUGIN
			if(!_recording)
			{
				if(samplerate > 0) SampleRate(samplerate);
				_dodither=dodither;
				if(dodither)
				{
					if(bitdepth>0)	dither.SetBitDepth(bitdepth);
					else			dither.SetBitDepth(Global::pConfig->_pOutputDriver->GetSampleValidBits());
					dither.SetPdf((helpers::dsp::Dither::Pdf::type)ditherpdf);
					dither.SetNoiseShaping((helpers::dsp::Dither::NoiseShape::type)noiseshape);
				}
				int channels = 2;
				if(channelmode != stereo) channels = 1;
				Stop();
				if (!psFilename.empty())
				{
					if(_outputWaveFile.OpenForWrite(psFilename.c_str(), samplerate, bitdepth, channels, isFloat) == DDC_SUCCESS)
						_recording = true;
					else
					{
						_recording = true;
						StopRecording(false);
					}
				}
				else
				{
					char *newbuf = new char[1000000];
					if ( newbuf)
					{
						pClipboardmem = clipboardmem;
						pClipboardmem->push_back(newbuf);
						_clipboardrecording = true;
						clipbufferindex = 1;
						_recording = true;
					}
					else {
						_recording = true;
						StopRecording(false);
					}
				}
			}
#endif //!defined WINAMP_PLUGIN
		}

		void Player::StopRecording(bool bOk)
		{
#if !defined WINAMP_PLUGIN
			if(_recording)
			{
				SampleRate(Global::pConfig->_pOutputDriver->GetSamplesPerSec());
				if (!_clipboardrecording)
					_outputWaveFile.Close();
				_recording = false;
				_clipboardrecording =false;
				if(!bOk)
				{
					MessageBox(0, "Wav recording failed.", "ERROR", MB_OK);
				}
			}
#endif //!defined WINAMP_PLUGIN
		}
	}
}
