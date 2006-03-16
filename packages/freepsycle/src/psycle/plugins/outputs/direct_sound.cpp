// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright � 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief \implementation psycle::plugins::outputs::direct_sound
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include <universalis/operating_system/exceptions/code_description.hpp>
#include <universalis/operating_system/threads/sleep.hpp>
#include <psycle/stream/formats/riff_wave/format.hpp>
#include "direct_sound.hpp"
#include <sstream>
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			using stream::formats::riff_wave::format;

			PSYCLE__PLUGINS__NODE_INSTANCIATOR(direct_sound)

			direct_sound::direct_sound(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, const std::string & name) throw(universalis::operating_system::exception)
			:
				resource(plugin_library_reference, graph, name),
				direct_sound_(0),
				buffer_(0)
			{
				new engine::ports::inputs::single(*this, "in");
				new engine::ports::inputs::single(*this, "amplification", 1);
			}

			void direct_sound::do_open() throw(universalis::operating_system::exception)
			{
				resource::do_open();
				if(HRESULT error = ::DirectSoundCreate(0, &direct_sound_, 0)) throw universalis::operating_system::exceptions::runtime_error("direct sound create: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
				allocate_buffer();
			}
		
			bool direct_sound::opened() const
			{
				return direct_sound_;
			}
		
			bool direct_sound::allocate_buffer() throw(universalis::operating_system::exception)
			{
				HRESULT error(0);
				try
				{
					bits_per_sample_ = 16; /// \todo parametrable
					format format(input_ports()[0]->channels(), static_cast<int>(input_ports()[0]->events_per_second()), bits_per_sample_);
					if(loggers::information()())
					{
						std::ostringstream s;
						s << "format: " << format.description();
						loggers::information()(s.str());
					}
					samples_per_buffer_ = 8192; /// \todo parametrable
					buffers_ = 4; /// \todo parametrable
					write_primary_ = false; /// \todo parametrable
					if(write_primary_)
					{
						if(error = direct_sound_instance().SetCooperativeLevel(::GetDesktopWindow(), DSSCL_WRITEPRIMARY))
						{
							buffer_ = 0;
							//throw universalis::operating_system::exceptions::runtime_error("direct sound set write primary cooperative level: " + operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
							// actually, we should not report this, since we may simply do not have the focus
							return false; // \todo that sux.. we're just going to crash latter
						}
					}
					else if(error = direct_sound_instance().SetCooperativeLevel(::GetDesktopWindow(), DSSCL_PRIORITY)) throw universalis::operating_system::exceptions::runtime_error("direct sound set priority cooperative level: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
					::DSBUFFERDESC description;
					std::memset(&description, 0, sizeof description);
					description.dwSize = sizeof description;
					description.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2;
					if(write_primary_)
					{
						description.dwFlags |= DSBCAPS_PRIMARYBUFFER;
					}
					else
					{
						description.dwFlags |= DSBCAPS_GLOBALFOCUS;
						description.lpwfxFormat = &format.wave_format_ex();
						description.dwBufferBytes = total_buffering_size_ = buffers_ * samples_per_buffer_ * static_cast<unsigned long int>(format.bytes_per_sample());
					}
					if(FAILED(error = direct_sound_instance().CreateSoundBuffer(&description, &buffer_, 0))) throw universalis::operating_system::exceptions::runtime_error("direct sound create sound buffer: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
					if(error && loggers::warning()())
					{
						std::ostringstream s;
						s << "warning: create sound buffer: " << universalis::operating_system::exceptions::code_description(error);
						loggers::warning()(s.str());
					}
					last_sample_ = 0; 
					if(write_primary_)
					{
						buffer().Stop(); // o_O`
						if(error = buffer().SetFormat(&format.wave_format_ex())) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer set format: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
					}
					::DSBCAPS caps;
					caps.dwSize = sizeof(caps);
					if(error = buffer().GetCaps(&caps))
					{
						buffer().Release();
						buffer_ = 0;
						throw universalis::operating_system::exceptions::runtime_error("direct sound buffer get caps: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(caps.dwFlags & DSBCAPS_STATIC) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer get caps, what kind of fancy sound card hardware you got ? it uses on-board memory only !", UNIVERSALIS__COMPILER__LOCATION);
					if(write_primary_) total_buffering_size_ = caps.dwBufferBytes;
					if(loggers::information()())
					{
						std::ostringstream s;
						s << "total buffering size: " << total_buffering_size_ << " bytes";
						loggers::information()(s.str());
					}
					bytes_per_buffer_ = static_cast<unsigned long int>(samples_per_buffer_ * format.bytes_per_sample());
					if(loggers::information()())
					{
						std::ostringstream s;
						s << "bytes per buffer: " << bytes_per_buffer_ << " bytes";
						loggers::information()(s.str());
					}
					real latency = static_cast<real>(samples_per_buffer_) / format.samples_per_second();
					if(loggers::information()())
					{
						std::ostringstream s;
						s << "latency: between " << latency << " and " << latency * buffers_ << " seconds ";
						loggers::information()(s.str());
					}
				}
				catch(const std::exception & e)
				{
					std::ostringstream s;
					s << e.what();
					if(FAILED(error)) s << ": " << universalis::operating_system::exceptions::code_description(error);
					close();
					throw universalis::operating_system::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				return true;
			}
		
			void direct_sound::do_start() throw(universalis::operating_system::exception)
			{
				resource::do_start();
				current_buffer_ = 0;
				buffer().Play(0, 0, DSBPLAY_LOOPING);
			}
		
			bool direct_sound::started() const
			{
				if(!opened()) return false;
				unsigned long int status;
				if(HRESULT error = buffer().GetStatus(&status)) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer get status: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
				return status & DSBSTATUS_PLAYING;
			}
		
			void direct_sound::do_process() throw(universalis::operating_system::exception)
			{
				bool const ultra_trace(false);
				for(;;)
				{
					unsigned long int position;
					if(HRESULT error = buffer().GetCurrentPosition(&position, 0)) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer get current position: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
					if(ultra_trace && loggers::trace()())
					{
						std::ostringstream s;
						s << "position: " << position;
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					if(position < current_buffer_ * bytes_per_buffer_) break;
					if(position > current_buffer_ * bytes_per_buffer_ + bytes_per_buffer_) break;
					universalis::operating_system::threads::sleep(0.0011); // beware: using a sleep time of 0 will freeze the system if process has real time priority class and thread has time critical priority!
				}
				if(!loggers::trace()())
				{
					static const char c [] = { '-', '\\', '|', '/' };
					std::cout << ' ' << c[current_buffer_ % sizeof c] << '\r' << std::flush;
				}
				typedef universalis::compiler::numeric<16>::signed_int integral_sample;
				integral_sample * samples(0), * samples2(0);
				unsigned long int bytes(0), bytes2(0);
				if(HRESULT error = buffer().Lock(current_buffer_ * bytes_per_buffer_, bytes_per_buffer_, reinterpret_cast<void**>(&samples), &bytes, reinterpret_cast<void**>(&samples2), &bytes2, 0))
				{
					if(error != DSERR_BUFFERLOST) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer lock: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
					buffer().Restore(); // \todo may fail again and again!
					if(loggers::information()())
					{
						std::ostringstream s;
						s << "buffer restored";
						loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
				}
				if(bytes2) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer lock unaligned", UNIVERSALIS__COMPILER__LOCATION);
				engine::buffer & in = input_ports()[0]->buffer();
				//real amplification = input_ports()[1]->buffer()[0][0].sample();
				for(int channel(0) ; channel < in.size() ; ++channel)
				{
					int spread(0);
					for(int event(0) ; event < in.events() && in[channel][event].index() < in.events() ; ++event)
					{
						if(ultra_trace && loggers::trace()())
						{
							std::ostringstream s;
							s << "event: " << event << ", spread: " << spread << ", sample: "<< in[channel][event].sample();
							loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						real sample(in[channel][event].sample());
						{
							sample *= 32767;
							if(sample < -32768) sample = -32768;
							else if(sample > 32767) sample = 32767;
							assert(-32768 <= sample && sample <= 32767);
						}
						last_sample_ = static_cast<integral_sample>(sample);
						for( ; spread <= in[channel][event].index() ; ++spread)
						{
							samples[spread + channel] = last_sample_;
						}
						if(ultra_trace && loggers::trace()())
						{
							std::ostringstream s;
							s << "spread: " << spread << ", sample: " << last_sample_;
							loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
					}
					for( ; spread < samples_per_buffer_ ; ++spread)
					{
						samples[spread + channel] = last_sample_;
					}
				}
				if(unsigned long int error = buffer().Unlock(samples, bytes, samples2, 0)) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer unlock: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
				++current_buffer_ %= buffers_;
			}
		
			void direct_sound::do_stop() throw(universalis::operating_system::exception)
			{
				if(HRESULT error = buffer().Stop()) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer stop: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
				resource::do_stop();
			}
		
			void direct_sound::do_close() throw(universalis::operating_system::exception)
			{
				try
				{
					if(HRESULT error = buffer().Release()) throw universalis::operating_system::exceptions::runtime_error("direct sound buffer release: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
				}
				catch(...)
				{
					loggers::crash()("please ignore the crash above, i think it is a bug in your driver", UNIVERSALIS__COMPILER__LOCATION);
				}
				buffer_ = 0;
				if(HRESULT error = direct_sound_instance().Release()) throw universalis::operating_system::exceptions::runtime_error("direct sound release: " + universalis::operating_system::exceptions::code_description(error), UNIVERSALIS__COMPILER__LOCATION);
				direct_sound_ = 0;
				resource::do_close();
			}
		}
	}
}
