///\file
///\brief interface file for psycle::host::WaveOut.
#pragma once
#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#include <mmsystem.h>
	#pragma comment(lib, "winmm")
#pragma warning(pop)
#include "AudioDriver.hpp"
namespace psycle
{
	namespace host
	{
		#define MAX_WAVEOUT_BLOCKS 8

		/// output device interface implemented by mme.
		class WaveOut : public AudioDriver
		{
		public:
			WaveOut();
			virtual ~WaveOut() throw();
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void * context);
			virtual void Reset();
			virtual bool Enable(bool e);	
			virtual int GetWritePos();
			virtual int GetPlayPos();
			virtual void Configure();
			virtual bool Initialized() { return _initialized; };
			virtual bool Configured() { return _configured; };
			virtual AudioDriverInfo* GetInfo() { return &_info; };
			virtual int GetNumBuffers( void ){ return _numBlocks; };
			virtual int GetBufferSize( void ){ return _blockSize; };
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
			int _numBlocks;
			int _blockSize;
			int _currentBlock;
			int _writePos;
			int _pollSleep;
			int _dither;
			bool _running;
			bool _stopPolling;
			CBlock _blocks[MAX_WAVEOUT_BLOCKS];
			void* _callbackContext;
			AUDIODRIVERWORKFN _pCallback;

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
