/***************************************************************************
*   Copyright (C) 2007 by Psycledelics   and others                    *
*   psycle.sf.net   *
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
#pragma once
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
#include "audiodriver.h"
#include "windows.h"
#include <mmsystem.h>
#include "dsound.h"
namespace psy
{
	namespace core
	{

		///\todo  this driver is (over)filling the buffer to fast

		/// output device interface implemented by direct sound.
		class MsDirectSound : public AudioDriver
		{
		public:

			MsDirectSound();

			~MsDirectSound();

			AudioDriverInfo info( ) const;
			MsDirectSound * clone( ) const;

			virtual void Initialize( AUDIODRIVERWORKFN pCallback, void * context );
			virtual void Reset();
			virtual bool Enable( bool );
			virtual int GetWritePos();
			virtual int GetPlayPos();
			int virtual GetMaxLatencyInSamples() { return settings().sampleSize() * _dsBufferSize; }
			virtual void Configure();
			virtual bool Initialized() { return _initialized; };
			virtual bool Configured() { return _configured; };


		private:
			bool _initialized;
			bool _configured;

			bool _running;
			bool _playing;
			bool _timerActive;
			//static AudioDriverEvent _event;
			//CCriticalSection _lock;

			GUID device_guid;
			bool _exclusive;
			bool _dither;
			int _bytesPerSample;
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

			static DWORD WINAPI PollerThread(void* pDirectSound);
			//																static void TimerCallback(UINT uTimerID, UINT uMsg, DWORD pDirectSound, DWORD dw1, DWORD dw2);
			void ReadConfig();
			void WriteConfig();
			void Error(const WCHAR msg[]);
			void DoBlocks();
			bool Start();
			bool Stop();

			void quantize(float *pin, int *piout, int c);
		};
	}
}
#endif
