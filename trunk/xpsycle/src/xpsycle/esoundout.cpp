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
#include <cerrno>
#include <cstdint>
namespace psycle
{
	namespace host
	{

		ESoundOut::ESoundOut()
		:
			AudioDriver(),
			initialized_(false),
			threadRunning_(false),
			killThread_(false),
			callback_(0),
			callbackContext_(0),
			host_(),
			port_(0)
		{
			setDefaults();
		}

		ESoundOut::~ESoundOut()
		{
			Enable(false);
		}

		ESoundOut * ESoundOut::clone( ) const
		{
			return new ESoundOut(*this);
		}

		AudioDriverInfo ESoundOut::info( ) const
		{
			return AudioDriverInfo("esd");
		}

		void ESoundOut::Initialize(AUDIODRIVERWORKFN callback, void * callbackContext)
		{
			#if !defined NDEBUG
				std::cout << "xpsycle: esound: initializing\n";
			#endif
			assert(!threadRunning_);
			callback_ = callback;
			callbackContext_ = callbackContext;
			initialized_ = true;
		}

		bool ESoundOut::Initialized()
		{
			return initialized_;
		}

		void ESoundOut::Configure( )
		{
			setDefaults();
		}

		void ESoundOut::setDefaults( )
		{
			channels_ = 2;
			bits_ = 16;
			rate_ = 44100;
		}

		int ESoundOut::bitsFlag()
		{
			switch(bits_)
			{
				case 8: return ESD_BITS8; break;
				case 16: return ESD_BITS16; break;
				default:
					{
						std::cout << "unsupported audio bit depth: " << bits_ << " (must be 8 or 16)";
						return 0;
					}
			}
		}

		int ESoundOut::channelsFlag()
		{
			switch(channels_)
			{
				case 1: return ESD_MONO; break;
				case 2: return ESD_STEREO; break;
				default:
					{
						std::cout << "unsupported audio channel count: " << channels_ << " (must be 1 or 2)" << std::endl;
						return 0;
					}
			}
		}

		/// ESD host:port
		std::string ESoundOut::hostPort()
		{
			std::string nrv;
			{
				char * env(std::getenv("ESPEAKER"));
				if(env)
				{
					nrv = env;
					return nrv;
				}
			}
			if(port_ > 0 && host_.length())
			{
				std::ostringstream s;
				s << host_ << ":" << port_;
				nrv = s.str();
			}
			return nrv;
		}

		int ESoundOut::open()
		{
			esd_format_t format = ESD_STREAM | ESD_PLAY;
			format |= channelsFlag();
			format |= bitsFlag();
			if((output_ = esd_open_sound(hostPort().c_str())) < 0)
			{
				std::string s(std::strerror(errno));
				std::cout << "failed to open esound output '" + hostPort() + "': " + s << std::endl;
				return 0;
			}
			{
				int resume(esd_resume(output_));
			}
			esd_print_all_info(esd_get_all_info(output_));
			deviceBuffer_ = esd_get_latency(output_);
			//deviceBuffer_ *= bits_ / 8 * channels_;
			if((fd_ = esd_play_stream_fallback(format, rate_, hostPort().c_str(), "psycle")) < 0)
			{
				std::string s(std::strerror(errno));
				std::cout << "failed to open esound output '" + hostPort() + "': " + s << std::endl;
				return 0;
			}
			return 1;
		}
		
		int ESoundOut::close()
		{
			return esd_close(output_);
		}

		bool ESoundOut::Enable(bool e)
		{		
			#if !defined NDEBUG
				std::cout << "xpsycle: esound: " << (e ? "en" : "dis") << "abling\n";
			#endif
			bool threadStarted = false;
			if (e && !threadRunning_ && open() ) {
					killThread_ = false;
					pthread_create(&threadId_, NULL, (void*(*)(void*))audioOutThreadStatic, (void*) this);
					threadStarted = true;
			} else
			if (!e && threadRunning_) {
				killThread_ = true;
				while(threadRunning_) usleep(500); // give thread time to close
				threadStarted = false;
			}
			return threadStarted;
		}
		
		int ESoundOut::audioOutThreadStatic( void * ptr )
		{
			reinterpret_cast<ESoundOut*>(ptr)->audioOutThread();
		}

		void ESoundOut::audioOutThread()
		{
			threadRunning_ = true;
			std::cout << "xpsycle: esound: device buffer: " << deviceBuffer_ << std::endl;
			int deviceBufferSamples(deviceBuffer_ * (3 - channels_) * 44100 / rate_);
			std::cout << "xpsycle: esound: device buffer: samples: " << deviceBufferSamples << std::endl;
			if (bits_ == 16) {
				int bufSize = deviceBuffer_ / 2;
				std::int16_t buf[bufSize];
				int newCount = bufSize / channels_;
				while(!killThread_)
				{
 					float const * input(callback_(callbackContext_, newCount));
					for (int i = 0; i < bufSize; i++) {
						buf[i] = *input++;
					}
					if(write(fd_, buf, sizeof buf) < 0) std::cout << "xpsycle: esound: write failed.\n";
				}
			} else {
				std::uint8_t buf[deviceBufferSamples];
				int bytes(sizeof buf);
				int samples(bytes);
				while(!killThread_)
				{
					float const * input(callback_(callbackContext_, samples));
					for (int i(0); i < samples; ++i) buf[i] = *input++ / 256 + 128;
					if(write(fd_, buf, bytes) < 0) std::cout << "xpsycle: esound: write failed.\n";
				}
			}
			close();
			threadRunning_ = false;
			pthread_exit(0);
		}

	} // end of psycle host
}



 // end of psycle namespace

#endif // !defined XPSYCLE__NO_ESOUND
