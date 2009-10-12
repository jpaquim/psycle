// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::alsa
#include <psycle/detail/project.private.hpp>
#include "alsa.hpp"
#include <diversalis/processor.hpp>
#include <universalis/processor/exception.hpp>
#include <universalis/operating_system/thread_name.hpp>
#include <universalis/operating_system/exceptions/code_description.hpp>
#include <poll.h>
#include <alloca.h> // beware: this is not in posix, but this is available on *bsd and linux.
#include <cstdio>
namespace psycle { namespace plugins { namespace outputs {

using engine::exceptions::runtime_error;

namespace {
	///\todo parametrable
	typedef std::int16_t output_sample_type;
	
	std::string open_mode_description(int open_mode) {
		std::string result;
		switch(open_mode) {
			case SND_PCM_ASYNC   : result = "asynchronous"; break;
			case SND_PCM_NONBLOCK: result = "non-blocking"; break;
			default              : result = "blocking";
		}
		return result;
	}
}

PSYCLE__PLUGINS__NODE_INSTANTIATOR(alsa)

alsa::alsa(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name) throw(engine::exception)
:
	resource(plugin_library_reference, graph, name),
	pcm_(),
	output_(),
	intermediate_buffer_(),
	//areas_(),
	thread_()
{
	engine::ports::inputs::single::create_on_heap(*this, "in");
	engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
}

void alsa::do_open() throw(engine::exception) {
	resource::do_open();
	
	int error;

	/**************************************************************/
	// attach to std output
	
	if(0 > (error = ::snd_output_stdio_attach(&output_, ::stdout, 0))) {
		std::ostringstream s; s << "could not attach to stdio output: " << ::snd_strerror(error);
		loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	/**************************************************************/
	// open the pcm device (device name, stream direction, open mode)

	std::string pcm_device_name("default"); ///\todo parametrable
	{ // get device name from env
		char const * const env(std::getenv("PSYCLE__PLUGINS__OUTPUTS__ALSA__PCM_DEVICE"));
		if(env) pcm_device_name = env;
	}

	::snd_pcm_stream_t const direction(::SND_PCM_STREAM_PLAYBACK);

	int const open_mode(SND_PCM_NONBLOCK); // 0: block, SND_PCM_NONBLOCK: non-block, SND_PCM_ASYNC: asynchronous
	// note: with SND_PCM_NONBLOCK, ::snd_pcm_open can return -EBUSY if the device is not free.

	if(0 > (error = ::snd_pcm_open(&pcm_, pcm_device_name.c_str(), direction, open_mode))) {
		std::ostringstream s;
		s <<
			"could not open pcm device: "
			"name: " << pcm_device_name << ", "
			"stream direction: " << ::snd_pcm_stream_name(direction) << ", "
			"open mode: " << open_mode_description(open_mode) <<
			": " << ::snd_strerror(error);
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(loggers::information()) {
		{ // display type
			std::ostringstream s;
			s << "alsa pcm device: type: " << ::snd_pcm_type_name(::snd_pcm_type(pcm_)); 
			loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		{ // display info
			// allocate info on the stack
			::snd_pcm_info_t * info; snd_pcm_info_alloca(&info);
			// get static information on pcm device
			if(0 > (error = ::snd_pcm_info(pcm_, info))) {
				std::ostringstream s;
				s <<
					"could not get static info for opened device named: " << pcm_device_name <<
					": " << ::snd_strerror(error);
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			} else if(loggers::information()) {
				std::ostringstream s;
				s <<
					"alsa pcm device: info:\n"
					"\t" "device name: " << ::snd_pcm_info_get_name(info) << "\n"
					"\t" "subdevice name: " << ::snd_pcm_info_get_subdevice_name(info) << "\n"
					"\t" "id: " << ::snd_pcm_info_get_id(info) << "\n"
					"\t" "card: " << ::snd_pcm_info_get_card(info);
				loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	}

	/**************************************************************/
	// set the work mode

	bool const work_mode_is_non_blocking(false);

	if(0 > (error = ::snd_pcm_nonblock(pcm_, work_mode_is_non_blocking))) {
		std::ostringstream s;
		s <<
			"could not set work mode to: " <<
			(work_mode_is_non_blocking ? "non-blocking" : "blocking") <<
			": " << ::snd_strerror(error);
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	/**************************************************************/
	// setup hardware parameters (resampling, access method, sample format, channels, rate, periods, period and buffer size)
	
	try {
		// allocate hardware parameters on the stack
		::snd_pcm_hw_params_t * pcm_hw_params; snd_pcm_hw_params_alloca(&pcm_hw_params);
		
		// make the configuration space full
		if(0 > (error = ::snd_pcm_hw_params_any(pcm_, pcm_hw_params))) {
			std::ostringstream s; s << "could not initialize hardware parameters: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}

		/**************************************************************/
		// set the resampling setting
		
		bool const allow_resample(true);

		#if SND_LIB_VERSION >= 0x10009 // 1.0.9
			// If allow_resample is false, we will only allow sample format conversion,
			// by disabling the resampling feature in the "plug" plugin, hence restricting
			// a configuration space to contain only real (or virtual, like pulse audio) hardware rates.
			if(0 > (error = ::snd_pcm_hw_params_set_rate_resample(pcm_, pcm_hw_params, allow_resample))) {
				std::ostringstream s;
				s << "could not set resampling setting to: " << (allow_resample ? "allowed" : "disallowed") << ": " << ::snd_strerror(error);
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		#endif
		
		/**************************************************************/
		// set the access method
		
		::snd_pcm_access_t const access(::SND_PCM_ACCESS_RW_INTERLEAVED);
		
		if(0 > (error = ::snd_pcm_hw_params_set_access(pcm_, pcm_hw_params, access))) {
			std::ostringstream s;
			s << "could not set access method to: " << ::snd_pcm_access_name(access) << ": " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		
		/**************************************************************/
		// set the sample format

		::snd_pcm_format_t format(::SND_PCM_FORMAT_S16); ///\todo parametrable
		{ // get format from env
			char const * const env(std::getenv("PSYCLE__PLUGINS__OUTPUTS__ALSA__FORMAT"));
			if(env) format = ::snd_pcm_format_value(env);
		}
		
		if(0 > (error = ::snd_pcm_hw_params_set_format(pcm_, pcm_hw_params, format))) {
			std::ostringstream s;
			s <<
				"could not set sample format to: " << ::snd_pcm_format_description(format) <<
				": " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		
		// get the sample width
		bits_per_channel_sample_ = ::snd_pcm_format_width(format);

		/**************************************************************/
		// set the channel count

		if(0 > (error = ::snd_pcm_hw_params_set_channels(pcm_, pcm_hw_params, in_port().channels()))) {
			std::ostringstream s;
			s <<
				"could not set channel count to: " << in_port().channels() <<
				": " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		
		/**************************************************************/
		// set the sample rate
		{ 
			unsigned int rate(::lround(in_port().events_per_second()));
			unsigned int rate_accepted(rate);
			int direction(0);
			if(0 > (error = ::snd_pcm_hw_params_set_rate_near(pcm_, pcm_hw_params, &rate_accepted, &direction))) {
				std::ostringstream s; s << "could not set sample rate to: " << rate << "Hz: " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(rate_accepted != rate) {
				if(loggers::information()) {
					std::ostringstream s;
					s <<
						"sample rate: "
						"requested: " << rate << "Hz, "
						"accepted: " << rate_accepted << "Hz";
					loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				in_port().events_per_second(rate_accepted);
			}
		}
		
		/**************************************************************/
		// set the period size and the number of periods

		unsigned int periods(4); ///\todo parametrable
		period_frames_ = 1024; ///\todo parametrable
		
		#if 1
			{ // set the period size in frames
				::snd_pcm_uframes_t period_frames_accepted(period_frames_);
				int direction(0);
				if(0 > (error = ::snd_pcm_hw_params_set_period_size_near(pcm_, pcm_hw_params, &period_frames_accepted, &direction))) {
					std::ostringstream s;
					s << "could not set period size to: " << period_frames_ << " samples: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(period_frames_accepted != period_frames_) {
					std::ostringstream s;
					s <<
						"period size: "
						"requested: " << period_frames_ << " samples, "
						"accepted: " << period_frames_accepted << " samples";
					loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					period_frames_ = period_frames_accepted;
				}
			}
			{ // set the period count
				unsigned int periods_accepted(periods);
				int direction(0);
				if(0 > (error = ::snd_pcm_hw_params_set_periods_near(pcm_, pcm_hw_params, &periods_accepted, &direction))) {
					std::ostringstream s;
					s << "could not set period count to: " << periods << ": " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(periods_accepted != periods) {
					std::ostringstream s;
					s <<
						"periods: "
						"requested: " << periods << ", "
						"accepted: " << periods_accepted;
					loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					periods = periods_accepted;
				}
			}
		#else
			{ // set the period time in microseconds
				unsigned int period_microseconds_accepted(period_microseconds);
				int const direction(0);
				if(0 > (error = ::snd_pcm_hw_params_set_period_time_near(pcm_, pcm_hw_params, &period_microseconds_accepted, direction))) {
					std::ostringstream s;
					s <<
						"could not set period time to: " << period_microseconds <<
						": " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(period_microseconds_accepted != period_microseconds) {
					std::ostringstream s;
					s <<
						"period time: "
						"requested: " << period_microseconds << "us, "
						"accepted: " << period_microseconds_accepted << "us";
					loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					period_microseconds = period_microseconds_accepted;
				}
			}
			{ // get the period size in frames
				int direction(0);
				if(0 > (error = ::snd_pcm_hw_params_get_period_size(params, &period_frames_, &direction))) {
					std::ostringstream s; s << "could not get period size: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		#endif

		/**************************************************************/
		// set the buffer size

		#if 1
			{ // set the buffer size in frames
				buffer_frames_ = period_frames_ * periods;
				::snd_pcm_uframes_t buffer_frames_accepted(buffer_frames_);
				if(0 > (error = ::snd_pcm_hw_params_set_buffer_size_near(pcm_, pcm_hw_params, &buffer_frames_accepted))) {
					std::ostringstream s;
					s << "could not set buffer size to: " << buffer_frames_ << " samples: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(buffer_frames_accepted != buffer_frames_) {
					std::ostringstream s;
					s <<
						"buffer size: "
						"requested: " << buffer_frames_ << " samples, "
						"accepted: " << buffer_frames_accepted << " samples";
					loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					buffer_frames_ = buffer_frames_accepted;
				}
			}
		#else
			{ // set the buffer time in microseconds
				unsigned int buffer_microseconds_accepted(buffer_microseconds);
				int direction(0);
				if(0 > (error = ::snd_pcm_hw_params_set_buffer_time_near(pcm_, pcm_hw_params, &buffer_microseconds_accepted, &direction))) {
					std::ostringstream s;
					s << "could not set buffer time to: " << buffer_microseconds << ": " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(buffer_microseconds_accepted != buffer_microseconds) {
					std::ostringstream s;
					s <<
						"buffer time: "
						"requested: " << buffer_microseconds << "us, "
						"accepted: " << buffer_microseconds_accepted << "us";
					loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					buffer_microseconds = buffer_microseconds_accepted;
				}
			}
			{ // get the buffer size in frames
				if(0 > (error = ::snd_pcm_hw_params_get_buffer_size(params, &buffer_frames_))) {
					std::ostringstream s; s << "could not get buffer size: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
		#endif

		/**************************************************************/
		// apply hw parameter settings to the pcm device and prepare it

		if(0 > (error = ::snd_pcm_hw_params(pcm_, pcm_hw_params))) {
			std::ostringstream s; s << "could not set hardware parameters: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		if(loggers::information()) {
			std::ostringstream s; s << "alsa pcm device: name: " << pcm_device_name << ": setup successful";
			loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		// At this point ::snd_pcm_prepare(pcm_) was called automatically,
		// and hence ::snd_pcm_state(pcm_) was brought to ::SND_PCM_STATE_PREPARED state.
		
		/**************************************************************/
		// allocate the intermediate buffer
		{ 
			// note: period_frames may be different from parent().events_per_buffer()
			std::size_t const bytes(parent().events_per_buffer() * in_port().channels() * bits_per_channel_sample_ / std::numeric_limits<unsigned char>::digits);
			if(!(intermediate_buffer_ = new char[bytes])) {
				std::ostringstream s; s << "not enough memory to allocate " << bytes << " bytes on heap";
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
		#if 1
			// nothing needed for write method
		#else
			// setup areas within buffer
			if(!(areas_ = new ::snd_pcm_channel_area_t[in_port().channels()])) {
				std::ostringstream s; s << "not enough memory to allocate " << (in_port().channels() * sizeof *areas_) << " bytes on heap";
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			for(unsigned int i(0); i < in_port().channels(); ++i) {
				areas_[i].addr = buffer_;
				areas_[i].first = i * bits_per_channel_sample_;
				areas_[i].step = in_port().channels() * bits_per_channel_sample_;
			}
		#endif
	} catch(...) {
		delete[] intermediate_buffer_; intermediate_buffer_ = 0;
		#if 1
			// nothing needed for write method
		#else
			delete[] areas_; areas_ = 0;
		#endif
		::snd_pcm_close(pcm_); pcm_ = 0; // or ::snd_pcm_free(pcm_); ?
		throw;
	}
}

bool alsa::opened() const {
	return pcm_;
}

void alsa::do_start() throw(engine::exception) {
	resource::do_start();

	// allocate software parameters on the stack
	::snd_pcm_sw_params_t * pcm_sw_params; snd_pcm_sw_params_alloca(&pcm_sw_params);

	int error;

	// get the current sw_params
	if(0 > (error = ::snd_pcm_sw_params_current(pcm_, pcm_sw_params))) {
		std::ostringstream s; s << "could not initialize software parameters: " << ::snd_strerror(error);
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// wake up the application when at least xxx samples can be written to the ring buffer
	if(0 > (error = ::snd_pcm_sw_params_set_avail_min(pcm_, pcm_sw_params, period_frames_))) {
		std::ostringstream s; s << "could not set available minimum: " << ::snd_strerror(error);
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// start the transfer when at least xxx samples have been written to the ring buffer
	if(0 > (error = ::snd_pcm_sw_params_set_start_threshold(pcm_, pcm_sw_params, buffer_frames_ / 2))) {
		std::ostringstream s; s << "could not set start threshold: " << ::snd_strerror(error);
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	#if SND_LIB_VERSION >= 0x10010 // 1.0.16
		// snd_pcm_sw_params_set_xfer_align() is deprecated, alignment is always 1
	#else
		// align all transfers to 1 sample
		if (0 > (error = ::snd_pcm_sw_params_set_xfer_align(pcm_, pcm_sw_params, 1))) {
			std::ostringstream s; s << "could not set transfer alignment: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	#endif
	// write the parameters to the playback device
	if(0 > (error = ::snd_pcm_sw_params(pcm_, pcm_sw_params))) {
		std::ostringstream s; s << "could not set software parameters: " << ::snd_strerror(error);
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	// dump information
	::snd_pcm_dump(pcm_, output_);

	// not really useful
	if(0 > (error = ::snd_pcm_prepare(pcm_))) {
		std::ostringstream s; s << "could not prepare device for use: " << ::snd_strerror(error);
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	
	stop_requested_ = false;
	// start the poller thread
	thread_ = new std::thread(boost::bind(&alsa::thread_function, this));
}

bool alsa::started() const {
	if(!opened()) return false;
	#if 1
		return thread_;
	#else
		::snd_pcm_state_t const state(::snd_pcm_state(pcm_));
		return state >= ::SND_PCM_STATE_PREPARED; // this is a bit meaningless without a thread
	#endif
}

void alsa::thread_function() {
	if(loggers::information()) loggers::information()("poller thread started", UNIVERSALIS__COMPILER__LOCATION);

	// set thread name
	universalis::operating_system::thread_name thread_name(universalis::compiler::typenameof(*this) + "#" + qualified_name());

	// install cpu/os exception handler/translator
	universalis::processor::exception::install_handler_in_thread();
	
	try {
		try {
			poll_loop();
		} catch(...) {
			loggers::exception()("caught exception in poller thread", UNIVERSALIS__COMPILER__LOCATION);
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
	loggers::information()("poller thread " + qualified_name() + " terminated", UNIVERSALIS__COMPILER__LOCATION);
}

void alsa::poll_loop() throw(engine::exception) {
	// get number of file descriptors to poll
	::nfds_t nfds;
	if(0 >= (nfds = ::snd_pcm_poll_descriptors_count(pcm_)))
		throw engine::exceptions::runtime_error("invalid poll descriptor count", UNIVERSALIS__COMPILER__LOCATION);
	// allocate memory for file descriptors on the stack
	::pollfd * fds(reinterpret_cast< ::pollfd*>(alloca(nfds * sizeof *fds)));
	int error;
	// get file descriptors to poll
	if(0 > (error = ::snd_pcm_poll_descriptors(pcm_, fds, nfds))) {
		std::ostringstream s; s << "could not get poll descriptors: " << ::snd_strerror(error);
		throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	while(true) {
		bool loop(false);
		int const timeout_ms(1000);
		// poll() returns the number of descriptors that are ready for I/O, or -1 if an error occurred.
		// If the time limit expires, poll() returns 0.
		// If poll() returns with an error, including one due to an interrupted call,
		// the fds array will be unmodified and the global variable errno will be set to indicate the error.
		if(0 > (error = ::poll(fds, nfds, timeout_ms)))
			if(errno == EAGAIN || errno == EINTR) loop = true;
			else throw engine::exceptions::runtime_error(universalis::operating_system::exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION);
		if(!error) {
			if(loggers::warning()) loggers::warning()("timed out", UNIVERSALIS__COMPILER__LOCATION);
			loop = true;
		}
		{ scoped_lock lock(mutex_);
			if(stop_requested_) return;
		}
		if(loop) continue;
		unsigned short revents;
		if(0 > (error = ::snd_pcm_poll_descriptors_revents(pcm_, fds, nfds, &revents))) {
			std::ostringstream s; s << "could not poll file descriptors: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		if(revents & POLLERR) throw engine::exceptions::runtime_error("error condition in poll", UNIVERSALIS__COMPILER__LOCATION);
		if(loggers::warning() && revents & POLLNVAL) loggers::warning()("invalid file descriptor in poll (could have been asynchronously closed)", UNIVERSALIS__COMPILER__LOCATION);
		if(revents & POLLOUT) {
			{ scoped_lock lock(mutex_);
				while(io_ready() && !stop_requested_) condition_.wait(lock);
			}
			if(stop_requested_) return; 
			{ // copy the intermediate buffer to the alsa buffer
				unsigned int const channels(in_port().channels());
				::snd_pcm_uframes_t const samples_per_buffer(parent().events_per_buffer());
	
				output_sample_type * in(reinterpret_cast<output_sample_type*>(intermediate_buffer_));
	
				::snd_pcm_uframes_t frames_to_write(samples_per_buffer);
				do {
					///\todo support for non-interleaved channels
					::snd_pcm_sframes_t const frames_written(::snd_pcm_writei(pcm_, in, frames_to_write));
					int error(frames_written);
					if(0 > error) switch(error) {
						case -EAGAIN: // for non-blocking mode
							if(false && loggers::trace()) {
								std::ostringstream s; s << "again: " << ::snd_strerror(error);
								loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							std::this_thread::yield();
							{ scoped_lock lock(mutex_);
								if(stop_requested_) return;
							}
							continue;
						case -EPIPE: // an underrun occured
							if(loggers::warning()) {
								std::ostringstream s; s << "underrun: " << ::snd_strerror(error);
								loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
							}
							if(0 > (error = ::snd_pcm_prepare(pcm_))) {
								if(loggers::warning()) {
									std::ostringstream s; s << "could not prepare device for use: " << ::snd_strerror(error);
									loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
								}
								goto next; // skip rest of period
							}
							break;
						case -ESTRPIPE: // a suspend event occurred (stream is suspended and waiting for an application recovery)
							// wait until device is resumed
							while(-EAGAIN == (error = ::snd_pcm_resume(pcm_))) {
								// resume cannot be proceeded immediately (the device is still suspended)
								if(loggers::trace()) {
									std::ostringstream s; s << "waiting for device to resume: " << ::snd_strerror(error);
									loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
								}
								// sleep a bit before retrying to resume
								std::this_thread::sleep(std::seconds(1));
								{ scoped_lock lock(mutex_);
									if(stop_requested_) return;
								}
							}
							if(0 > error) switch(error) {
								case -ENOSYS: // the device is no longer suspended, but it does not fully support resuming
									if(0 > (error = ::snd_pcm_prepare(pcm_))) {
										if(loggers::warning()) {
											std::ostringstream s; s << "could not prepare device for resume: " << ::snd_strerror(error);
											loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
										}
										goto next; // skip rest of period
									}
									break;
								default: {
									std::ostringstream s; s << "could not resume device: " << ::snd_strerror(error);
									throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
								}
							}
							break;
						case -EBADFD: // pcm device is not in the right state (::SND_PCM_STATE_PREPARED or ::SND_PCM_STATE_RUNNING)
						default: {
							std::ostringstream s; s << "write error: " << ::snd_strerror(error);
							throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
					}
					next:
					in += frames_written * channels;
					frames_to_write -= frames_written;
					if(false && loggers::trace() && frames_to_write) {
						std::ostringstream s; s << "overrun: " << frames_to_write << " frames";
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				} while(frames_to_write);
			}
			{ scoped_lock lock(mutex_);
				io_ready(true);
			}
			condition_.notify_one();
		}
	}
}

void alsa::do_process() throw(engine::exception) {
	if(false && loggers::trace()) {
		std::ostringstream s; s << "process";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(!in_port()) return;
	{ scoped_lock lock(mutex_);
		if(false && loggers::warning() && !io_ready()) loggers::warning()("blocking", UNIVERSALIS__COMPILER__LOCATION);
		while(!io_ready()) condition_.wait(lock);
	}
	{ // fill the intermediate buffer
		unsigned int const channels(in_port().channels());
		::snd_pcm_uframes_t const samples_per_buffer(parent().events_per_buffer());
		//assert(last_samples_.size() == channels); ///\todo last_samples_
		for(unsigned int c(0); c < channels; ++c) {
			engine::buffer::channel & in(in_port().buffer()[c]);
			output_sample_type * out(reinterpret_cast<output_sample_type*>(intermediate_buffer_));
		
			// retrieve the last sample written on this channel
			// sss: sparse spread sample
			///\todo last_samples_
			output_sample_type sss(0); ///\todo support for non-interleaved channels
			// ssi: sparse spread index
			unsigned int ssi(0);
			for(std::size_t e(0), s(in.size()); e < s && in[e].index() < samples_per_buffer; ++e) {
				real s(in[e].sample());
				{
					///\todo use bits_per_channel_sample_;
					s *= std::numeric_limits<output_sample_type>::max();
					if     (s < std::numeric_limits<output_sample_type>::min()) s = std::numeric_limits<output_sample_type>::min();
					else if(s > std::numeric_limits<output_sample_type>::max()) s = std::numeric_limits<output_sample_type>::max();
				}
				sss = ::lrint(s);
				for( ; ssi <= in[e].index(); ++ssi) out[ssi + c] = sss; ///\todo support for non-interleaved channels
			}
			for( ; ssi < samples_per_buffer; ++ssi) out[ssi + c] = sss; ///\todo support for non-interleaved channels
		}
	}
	// We don't write to the device here, because it must be done by the poller thread or libasound crashes.
	{ scoped_lock lock(mutex_);
		io_ready(false);
	}
	condition_.notify_one();
}

void alsa::do_stop() throw(engine::exception) {
	if(loggers::information()) loggers::information()("terminating and joining poller thread ...", UNIVERSALIS__COMPILER__LOCATION);
	if(!thread_) {
		if(loggers::information()) loggers::information()("poller thread was not running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
	}
	condition_.notify_one();
	thread_->join();
	if(loggers::information()) loggers::information()("poller thread joined", UNIVERSALIS__COMPILER__LOCATION);
	delete thread_; thread_ = 0;
	resource::do_stop();
}

void alsa::do_close() throw(engine::exception) {
	if(pcm_) ::snd_pcm_close(pcm_); pcm_ = 0;  // or ::snd_pcm_free(pcm_); ?
	#if 1
		// nothing needed for write method
	#else
		delete[] areas_; areas_ = 0;
	#endif
	delete[] intermediate_buffer_; intermediate_buffer_ = 0;
	resource::do_close();
}

alsa::~alsa() throw() {
	close();
}

}}}
