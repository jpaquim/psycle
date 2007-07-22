///\file
///\brief interface file for psycle::host::WaveOut.
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
#else
	#error "sorry"
#endif

namespace psycle
{
	namespace host
	{
		/// output device interface implemented by mme.
		class WaveOut : public AudioDriver
		{
		public:
			WaveOut();
			virtual ~WaveOut() throw();
			virtual void Initialize(HWND, WorkFunction, void*);
			virtual void Reset();
			virtual bool Enable(bool);
			virtual int GetWritePos();
			virtual int GetPlayPos();
			int virtual GetMaxLatencyInSamples() { return GetSampleSize() * _blockSize * _numBlocks; }
			virtual void Configure();
			virtual bool Initialized() { return _initialized; };
			virtual bool Configured() { return _configured; };
			virtual AudioDriverInfo* GetInfo() { return &_info; };
		private:
			static AudioDriverInfo _info;

			int _deviceID;
			HWAVEOUT _handle;

			bool _initialized;

			void ReadConfig();
			bool _configured;
			void WriteConfig();

			bool Start();
			bool _running;
			bool Stop();

			WorkFunction _pCallback;
			void* _callbackContext;
			static void PollerThread(void *pWaveOut);
			int _pollSleep;
			bool _stopPolling;
			static AudioDriverEvent _event;
			static CCriticalSection _lock;

			int const static MAX_WAVEOUT_BLOCKS = 8;
			int _numBlocks;
			int _currentBlock;
			class CBlock
			{
				public:
					HANDLE Handle;
					unsigned char *pData;
					WAVEHDR *pHeader;
					HANDLE HeaderHandle;
					bool Prepared;
			};
			CBlock _blocks[MAX_WAVEOUT_BLOCKS];

			void DoBlocks();
			int _writePos;

			int _dither;

			void Error(const char msg[]);
		};
	}
}
