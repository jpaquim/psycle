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
#pragma once

#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
#include "audiodriver.h"
#include <windows.h>
#include <mmsystem.h>
#undef min
#undef max
#include "dsound.h"
#include "cstdint.h"
#include "helpers.h"
#include <iostream>

namespace psycle
{
	namespace audio_drivers
	{
		class MicrosoftDirectSoundOut : public AudioDriver
		{
		public:
			MicrosoftDirectSoundOut();
			virtual ~MicrosoftDirectSoundOut();
			virtual MicrosoftDirectSoundOut* clone()  const; // Uses the copy constructor

			virtual AudioDriverInfo info() const;

			void Initialize(AUDIODRIVERWORKFN pCallback, void * context );
			virtual bool Initialized() { return _initialized; };
			virtual bool Enable(bool);
			virtual void configure();

			//virtual void Reset();
			virtual int GetWritePos();
			virtual int GetPlayPos();
			//int virtual GetMaxLatencyInSamples() { return GetSampleSize() * _dsBufferSize; }
			//virtual bool Configured() { return _configured; };


		private:
			bool _initialized;
			//bool _configured;
			static AudioDriverInfo _info;
			bool _running;
			bool _playing;

			HANDLE _hThread;
			//HWND _hwnd;
			//MMRESULT _timer;
			//bool _timerActive;
			//static AudioDriverEvent _event;
			//CCriticalSection _lock;

			GUID device_guid;
			bool _exclusive;
			bool _dither;
			int _bufferSize;
			int _numBuffers;

			int _dsBufferSize;
			int _currentOffset;
			int _lowMark;
			int _highMark;
			int _buffersToDo;

			LPDIRECTSOUND _pDs;
			LPDIRECTSOUNDBUFFER _pBuffer;
			void* _callbackContext;
			AUDIODRIVERWORKFN _pCallback;

			bool Start();
			bool Stop();
			static DWORD WINAPI audioOutThread(void * pDirectSound);
			void DoBlocks();
			//void ReadConfig();
			//void WriteConfig();
			void Error(const char msg[]);
		};
	}
}

#endif
