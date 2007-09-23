// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\implementation psycle::plugins::outputs::alsa
#include <psycle/detail/project.private.hpp>
#include "alsa.hpp"
namespace psycle { namespace plugins { namespace outputs {
	PSYCLE__PLUGINS__NODE_INSTANTIATOR(alsa)

	alsa::alsa(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name) throw(engine::exception)
	:
		resource(plugin_library_reference, graph, name),
		pcm_name_("plughw:0,0"),
		pcm_hw_params_(0),
		pcm_(0)
	{
		engine::ports::inputs::single::create_on_heap(*this, "in");
		engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
	}

	void alsa::do_open() throw(engine::exception) {
		resource::do_open();
		
		///\todo parametrable
		unsigned int const rate_requested(44100);
		unsigned int const channels(2);
		unsigned int const bytes_per_sample(4); // for total of channels
		unsigned int const periods(4);
		unsigned int const period_size(4096);
		
		int error;
		if(0 > (error = ::snd_pcm_open(&pcm_, pcm_name_.c_str(), ::SND_PCM_STREAM_PLAYBACK, 0 /* ::SND_PCM_NONBLOCK ::SND_PCM_ASYNC */))) {
			std::ostringstream s; s << "could not open pcm device: " << pcm_name_ << ": " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		try {
			if(0 > (error = ::snd_pcm_hw_params_malloc(&pcm_hw_params_))) {
				std::ostringstream s; s << "could not allocate hardware parameter structure: " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			try {
				if(0 > (error = ::snd_pcm_hw_params_any(pcm_, pcm_hw_params_))) {
					std::ostringstream s; s << "could not initialize hardware parameter structure: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(0 > (error = ::snd_pcm_hw_params_set_access(pcm_, pcm_hw_params_, ::SND_PCM_ACCESS_RW_INTERLEAVED))) {
					std::ostringstream s; s << "could not set access type: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(0 > (error = ::snd_pcm_hw_params_set_format(pcm_, pcm_hw_params_, ::SND_PCM_FORMAT_S16_LE))) {
					std::ostringstream s; s << "could not set sample format: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				unsigned int rate_accepted(rate_requested);
				if(0 > (error = ::snd_pcm_hw_params_set_rate_near(pcm_, pcm_hw_params_, &rate_accepted, 0))) {
					std::ostringstream s; s << "could not set sample rate to " << rate_requested << ": " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(loggers::warning()() && rate_accepted != rate_requested) {
					std::ostringstream s; s << "warning: sample rate requested: " << rate_requested << ", sample rate accepted: " << rate_accepted;
					loggers::warning()(s.str());
				}
				if(0 > (error = ::snd_pcm_hw_params_set_channels(pcm_, pcm_hw_params_, 2))) {
					std::ostringstream s; s << "could not set channel count to " << channels << ": " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(0 > (error = ::snd_pcm_hw_params_set_periods(pcm_, pcm_hw_params_, periods, 0))) {
					std::ostringstream s; s << "could not set period count to " << periods << ": " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				int const buffer_size(period_size * periods / bytes_per_sample);
				if(0 > (error = ::snd_pcm_hw_params_set_buffer_size(pcm_, pcm_hw_params_, buffer_size))) {
					std::ostringstream s; s << "could not set buffer size to " << buffer_size << ": " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(0 > (error = ::snd_pcm_hw_params(pcm_, pcm_hw_params_))) {
					std::ostringstream s; s << "could not set hardware parameters: " << ::snd_strerror(error);
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			} catch(...) {
				::snd_pcm_hw_params_free(pcm_hw_params_);
				pcm_hw_params_ = 0;
				throw;
			}
		} catch(...) {
			::snd_pcm_close(pcm_);
			pcm_ = 0;
			throw;
		}
	}

	bool alsa::opened() const {
		return pcm_;
	}

	void alsa::do_start() throw(engine::exception) {
		resource::do_start();
		int error;
		if((error = ::snd_pcm_prepare(pcm_)) < 0) {
			std::ostringstream s; s << "could not prepare audio interface for use: " << ::snd_strerror(error);
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}

	bool alsa::started() const {
		if(!opened()) return false;
		return true;
	}

	void alsa::do_process() throw(engine::exception) {
		for (int i(0) ; i < 10 ; ++i) {
			universalis::compiler::numeric<16>::signed_int buf[128];
			int error;
			if((error = ::snd_pcm_writei(pcm_, buf, 128)) != 128) {
				std::ostringstream s; s << "write to audio interface failed: " << ::snd_strerror(error);
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	}

	void alsa::do_stop() throw(engine::exception) {
		resource::do_stop();
	}

	void alsa::do_close() throw(engine::exception) {
		::snd_pcm_hw_params_free(pcm_hw_params_);
		pcm_hw_params_ = 0;
		::snd_pcm_close(pcm_);
		pcm_ = 0;
		resource::do_close();
	}
	
	alsa::~alsa() throw() {
		close();
	}
}}}

