// copyright 2004-2007 psycledelics http://psycle.sourceforge.net
/*************************************************************************
*  This program is covered by the GNU General Public License:            *
*  http://gnu.org/licenses/gpl.html                                      *
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
*  This program is distributed in the hope that it will be useful,       *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
*  See the GNU General Public License for more details.                  *
*                                                                        *
*  You should have received a copy of the GNU General Public License     *
*  along with this program; if not, write to the                         *
*  Free Software Foundation, Inc.,                                       *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
**************************************************************************/
#include "wave_file_out.hpp"
#include <iostream>
namespace psycle
{
	namespace audio_drivers
	{
		volatile int WaveFileOut::kill_thread = 0; ///\todo why volatile, why static?
		volatile int WaveFileOut::threadOpen = 0; ///\todo why volatile, why static?

		WaveFileOut::WaveFileOut()
		:
			AudioDriver(),
			kill_thread(),
			threadOpen()
			_initialized()
		{}

		WaveFileOut::~WaveFileOut()
		{
			while ( threadOpen ) {
				kill_thread = 1;					
				#if defined __unix__ ///\todo use proper synchronisation technic
					usleep(200);
				#elif defined _WIN64 || defined _WIN32
					Sleep(1);
				#else
					#error unsupported operating system
				#endif
			}
		}

		WaveFileOut * WaveFileOut::clone( ) const
		{
			return new WaveFileOut(*this);
		}

		AudioDriverInfo WaveFileOut::info( ) const
		{
			return AudioDriverInfo("wavefileout","Wave to File Driver","Recording a wav to a file",false);
		}

		void WaveFileOut::Initialize(AUDIODRIVERWORKFN pCallback, void * context )
		{
			_pCallback = pCallback;
			_callbackContext = context;
			_initialized = true;
		}

		bool WaveFileOut::Initialized(void)
		{
			return _initialized;
		}

		bool WaveFileOut::Enable( bool e )
		{
			bool threadStarted = false;
			if (e && !threadOpen) {
				kill_thread = 0;
				#if defined __unix__
					pthread_create(&threadid, NULL, (void*(*)(void*))audioOutThread, (void*) this);
				#elif defined _WIN64 || defined _WIN32
					CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)audioOutThread, this, 0, &threadid);
				#else
					#error unsupported operating system
				#endif
				threadStarted = true;
			} else
				if (!e && threadOpen) {
					kill_thread = 1;
					threadStarted = false;
					while ( threadOpen ) {
						#if defined __unix__ ///\todo use proper synchronisation technic
							usleep(10); // give thread time to close
						#elif defined _WIN64 || defined _WIN32
							Sleep(1);
						#else
							#error unsupported operating system
						#endif
					}
				}
				return threadStarted;
		}

		int WaveFileOut::audioOutThread( void * ptr )
		{
			WaveFileOut* waveFileOut = (WaveFileOut*) ptr;
			waveFileOut->writeBuffer();
		}

		void WaveFileOut::writeBuffer( )
		{
			threadOpen = 1;
			int count = 441;

			while(!(kill_thread))
			{
				#if defined __unix__ ///\todo use proper synchronisation technic
					usleep(50); // give cpu time to breath, and not too much :)
				#elif defined _WIN64 || defined _WIN32
					Sleep(1);
				#else
					#error unsupported operating system
				#endif
				float const * input(_pCallback(_callbackContext, count));
			}

			threadOpen = 0;
			std::cout << "closing thread" << std::endl;
			#if defined __unix__ ///\todo use proper synchronisation technic
				pthread_exit(0);
			#elif defined _WIN64 || defined _WIN32
				ExitThread(0);
			#else
				#error unsupported operating system
			#endif
		}
	}
}
