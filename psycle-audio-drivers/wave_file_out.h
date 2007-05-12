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
#ifndef PSYCLE__AUDIO_DRIVERS__WAVE_FILE_OUT
#define PSYCLE__AUDIO_DRIVERS__WAVE_FILE_OUT
#ifdef __unix__
	#include <unistd.h>
#elif defined _WIN64 || defined _WIN32
	#include <windows.h>
#else
	#error unsupported operating system
#endif
#include "audio_driver.h"
namespace psycle
{
	namespace audio_drivers
	{
		/**
		@author Psycledelics  
		*/
		class WaveFileOut : public AudioDriver
		{
		public:
			WaveFileOut();
			~WaveFileOut();
			/// uses the copy constructor
			virtual WaveFileOut* clone()  const;
			virtual AudioDriverInfo info() const;
			virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
			virtual bool Initialized();
			/// starts stops file writing
			virtual bool Enable(bool e);
		private:
			/// player callback
			void* _callbackContext;
			AUDIODRIVERWORKFN _pCallback;

			#ifdef __unix__
				pthread_t threadid;
			#elif __defined _WIN64 || defined _WIN32
				DWORD threadid;
			#else
				#error unsupported operating system
			#endif

			static volatile int kill_thread; ///\todo why volatile, why static?
			static volatile int threadOpen; ///\todo why volatile, why static?
			bool _initialized;
			static int audioOutThread(void * ptr);
			void writeBuffer();
		};
	}
}
#endif
