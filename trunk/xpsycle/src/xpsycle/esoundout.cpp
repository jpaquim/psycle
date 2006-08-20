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

		void ESoundOut::Initialize(AUDIODRIVERWORKFN callback, void * callbackContext)
		{
			#if !defined NDEBUG
				std::cout << "initializing esound\n";
			#endif
			assert(!threadRunning_);
			open();
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
				case 24: return ESD_BITS16; break;
			}
			return 0;
		}

		int ESoundOut::channelsFlag()
		{
			switch(channels_)
			{
				case 1: return ESD_MONO; break;
				case 2: return ESD_STEREO; break;
				default: return ESD_STEREO; // no more than 2 channels
			}
		}

		std::string ESoundOut::hostPort()
		{
			std::string nrv;
			{
				char * env(std::getenv("ESPEAKER"));
				if(env) nrv = env;
			}
			// ESD host:port
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
			if((output_ = esd_open_sound(hostPort().c_str())) <= 0)
			{
				throw std::runtime_error("failed to open esound output '" + hostPort() + "'");
			};
			fd_ = esd_play_stream_fallback
			(
				format,
				rate_,
				hostPort().c_str(),
				"psycle"
			);
			deviceBuffer_ = esd_get_latency(output_);
			deviceBuffer_ *= bits_ / 8 * channels_;
		}
		
		void ESoundOut::close() throw(std::exception)
		{
			///\todo
		}

		bool ESoundOut::Enable(bool e)
		{
			#if !defined NDEBUG
				std::cout << (e ? "en" : "dis") << "abling esound\n";
			#endif
			bool threadStarted = false;
			if (e && !threadRunning_) {
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

			int count = 8192;

			while(!killThread_)
			{
				usleep(100); // give cpu time to breath

				float const * input(callback_(callbackContext_, count));
				std::cout << "spam me" << std::endl;
			}

			threadRunning_ = false;
			pthread_exit(0);
		}

	} // end of psycle host
} // end of psycle namespace

#endif // !defined XPSYCLE__NO_ESOUND
