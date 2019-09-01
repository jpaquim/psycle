/***************************************************************************
*   Copyright (C) 2007 by Psycledelics  , Johan Boule                  *
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
#if defined PSYCLE__NET_AUDIO_AVAILABLE
#include "netaudioout.h"
#include <psycle/core/cstdint.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cerrno>
#include "cstdint.h"
namespace psy
{
	namespace core
	{

		NetAudioOut::NetAudioOut()
		:
			AudioDriver(),
			initialized_(false),
			threadRunning_(false),
			killThread_(false),
			threadId_(0),
			callback_(0),
			callbackContext_(0),
			host_(),
			port_(0),
			latency_(4096)
		{
			setDefaults();
		}

		NetAudioOut::~NetAudioOut()
		{
			Enable(false);
		}

		AudioDriverInfo NetAudioOut::info( ) const
		{
			return AudioDriverInfo("netaudio","nas sound system driver","not working",false);
		}

		void NetAudioOut::Initialize(AUDIODRIVERWORKFN callback, void * callbackContext)
		{
//												#if !defined NDEBUG
				std::cout << "xpsycle: nas: initializing\n";
//												#endif
			assert(!threadRunning_);
			callback_ = callback;
			callbackContext_ = callbackContext;
			initialized_ = true;
		}

		bool NetAudioOut::Initialized()
		{
			return initialized_;
		}

		void NetAudioOut::Configure( )
		{
			setDefaults();
		}

		void NetAudioOut::setDefaults( )
		{
			///\todo?
		}
		AudioDriver* NetAudioOut::clone() const
		{
			return new NetAudioOut(*this);
		}
		bool NetAudioOut::Enable(bool e)
		{								
//								#if !defined NDEBUG
			std::cout << "xpsycle: NetworkAudioServer: " << (e ? "en" : "dis") << "abling\n";
//								#endif
			if (e && !threadRunning_ ) {
				if (!aud_) open();
				AuStatus status= AuBadValue;
				AuStartFlow(aud_, flow_, &status);
				if (status != AuSuccess) {
					std::cout << " couldn't start the flow" << std::endl;
					return false;
				}

				killThread_ = false;
				pthread_create(&threadId_, NULL, (void*(*)(void*))audioOutThreadStatic, (void*) this);
				return true;
			} else if (!e && threadRunning_) {
				killThread_ = true;
				while(threadRunning_) usleep(1000); // give thread time to close
				close();
			}
			return false;
		}

		bool NetAudioOut::open()
		{
			if (aud_) return true;

			// 1) Open a connection with the Server
			if(!(aud_ = AuOpenServer(hostPort().c_str(),0,NULL,0,NULL,NULL)))
			{
				std::cout << "failed to open NetworkAudioServer output '" + hostPort() << std::endl;
				return false;
			}
			// 1.5)create a flow (workspace) for us.
			if (!(flow_ = AuCreateFlow(aud_, NULL)))
			{
				std::cout << "failed to create flow!" << std::endl;
				AuCloseServer(aud_);
				return false;
			}
//												handler_ = AuRegisterEventHandler(aud_, 0, 0, 0, EventHandlerFunc,(AuPointer) &(*this));

			// 2)locate an stereo output device
			AuDeviceID device = AuNone;
			for (int i=0; i < AuServerNumDevices(aud_); i++) {
				AuDeviceAttributes *dev = AuServerDevice(aud_, i);
				if ((AuDeviceKind(dev) == AuComponentKindPhysicalOutput) &&
					AuDeviceNumTracks(dev) == 2) {
						device = AuDeviceIdentifier(dev);
						std::cout << "i:" << i << " device ID :" << device << std::endl;
						break;
					}
			}
			if ( device == AuNone ) 
			{
				std::cout << "Couldn't locate a stereo output device!" << std::endl;
				AuCloseServer(aud_);
				return false;
			}

			// 3) Put the elements in the flow, prepared to work.
			AuStatus  status = AuBadValue;
			AudioDriverSettings sets;
			sets = settings();
			sets.setSamplesPerSec(48000);
			setSettings(sets);
			AuMakeElementImportClient(&nas_elements[0], settings().samplesPerSec(), toNasFormat(), 2, AuTrue,
				latency_, latency_/ 2, 0, NULL);
			AuMakeElementExportDevice(&nas_elements[1], 0, device_, settings().samplesPerSec(),
				AuUnlimitedSamples, 0, NULL);
			AuSetElements(aud_, flow_, AuTrue, 2, nas_elements, NULL);
//												if (status != AuSuccess) {
//																std::cout << "Can't set audio elements" << nas_error(aud_,status) << std::endl;
//																AuCloseServer(aud_);
//																return false;
//												}


			std::cout << " netaudio opened at " << hostPort() << std::endl;
			return true;
		}
		
		bool NetAudioOut::close()
		{
			AuStopFlow(aud_, flow_, NULL);
			AuCloseServer(aud_);
		}

		/// AUDIOSERVER host:port
		std::string NetAudioOut::hostPort()
		{
			std::string nrv;
			{
				char * env(std::getenv("AUDIOSERVER"));
				if (!env) env = std::getenv("DISPLAY");
				if (env)
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

		AuBool NetAudioOut::EventHandlerFunc(AuServer *aud, AuEvent *ev, AuEventHandlerRec *handler)
		{
		//				GlobalDataPtr   g = (GlobalDataPtr) handler->data;
			AuElementNotifyEvent *event = (AuElementNotifyEvent *) ev;

			if (ev->type == AuEventTypeElementNotify) {
			switch (event->kind)
			{
				case AuElementNotifyKindHighWater:
//																				readData(g, event);
//																				writeData(g);
					break;
				case AuElementNotifyKindLowWater:
//																				g->outBytes += event->num_bytes;
//																				writeData(g);
					break;
				case AuElementNotifyKindState:
					switch (event->cur_state)
					{
					case AuStateStop:
					//				(*g->local.callback) (g);
						break;
					case AuStatePause:
//																								readData(g, event);
//																								writeData(g);
						break;
					}
					break;
			}}

			return AuTrue;
		}

		int NetAudioOut::audioOutThreadStatic( void * ptr )
		{
			reinterpret_cast<NetAudioOut*>(ptr)->audioOutThread();
		}

		void NetAudioOut::audioOutThread()
		{
			threadRunning_ = true;
			std::cout << "xpsycle: nasdriver: device latency: " << latency_ << std::endl;
			AuStatus state;
			while(!killThread_)
			{
				float const * input(callback_(callbackContext_, latency_));
				// Stereo Hardcoded!!
				for (int i = 0; i < latency_*2; i++) {
					buf[i] = *input++;
				}
				AuWriteElement(aud_, flow_, 0, latencyInBytes(), (void*)buf, AuFalse, &state);
				/*
				* Dispose of waiting event messages as we don't implement
				* event handling for simplicity reasons.
				*/
				AuSync(aud_, AuTrue);
				if (state == AuBadValue) {
					/*
					* Does not fit in remaining buffer space,
					* wait a bit and try again
					*/
					(void)usleep(1000);
					continue;
				}
				if (state != AuSuccess )
				{
					std::cout << nas_error(aud_,state);
				}
			}
			close();
			threadRunning_ = false;
			pthread_exit(0);
		}

/*								static void NetAudioOut::writebuffer(GlobalDataPtr   g)
		{
			int             n;

			if (g->inBytes > MAX_LATENCY_BYTES)
			{
				n = g->inBytes - MAX_LATENCY_BYTES;
				g->out += n;
				g->inBytes -= n;
			}

			n = min(g->inBytes, g->outBytes);

			if (!n)
				return;

			AuWriteElement(g->remote.aud, g->remote.flow, 0, n, g->buf + g->out,
				AuFalse, NULL);

			g->out += n;
			g->outBytes -= n;
			g->inBytes -= n;

			if (!g->inBytes)
			{
				free(g->buf);
				g->buf = NULL;
				g->in = g->out = g->bufSize = 0;
			}
		}
*/
		int NetAudioOut::latencyInBytes()
		{
			int bytes(latency_);
			if ( settings().channelMode() == 3) bytes*=2;
			bytes*=(settings().bitDepth()/8);
			return bytes;
		}
		char *NetAudioOut::nas_error(AuServer* aud,AuStatus status)
		{
			static char s[100];

			AuGetErrorText(aud, status, s, sizeof(s));

			return s;
		}
		unsigned char NetAudioOut::toNasFormat()
		{
			// hardcoded to 16bits for now.
			return AuFormatLinearSigned16LSB;
		}

	}
}
#endif
