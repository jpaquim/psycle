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
			close();
		}

		AudioDriverInfo ESoundOut::info( ) const
		{
			return AudioDriverInfo("esd");
		}

		void ESoundOut::Initialize(AUDIODRIVERWORKFN callback, void * callbackContext)
		{
			#if !defined NDEBUG
				std::cout << "initializing esound\n";
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

		int ESoundOut::bitsFlag() throw(std::exception)
		{
			switch(bits_)
			{
				case 8: return ESD_BITS8; break;
				case 16: return ESD_BITS16; break;
				default:
					{
						std::ostringstream s;
						s << "unsupported audio bit depth: " << bits_ << " (must be 8 or 16)";
						throw std::runtime_error(s.str());
					}
			}
		}

		int ESoundOut::channelsFlag() throw(std::exception)
		{
			switch(channels_)
			{
				case 1: return ESD_MONO; break;
				case 2: return ESD_STEREO; break;
				default:
					{
						std::ostringstream s;
						s << "unsupported audio channel count: " << channels_ << " (must be 1 or 2)";
						throw std::runtime_error(s.str());
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

		void ESoundOut::open() throw(std::exception)
		{
			esd_format_t format = ESD_STREAM | ESD_PLAY;
			format |= channelsFlag();
			format |= bitsFlag();
			if((output_ = esd_open_sound(hostPort().c_str())) < 0)
			{
				std::string s(std::strerror(errno));
				std::cout << "failed to open esound output '" + hostPort() + "': " + s << std::endl;
				throw std::runtime_error("failed to open esound output '" + hostPort() + "': " + s);
			}
			deviceBuffer_ = esd_get_latency(output_);
			//deviceBuffer_ *= bits_ / 8 * channels_;
			if((fd_ = esd_play_stream_fallback(format, rate_, hostPort().c_str(), "psycle")) < 0)
			{
				std::string s(std::strerror(errno));
				throw std::runtime_error("failed to open esound output stream '" + hostPort() + "': " + s);
			}
		}
		
		void ESoundOut::close() throw(std::exception)
		{
			esd_close(output_);
		}

		bool ESoundOut::Enable(bool e)
		{		
			#if !defined NDEBUG
				std::cout << (e ? "en" : "dis") << "abling esound\n";
			#endif
			bool threadStarted = false;
			if (e && !threadRunning_) {
					open();
					killThread_ = false;
					pthread_create(&threadId_, NULL, (void*(*)(void*))audioOutThreadStatic, (void*) this);
					threadStarted = true;
			} else
			if (!e && threadRunning_) {
				killThread_ = true;
				usleep(500); // give thread time to close
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

			if (bits_ == 16) {
				std::cout << deviceBuffer_ << std::endl;
				short buf[deviceBuffer_]; /* really should be same size as latency buffer */
				int count = sizeof(buf);
	
				while(!killThread_)
				{
					//int newCount =  count/(2*channels_);
					int newCount = deviceBuffer_;// / (2*channels_);
 					float const * input(callback_(callbackContext_, newCount));
					for (int i = 0; i < newCount; i++) {
						buf[i] = *input++;
					}
					write(fd_, buf,  sizeof(buf));
				}
			} else {
				unsigned char buf[8192];
				int count = sizeof(buf);
	
				while(!killThread_)
				{
          /*int newCount = count/ channels_;
					float const * input(callback_(callbackContext_, newCount));
					for (int i = 0; i < count; i++) {
						buf[i] = *input++;
						buf[i] ^= 0x80;
					}
					write(fd_, buf, sizeof(buf));*/
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
