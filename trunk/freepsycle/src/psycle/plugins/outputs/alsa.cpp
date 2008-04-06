// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::alsa
#include <psycle/detail/project.private.hpp>
#include "alsa.hpp"
#include <diversalis/processor.hpp>
#include <cstdio>
#include <thread>
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
		buffer_(),
		areas_()
	{
		engine::ports::inputs::single::create_on_heap(*this, "in");
		engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
	}

	void alsa::do_open() throw(engine::exception) {
		resource::do_open();
		
		int error;

		// attach to std output
		if(0 > (error = ::snd_output_stdio_attach(&output_, ::stdout, 0))) {
			std::ostringstream s; s << "could not attach to stdio output: " << ::snd_strerror(error);
			loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}

		unsigned int rate(::lround(in_port().events_per_second()));
		bool const allow_resample(true);

		unsigned int const channels(in_port().channels());

		::snd_pcm_format_t format(::SND_PCM_FORMAT_S16); ///\todo parametrable
		{ // get format from env
			char const * const env(std::getenv("PSYCLE__PLUGINS__OUTPUTS__ALSA__FORMAT"));
			if(env) format = ::snd_pcm_format_value(env);
		}
		// get the sample width
		bits_per_channel_sample_ = ::snd_pcm_format_width(format);

		std::string pcm_device_name("default"); ///\todo parametrable
		{ // get device name from env
			char const * const env(std::getenv("ALSA_PCM_DEVICE"));
			if(env) pcm_device_name = env;
		}

		unsigned int periods(4); ///\todo parametrable
		{ // get periods from env
			char const * const env(std::getenv("PSYCLE__PLUGINS__OUTPUTS__ALSA__PERIODS"));
			if(env) { std::stringstream s; s << env; s >> periods; }
		}

		#if 1
			period_frames_ = parent().events_per_buffer();
		#else
			period_frames_ = 4096; ///\todo parametrable
			{ // get period frames from env
				char const * const env(std::getenv("PSYCLE__PLUGINS__OUTPUTS__ALSA__PERIOD_FRAMES"));
				if(env) { std::stringstream s; s << env; s >> period_frames_; }
			}
		#endif

		::snd_pcm_stream_t const direction(::SND_PCM_STREAM_PLAYBACK);
		int const open_mode(0); // 0: block, ::SND_PCM_NONBLOCK: non-block, ::SND_PCM_ASYNC: asynchronous
		bool const work_mode_is_non_blocking(false);
		::snd_pcm_access_t const access(::SND_PCM_ACCESS_RW_INTERLEAVED);

		if(loggers::information()()) {
			std::ostringstream s;
			s <<
				"alsa pcm device: about to setup:\n"
				"\t" "device name: " << pcm_device_name << "\n"
				"\t" "stream direction: " << ::snd_pcm_stream_name(direction) << "\n"
				"\t" "stream parameters: "
					"rate: " << rate << "Hz (resampling " << (allow_resample ? "allowed" : "disallowed") << "), "
					"format: " << ::snd_pcm_format_description(format) << ", "
					"channels: " << channels << "\n"
				"\t" "open mode: " << open_mode_description(open_mode) << "\n"
				"\t" "work mode: " << (work_mode_is_non_blocking ? "non-blocking" : "blocking") << "\n"
				"\t" "access method: " << ::snd_pcm_access_name(access);
			loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}

		// open the pcm device
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
		if(loggers::information()()) {
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
				} else if(loggers::information()()) {
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
		// set the work mode
		if(0 > (error = ::snd_pcm_nonblock(pcm_, work_mode_is_non_blocking))) {
			std::ostringstream s;
			s <<
				"could not set work mode to: " <<
				(work_mode_is_non_blocking ? "non-blocking" : "blocking") <<
				": " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		try {
			// allocate hardware parameters on the stack
			::snd_pcm_hw_params_t * pcm_hw_params; snd_pcm_hw_params_alloca(&pcm_hw_params);
			// make the configuration space full
			if(0 > (error = ::snd_pcm_hw_params_any(pcm_, pcm_hw_params))) {
				std::ostringstream s; s << "could not initialize hardware parameters: " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
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
			// set the access method
			if(0 > (error = ::snd_pcm_hw_params_set_access(pcm_, pcm_hw_params, access))) {
				std::ostringstream s;
				s << "could not set access method to: " << ::snd_pcm_access_name(access) << ": " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			// set the sample format
			if(0 > (error = ::snd_pcm_hw_params_set_format(pcm_, pcm_hw_params, format))) {
				std::ostringstream s;
				s <<
					"could not set sample format to: " << ::snd_pcm_format_description(format) <<
					": " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			// set the channel count
			if(0 > (error = ::snd_pcm_hw_params_set_channels(pcm_, pcm_hw_params, channels))) {
				std::ostringstream s;
				s <<
					"could not set channel count to: " << channels <<
					": " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			{ // set the sample rate
				unsigned int rate_accepted(rate);
				int direction(0);
				if(0 > (error = ::snd_pcm_hw_params_set_rate_near(pcm_, pcm_hw_params, &rate_accepted, &direction))) {
					std::ostringstream s; s << "could not set sample rate to: " << rate << "Hz: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(rate_accepted != rate) {
					std::ostringstream s;
					s <<
						"sample rate: "
						"requested: " << rate << "Hz, "
						"accepted: " << rate_accepted << "Hz";
					loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					rate = rate_accepted;
				}
			}
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
			// apply hw parameter settings to the pcm device and prepare it
			if(0 > (error = ::snd_pcm_hw_params(pcm_, pcm_hw_params))) {
				std::ostringstream s; s << "could not set hardware parameters: " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(loggers::information()()) {
				std::ostringstream s; s << "alsa pcm device: name: " << pcm_device_name << ": setup successful";
				loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			// At this point ::snd_pcm_prepare(pcm_) was called automatically,
			// and hence ::snd_pcm_state(pcm_) was brought to ::SND_PCM_STATE_PREPARED state.
			
			{ // allocate a buffer
				std::size_t const bytes(period_frames_ * channels * bits_per_channel_sample_ / std::numeric_limits<unsigned char>::digits);
				if(!(buffer_ = new char[bytes])) {
					std::ostringstream s; s << "not enough memory to allocate " << bytes << " bytes on heap";
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
			// setup areas within buffer
			if(!(areas_ = new ::snd_pcm_channel_area_t[channels])) {
				std::ostringstream s; s << "not enough memory to allocate " << (channels * sizeof *areas_) << " bytes on heap";
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			for(unsigned int i(0); i < channels; ++i) {
				areas_[i].addr = buffer_;
				areas_[i].first = i * bits_per_channel_sample_;
				areas_[i].step = channels * bits_per_channel_sample_;
			}
		} catch(...) {
			delete[] buffer_; buffer_ = 0;
			delete[] areas_; areas_ = 0;
			::snd_pcm_close(pcm_); pcm_ = 0; // or ::snd_pcm_free(pcm_); ?
			throw;
		}
	}

	bool alsa::opened() const {
		return pcm_;
	}

	void alsa::do_start() throw(engine::exception) {
		resource::do_start();

		unsigned int const channels(in_port().channels());
		::snd_pcm_uframes_t const samples_per_buffer(parent().events_per_buffer());

		int error;

		// allocate software parameters on the stack
		::snd_pcm_sw_params_t * pcm_sw_params; snd_pcm_sw_params_alloca(&pcm_sw_params);
		// get the current sw_params
		if(0 > (error = ::snd_pcm_sw_params_current(pcm_, pcm_sw_params))) {
			std::ostringstream s; s << "could not initialize software parameters: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		// start the transfer when the buffer is almost full: buffer_size / avail_min * avail_min
		if(0 > (error = ::snd_pcm_sw_params_set_start_threshold(pcm_, pcm_sw_params, buffer_frames_ / period_frames_ * period_frames_))) {
			std::ostringstream s; s << "could not set start threshold mode for playback: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		// allow the transfer when at least period_frames samples can be processed
		if(0 > (error = ::snd_pcm_sw_params_set_avail_min(pcm_, pcm_sw_params, period_frames_))) {
			std::ostringstream s; s << "could not set avail min for playback: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		#if SND_LIB_VERSION >= 0x10010 // 1.0.16
			// snd_pcm_sw_params_set_xfer_align() is deprecated, alignment is always 1
		#else
			// align all transfers to 1 sample
			if (0 > (error = ::snd_pcm_sw_params_set_xfer_align(pcm_, pcm_sw_params, 1)) {
				std::ostringstream s; s << "could not set transfer align for playback: " << ::snd_strerror(error);
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
	}

	#if 0 // this is a bit meaningless without a thread
		bool alsa::started() const {
			if(!opened()) return false;
			::snd_pcm_state_t const state(::snd_pcm_state(pcm_));
			return state >= ::SND_PCM_STATE_PREPARED; // this is a bit meaningless without a thread
		}
	#endif

	void alsa::do_process() throw(engine::exception) {
		if(!in_port()) return;
		assert(parent().events_per_buffer() == period_frames_);
		::snd_pcm_uframes_t frames_to_write(parent().events_per_buffer());
		do {
			fill_buffer();
			write_to_device();
			frames_to_write -= period_frames_;
		} while(frames_to_write);
	}
		
	void alsa::fill_buffer() throw(engine::exception) {
		unsigned int const channels(in_port().channels());
		::snd_pcm_uframes_t const samples_per_buffer(parent().events_per_buffer());
		for(unsigned int c(0); c < channels; ++c) {
			engine::buffer::channel & in(in_port().buffer()[c]);
			output_sample_type * out(reinterpret_cast<output_sample_type*>(buffer_));
			
			// retrieve the last sample written on this channel
			// sss: sparse spread sample
			output_sample_type sss(out[samples_per_buffer * channels - 1 - c]); ///\todo support for non-interleaved channels
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
	
	void alsa::write_to_device() throw(engine::exception) {
		unsigned int const channels(in_port().channels());
		output_sample_type * samples(reinterpret_cast<output_sample_type*>(buffer_));
		::snd_pcm_uframes_t frames_to_write(period_frames_);
		do {
			///\todo support for non-blocking mode
			///\todo support for non-interleaved channels
			::snd_pcm_sframes_t const frames_written(::snd_pcm_writei(pcm_, samples, frames_to_write));
			int error(frames_written);
			if(0 > error) switch(error) {
				case -EAGAIN: // not documented, but seen in example code!
					if(loggers::warning()()) {
						std::ostringstream s; s << "weird: " << ::snd_strerror(error);
						loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					continue;
				case -EPIPE: // an underrun occured
					if(loggers::warning()()) {
						std::ostringstream s; s << "underrun: " << ::snd_strerror(error);
						loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(0 > (error = ::snd_pcm_prepare(pcm_))) {
						if(loggers::warning()()) {
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
						if(loggers::trace()()) {
							std::ostringstream s; s << "waiting for device to resume: " << ::snd_strerror(error);
							loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						// sleep a bit before retrying to resume
						std::this_thread::sleep(std::seconds(1));
					}
					if(0 > error) switch(error) {
						case -ENOSYS: // the device is no longer suspended, but it does not fully support resuming
							if(0 > (error = ::snd_pcm_prepare(pcm_))) {
								if(loggers::warning()()) {
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
			samples += frames_written * channels;
			frames_to_write -= frames_written;
		} while(frames_to_write);
	}

	#if 0 // meaningless without a thread
		void alsa::do_stop() throw(engine::exception) {
			resource::do_stop();
		}
	#endif

	void alsa::do_close() throw(engine::exception) {
		if(pcm_) ::snd_pcm_close(pcm_); pcm_ = 0;  // or ::snd_pcm_free(pcm_); ?
		delete[] areas_; areas_ = 0;
		delete[] buffer_; buffer_ = 0;
		resource::do_close();
	}
	
	alsa::~alsa() throw() {
		close();
	}
}}}

