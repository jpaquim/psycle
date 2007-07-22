///\file
///\interface psycle::host::DirectSound.
#pragma once
#include <psycle/audiodrivers/AudioDriver.hpp>

#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include "windows.h"
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(push)
		#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#endif
	#include <mmsystem.h>
	#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
		#pragma comment(lib, "winmm")
	#endif
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(pop)
	#endif
	#include <dsound.h>
	#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
		#pragma comment(lib, "dsound")
	#endif
#else
	#error "sorry"
#endif

namespace psycle
{
	namespace host
	{
		/// output device interface implemented by direct sound.
		class DirectSound : public AudioDriver
		{
		public:
			DirectSound();
			virtual ~DirectSound() throw();
			virtual void Initialize(HWND, WorkFunction, void*);
			virtual void Reset();
			virtual bool Enable(bool);
			virtual int GetWritePos();
			virtual int GetPlayPos();
			int virtual GetMaxLatencyInSamples() { return GetSampleSize() * _dsBufferSize; }
			virtual void Configure();
			virtual bool Initialized() { return _initialized; };
			virtual bool Configured() { return _configured; };
			virtual AudioDriverInfo* GetInfo() { return &_info; };

		private:
			bool _initialized;
			bool _configured;
			static AudioDriverInfo _info;

			HWND _hwnd;
			MMRESULT _timer;
			bool _running;
			bool _playing;
			bool _timerActive;
			static AudioDriverEvent _event;
			CCriticalSection _lock;

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
			WorkFunction _pCallback;

			static void PollerThread(void* pDirectSound);
		//	static void TimerCallback(UINT uTimerID, UINT uMsg, DWORD pDirectSound, DWORD dw1, DWORD dw2);
			void ReadConfig();
			void WriteConfig();
			void Error(const char msg[]);
			void DoBlocks();
			bool Start();
			bool Stop();

		};
	}
}
