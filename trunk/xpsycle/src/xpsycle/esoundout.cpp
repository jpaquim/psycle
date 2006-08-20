/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper, Johan Boule                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/esound_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_ESOUND
#include "esoundout.h"
#include <esd.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cassert>
namespace psycle
{
	namespace host
	{
		ESoundOut::ESoundOut()
		:
			AudioDriver(),
			initialized_(false),
			enabled_(false)
		{
		}

		ESoundOut::~ESoundOut()
		{
		}

		void ESoundOut::Initialize(AUDIODRIVERWORKFN callback, void * context)
		{
			#if !defined NDEBUG
				std::cout << "initializing esound\n";
			#endif
			callback_ = callback;
			callback_context_ = context;
			setDefault();
			open_output();
			initialized_ = true;
		}

		bool ESoundOut::Initialized( )
		{
		  return initialized_;
		}

		void ESoundOut::Configure( )
		{
			setDefault();
		}

		void ESoundOut::setDefault( )
		{
			channels_ = 2;
			bits_ = 16;
			rate_ = 44100;
		}

		bool ESoundOut::Enable(bool e)
		{
			#if !defined NDEBUG
				std::cout << "enabling esound\n";
			#endif
			//if(e) start() else stop();
			return true;
			if(e == enabled_) return true;
			if(e)
			{
				stop_requested_ = false;
				pthread_create(&thread_id_, 0, (void* (*) (void*)) thread_function_static, (void*) this);
				enabled_ = true;
			}
			else
			{
				stop_requested_ = true;
				usleep(500); // give thread time to close
			}
			assert(enabled_ = e);
			return enabled_;
		}

		int ESoundOut::bits_flag()
		{
			switch(bits_)
			{
				case 8: return ESD_BITS8; break;
				case 16: return ESD_BITS16; break;
				case 24: return ESD_BITS16; break;
			}
			return 0;
		}

		int ESoundOut::channels_flag()
		{
			switch(channels_)
			{
				case 1: return ESD_MONO; break;
				case 2: return ESD_STEREO; break;
				default: return ESD_STEREO; // no more than 2 channels
			}
			return 0;
		}

		std::string ESoundOut::host_port()
		{
			std::string nrv(std::getenv("ESPEAKER"));
			// ESD host:port
			if(port_ > 0 && host_.length())
			{
				std::ostringstream s;
				s << host_ << ":" << port_;
				nrv = s.str();
			}
			return nrv;
		}

		void ESoundOut::open_output() throw(std::exception)
		{
			esd_format_t format = ESD_STREAM | ESD_PLAY;
			format |= channels_flag();
			format |= bits_flag();
			if((output_ = esd_open_sound(host_port().c_str())) < 0)
			{
				throw std::runtime_error("failed to open esound output " + host_port());
			};
			fd_ = esd_play_stream_fallback
			(
				format,
				rate_,
				host_port().c_str(),
				"psycle"
			);
			device_buffer_ = esd_get_latency(output_);
			device_buffer_ *= bits_ / 8 * channels_;
		}

		void * ESoundOut::thread_function_static(void * object)
		{
			reinterpret_cast<ESoundOut*>(object)->thread_function();
		}
		
		void ESoundOut::thread_function()
		{
			#if !defined NDEBUG
				std::cout << "esound thread started\n";
			#endif
			while(!stop_requested_)
			{
				int sample_count(8192);
				float * input(callback_(callback_context_, sample_count));
				std::cout << "foo\n";
				usleep(1000);
			}
			enabled_ = false;
		}
		
		int ESoundOut::write_buffer(char *buffer, long size)
		{
			if(fd_ > 0) return write(fd_, buffer, size);
			else return 0;
		}
	}
}
#endif // !defined XPSYCLE__NO_ESOUND
