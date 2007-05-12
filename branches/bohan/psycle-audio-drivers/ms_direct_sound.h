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
#if !defined _WIN64 && !defined _WIN32
	#error unsupported operating system
#endif
#include <windows.h>
#include <mmsystem.h> ///\ todo is this one needed?
#if !defined __GNUG__
	#pragma comment(lib, "winmm")
#endif
#include "dsound.h" ///\ todo shouldn't be in the same dir
//#include <dsound.h>
#if !defined __GNUG__
	#pragma comment(lib, "dsound")
#endif
#include <cwchar>
#include "audio_driver.h"
namespace psycle
{
	namespace audio_drivers
	{
		/// output device interface implemented by direct sound.
		///\todo BUG! this driver is (over)filling the buffer too fast
		class MicrosoftDirectSound : public AudioDriver
		{
			public:
				MicrosoftDirectSound();
				~MicrosoftDirectSound();
				
				AudioDriverInfo info( ) const;
				MicrosoftDirectSound * clone( ) const;
				
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

				DIRECTSOUND * _pDs;
				DIRECTSOUNDBUFFER * _pBuffer;
				void* _callbackContext;
				AUDIODRIVERWORKFN _pCallback;

				static DWORD WINAPI PollerThread(void* pDirectSound);
				void ReadConfig();
				void WriteConfig();
				void Error(const wchar_t msg[]);
				void DoBlocks();
				bool Start();
				bool Stop();

				void quantize(float *pin, int *piout, int c);
		};
	}
}
#endif
