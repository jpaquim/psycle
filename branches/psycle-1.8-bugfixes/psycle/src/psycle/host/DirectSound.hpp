///\file
///\interface psycle::host::DirectSound.
#pragma once
#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#include <mmsystem.h>
	#pragma comment(lib, "winmm")
#pragma warning(pop)
#include <dsound.h>
#pragma comment(lib, "dsound")
#include "AudioDriver.hpp"
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
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void * context);
			virtual void Reset();
			virtual bool Enable(bool e);
			virtual int GetWritePos();
			virtual int GetPlayPos();
//			int virtual GetMaxLatencyInSamples() { return settings().sampleSize() * _dsBufferSize; }
			virtual void Configure();
			virtual bool Initialized() { return _initialized; };
			virtual bool Configured() { return _configured; };
			virtual AudioDriverInfo* GetInfo() { return &_info; };

		private:
			bool _initialized;
			bool _configured;
			static AudioDriverInfo _info;

			HWND _hwnd;
//			MMRESULT _timer;
			bool _running;
			bool _playing;
			bool _threadRun;
			static AudioDriverEvent _event;
			CCriticalSection _lock;

			GUID device_guid;
			bool _exclusive;
			bool _dither;
			int _bytesPerSample;
			int _bufferSize;
			int _numBuffers;

			int _dsBufferSize;
			int _runningBufSize;
			int _lowMark;
			int _highMark;
			int _buffersToDo;

			LPDIRECTSOUND8 _pDs;
			LPDIRECTSOUNDBUFFER8 _pBuffer;
			void* _callbackContext;
			AUDIODRIVERWORKFN _pCallback;

			static DWORD WINAPI PollerThread(void* pDirectSound);
		//	static void TimerCallback(UINT uTimerID, UINT uMsg, DWORD pDirectSound, DWORD dw1, DWORD dw2);
			void ReadConfig();
			void WriteConfig();
			void Error(const TCHAR msg[]);
			void DoBlocks();
			bool WantsMoreBlocks();
			bool Start();
			bool Stop();
		};
	}
}
