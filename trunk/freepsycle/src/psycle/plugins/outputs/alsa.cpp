// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::alsa
#include <psycle/detail/project.private.hpp>
#include "alsa.hpp"
#include <diversalis/processor.hpp>
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
		period_frames_(),
		samples_(),
		areas_(),
		output_()
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

		unsigned int rate(::lround(single_input_ports()[0]->events_per_second()));
		bool const allow_resample(true);

		unsigned int const channels(single_input_ports()[0]->channels());

		::snd_pcm_format_t format(::SND_PCM_FORMAT_S16); ///\todo parametrable
		{ // get format from env
			char const * const env(std::getenv("PSYCLE__PLUGINS__OUTPUTS__ALSA__FORMAT"));
			if(env) format = ::snd_pcm_format_value(env);
		}
		std::size_t const bytes_per_sample(::snd_pcm_format_width(format) / 8 * channels); // for total of channels

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

		if(!period_frames_) {
			// get period frames from env
			char const * const env(std::getenv("PSYCLE__PLUGINS__OUTPUTS__ALSA__PERIOD_FRAMES"));
			if(env) { std::stringstream s; s << env; s >> period_frames_; }
			else period_frames_ = 4096;
		}

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
			#if SND_LIB_VERSION >= 0x010009
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
					unsigned int period_time_accepted(period_time);
					int const direction(0);
					if(0 > (error = ::snd_pcm_hw_params_set_period_time_near(pcm_, pcm_hw_params, &period_time_accepted, direction))) {
						std::ostringstream s;
						s <<
							"could not set period time to: " << period_time_requested <<
							": " << ::snd_strerror(error);
						throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(period_time_accepted != period_time) {
						std::ostringstream s;
						s <<
							"period time: "
							"requested: " << period_time_requested << "us, "
							"accepted: " << period_time_accepted << "us";
						loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						period_time = period_time_accepted;
					}
				}
				{ // get the period size in frames
					int direction(0);
					if (0 > (error = ::snd_pcm_hw_params_get_period_size(params, &period_frames_, &direction))) {
						std::ostringstream s; s << "could not get period size: " << ::snd_strerror(error);
						throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			#endif
			#if 1
				{ // set the buffer size in frames
					::snd_pcm_uframes_t buffer_frames(period_frames_ * periods);
					::snd_pcm_uframes_t buffer_frames_accepted(buffer_frames);
					if(0 > (error = ::snd_pcm_hw_params_set_buffer_size_near(pcm_, pcm_hw_params, &buffer_frames_accepted))) {
						std::ostringstream s;
						s << "could not set buffer size to: " << buffer_frames << " samples: " << ::snd_strerror(error);
						throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(buffer_frames_accepted != buffer_frames) {
						std::ostringstream s;
						s <<
							"buffer size: "
							"requested: " << buffer_frames << " samples, "
							"accepted: " << buffer_frames_accepted << " samples";
						loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						buffer_frames = buffer_frames_accepted;
					}
				}
			#else
				{ // set the buffer time in microseconds
					unsigned int buffer_time_accepted(buffer_time);
					int direction(0);
					if(0 > (error = ::snd_pcm_hw_params_set_buffer_time_near(pcm_, pcm_hw_params, &buffer_time_accepted, &direction))) {
						std::ostringstream s;
						s << "could not set buffer time to: " << buffer_time << ": " << ::snd_strerror(error);
						throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(buffer_time_accepted != buffer_time) {
						std::ostringstream s;
						s <<
							"buffer time: "
							"requested: " << buffer_time << "us, "
							"accepted: " << buffer_time_accepted << "us";
						loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						buffer_time = buffer_time_accepted;
					}
				}
				{ // get the buffer size in frames
					if (0 > (error = ::snd_pcm_hw_params_get_buffer_size(params, &buffer_frames))) {
						std::ostringstream s; s << "could not get buffer size: " << ::snd_strerror(error);
						throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
			#endif
			// apply hw parameter settings to the pcm device and prepare it
			if(0 > (error = ::snd_pcm_hw_params(pcm_, pcm_hw_params))) {
				std::ostringstream s; s << "could not set hardware parameters: " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			} else if(loggers::information()()) {
				std::ostringstream s; s << "alsa pcm device: name: " << pcm_device_name << ": setup successful";
				loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			// At this point ::snd_pcm_prepare(pcm_) was called automatically,
			// and hence ::snd_pcm_state(pcm_) is ::SND_PCM_STATE_PREPARE.
		} catch(...) {
			::snd_pcm_close(pcm_); pcm_ = 0; // or ::snd_pcm_free(pcm_); ?
			throw;
		}
	}

	bool alsa::opened() const {
		return pcm_;
	}

	void alsa::do_start() throw(engine::exception) {
		resource::do_start();
		int error;
		if(0 > (error = ::snd_pcm_prepare(pcm_))) {
			std::ostringstream s; s << "could not prepare device for use: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}

	bool alsa::started() const {
		if(!opened()) return false;
		::snd_pcm_state_t const state(::snd_pcm_state(pcm_));
		return
			state == ::SND_PCM_STATE_RUNNING ||
			state == ::SND_PCM_STATE_XRUN ||
			state == ::SND_PCM_STATE_DRAINING ||
			state == ::SND_PCM_STATE_PAUSED ||
			state == ::SND_PCM_STATE_SUSPENDED;
	}

	void alsa::do_process() throw(engine::exception) {
		for (int i(0) ; i < 10 ; ++i) {
			std::int16_t buf[128];
			int error;
			if((error = ::snd_pcm_writei(pcm_, buf, 128)) != 128) {
				std::ostringstream s; s << "write to device failed: " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	}

	void alsa::do_stop() throw(engine::exception) {
		resource::do_stop();
	}

	void alsa::do_close() throw(engine::exception) {
		::snd_pcm_close(pcm_); pcm_ = 0;  // or ::snd_pcm_free(pcm_); ?
		resource::do_close();
	}
	
	alsa::~alsa() throw() {
		close();
	}
}}}

