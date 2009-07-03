// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::direct_sound
#include <psycle/detail/project.private.hpp>
#include "direct_sound.hpp"
#include <psycle/stream/formats/riff_wave/format.hpp>
#include <universalis/os/exceptions/code_description.hpp>
#include <thread>
#include <sstream>
namespace psycle { namespace plugins { namespace outputs {
	using stream::formats::riff_wave::format;

	PSYCLE__PLUGINS__NODE_INSTANTIATOR(direct_sound)

	direct_sound::direct_sound(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name) throw(universalis::os::exception)
	:
		resource(plugin_library_reference, graph, name),
		direct_sound_(),
		buffer_(),
		started_()
	{
		engine::ports::inputs::single::create_on_heap(*this, "in");
		engine::ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
	}

	void direct_sound::channel_change_notification_from_port(engine::port const & port) throw(engine::exception) {
		if(&port == &in_port()) {
			last_samples_.resize(port.channels());
			for(std::size_t i(0); i < last_samples_.size(); ++i) last_samples_[i] = 0;
		}
		resource::channel_change_notification_from_port(port);
	}

	void direct_sound::do_open() throw(universalis::os::exception) {
		HRESULT error(0);
		try {
			resource::do_open();
			if(error = ::DirectSoundCreate(0, &direct_sound_, 0)) throw universalis::os::exceptions::runtime_error("direct sound create: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			format format(in_port().channels(), in_port().events_per_second(), /*significant_bits_per_channel_sample*/ 16); /// \todo parametrable
			if(loggers::information()()) {
				std::ostringstream s;
				s << "format: " << format.description();
				loggers::information()(s.str());
			}
			samples_per_buffer_ = parent().events_per_buffer();
			buffers_ = 4; /// \todo parametrable
			write_primary_ = false; /// \todo parametrable
			if(write_primary_) {
				if(error = direct_sound_implementation().SetCooperativeLevel(::GetDesktopWindow(), DSSCL_WRITEPRIMARY)) {
					buffer_ = 0;
					//throw universalis::os::exceptions::runtime_error("direct sound set write primary cooperative level: " + os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
					// actually, we should not report this, since we may simply do not have the focus
					return; ///\todo that sux.. we're just going to crash latter
				}
			}
			else if(error = direct_sound_implementation().SetCooperativeLevel(::GetDesktopWindow(), DSSCL_PRIORITY)) throw universalis::os::exceptions::runtime_error("direct sound set priority cooperative level: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			::DSBUFFERDESC description;
			std::memset(&description, 0, sizeof description);
			description.dwSize = sizeof description;
			description.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2;
			total_buffer_size_ = 0;
			if(write_primary_) {
				description.dwFlags |= DSBCAPS_PRIMARYBUFFER;
			} else {
				description.dwFlags |= DSBCAPS_GLOBALFOCUS;
				description.lpwfxFormat = &format.wave_format_ex();
				description.dwBufferBytes = total_buffer_size_ = buffers_ * samples_per_buffer_ * static_cast<unsigned long int>(format.bytes_per_sample());
			}
			if(FAILED(error = direct_sound_implementation().CreateSoundBuffer(&description, &buffer_, 0))) throw universalis::os::exceptions::runtime_error("direct sound create sound buffer: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			if(error && loggers::warning()()) {
				std::ostringstream s;
				s << "warning: create sound buffer: " << universalis::os::exceptions::code_description(error);
				loggers::warning()(s.str());
			}
			if(write_primary_) {
				buffer().Stop(); // o_O`
				if(error = buffer().SetFormat(&format.wave_format_ex())) throw universalis::os::exceptions::runtime_error("direct sound buffer set format: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			}
			::DSBCAPS caps;
			caps.dwSize = sizeof(caps);
			if(error = buffer().GetCaps(&caps)) {
				buffer().Release();
				buffer_ = 0;
				throw universalis::os::exceptions::runtime_error("direct sound buffer get caps: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(caps.dwFlags & DSBCAPS_STATIC) throw universalis::os::exceptions::runtime_error("direct sound buffer get caps, what kind of fancy sound card hardware you got ? it uses on-board memory only !", UNIVERSALIS__COMPILER__LOCATION);
			if(write_primary_) total_buffer_size_ = caps.dwBufferBytes;
			assert(total_buffer_size_);
			if(loggers::information()()) {
				std::ostringstream s;
				s << "total buffer size: " << total_buffer_size_ << " bytes";
				loggers::information()(s.str());
			}
			buffer_size_ = static_cast<unsigned int>(samples_per_buffer_ * format.bytes_per_sample());
			if(loggers::information()()) {
				std::ostringstream s;
				s << "buffer size: " << buffer_size_ << " bytes";
				loggers::information()(s.str());
			}
			{
				real const latency(static_cast<real>(samples_per_buffer_) / format.samples_per_second());
				if(loggers::information()()) {
					std::ostringstream s;
					s << "latency: between " << latency << " and " << latency * buffers_ << " seconds ";
					loggers::information()(s.str());
				}
			}
		}
		catch(std::exception const & e) {
			std::ostringstream s;
			s << e.what();
			if(FAILED(error)) s << ": " << universalis::os::exceptions::code_description(error);
			throw universalis::os::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}

	bool direct_sound::opened() const {
		return direct_sound_; //&& buffer_;
	}

	void direct_sound::do_start() throw(universalis::os::exception) {
		resource::do_start();
		current_position_ = 0;
		buffer().Play(0, 0, DSBPLAY_LOOPING); // may return DSERR_BUFFERLOST
		started_ = true;
	}

	bool direct_sound::started() const {
		return started_;
	}

	void direct_sound::do_process() throw(universalis::os::exception) {
		bool const ultra_trace(false);
		while(true) {
			DWORD position;
			if(HRESULT error = buffer().GetCurrentPosition(&position, 0)) throw universalis::os::exceptions::runtime_error("direct sound buffer get current position: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			if(ultra_trace && loggers::trace()()) {
				std::ostringstream s;
				s << "position: " << position;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(position <  current_position_ * buffer_size_) break;
			if(position >= current_position_ * buffer_size_ + buffer_size_) break;
			// beware: using a sleep time of 0 will freeze the system if process has real time priority class and thread has time critical priority!
			//std::this_thread::sleep(std::milliseconds(1));
			std::this_thread::yield();
		}
		if(!loggers::trace()()) {
			static const char c [] = { '-', '\\', '|', '/' };
			std::cout << ' ' << c[current_position_ % sizeof c] << '\r' << std::flush;
		}
		output_sample_type * samples(0), * samples2(0);
		DWORD bytes(0), bytes2(0);
		bool restored(false);
		while(HRESULT error = buffer().Lock(current_position_ * buffer_size_, buffer_size_, reinterpret_cast<void**>(&samples), &bytes, reinterpret_cast<void**>(&samples2), &bytes2, 0)) {
			if(error != DSERR_BUFFERLOST) throw universalis::os::exceptions::runtime_error("direct sound buffer lock: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			while(true) if(!buffer().Restore()) break;
			if(loggers::information()()) {
				std::ostringstream s;
				s << "buffer restored";
				loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			current_position_ = 0;
			restored = true;
		}
		if(bytes2) throw universalis::os::exceptions::runtime_error("direct sound buffer lock unaligned", UNIVERSALIS__COMPILER__LOCATION);
		assert(last_samples_.size() == in_port().channels());
		engine::buffer & in = in_port().buffer();
		for(int channel(0) ; channel < in.channels() ; ++channel) {
			int spread(0);
			for(int event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event) {
				if(ultra_trace && loggers::trace()()) {
					std::ostringstream s;
					s << "event: " << event << ", spread: " << spread << ", sample: "<< in[channel][event].sample();
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				real sample(in[channel][event].sample());
				{
					sample *= std::numeric_limits<output_sample_type>::max();
					if     (sample < std::numeric_limits<output_sample_type>::min()) sample = std::numeric_limits<output_sample_type>::min();
					else if(sample > std::numeric_limits<output_sample_type>::max()) sample = std::numeric_limits<output_sample_type>::max();
					assert(std::numeric_limits<output_sample_type>::min() <= sample && sample <= std::numeric_limits<output_sample_type>::max());
				}
				last_samples_[channel] = static_cast<output_sample_type>(sample);
				for( ; spread <= in[channel][event].index() ; ++spread) samples[spread + channel] = last_samples_[channel];
				if(ultra_trace && loggers::trace()()) {
					std::ostringstream s;
					s << "spread: " << spread << ", sample: " << last_samples_[channel];
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
			}
			for( ; spread < samples_per_buffer_ ; ++spread) samples[spread + channel] = last_samples_[channel];
		}
		if(HRESULT error = buffer().Unlock(samples, bytes, samples2, 0)) throw universalis::os::exceptions::runtime_error("direct sound buffer unlock: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
		if(restored) buffer().Play(0, 0, DSBPLAY_LOOPING); // may return DSERR_BUFFERLOST
		++current_position_ %= buffers_;
	}

	void direct_sound::do_stop() throw(universalis::os::exception) {
		if(HRESULT error = buffer().Stop()) throw universalis::os::exceptions::runtime_error("direct sound buffer stop: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
		resource::do_stop();
		started_ = false;
	}

	void direct_sound::do_close() throw(universalis::os::exception) {
		if(buffer_) {
			try {
				if(HRESULT error = buffer().Release()) throw universalis::os::exceptions::runtime_error("direct sound buffer release: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			} catch(...) {
				loggers::crash()("please ignore the crash above, this is possibly a bug in your driver", UNIVERSALIS__COMPILER__LOCATION);
			}
			buffer_ = 0;
		}
		if(direct_sound_) {
			if(HRESULT error = direct_sound_implementation().Release()) throw universalis::os::exceptions::runtime_error("direct sound release: " + universalis::os::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
			direct_sound_ = 0;
		}
		resource::do_close();
	}
}}}
