///\file
///\brief interface file for psycle::host::WaveOut.
#pragma once
#include "AudioDriver.hpp"

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
			virtual void Initialize(HWND hwnd, WorkFunction pCallback, void * context);
			virtual void Reset();
			virtual bool Enable(bool e);	
			virtual int GetWritePos();
			virtual int GetPlayPos();
			virtual void Configure();
			virtual bool Initialized() { return _initialized; };
			virtual bool Configured() { return _configured; };
			virtual AudioDriverInfo* GetInfo() { return &_info; };
		private:
			class CBlock
			{
			public:
				HANDLE Handle;
				byte *pData;
				WAVEHDR *pHeader;
				HANDLE HeaderHandle;
				bool Prepared;
			};
			bool _initialized;
			bool _configured;
			static AudioDriverInfo _info;
			static AudioDriverEvent _event;
			static CCriticalSection _lock;

			HWAVEOUT _handle;
			int _deviceID;
			int _currentBlock;
			int _writePos;
			int _pollSleep;
			int _dither;
			bool _running;
			bool _stopPolling;
			int const static MAX_WAVEOUT_BLOCKS = 8;
			CBlock _blocks[MAX_WAVEOUT_BLOCKS];
			void* _callbackContext;
			WorkFunction _pCallback;

			static void PollerThread(void *pWaveOut);
			void ReadConfig();
			void WriteConfig();
			void Error(const char msg[]);
			void DoBlocks();
			bool Start();
			bool Stop();
		};
	}
}
