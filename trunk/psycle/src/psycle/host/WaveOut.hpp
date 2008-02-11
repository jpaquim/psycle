// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for psycle::host::WaveOut.
#pragma once
#include <diversalis/compiler.hpp>

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

#include "AudioDriver.hpp"
#include <map>
namespace psycle
{
	namespace host
	{
		#define MAX_WAVEOUT_BLOCKS 8

		/// output device interface implemented by mme.
		class WaveOut : public AudioDriver
		{
			class PortEnums
			{
			public:
				PortEnums() {};
				PortEnums(std::string _pname):portname(_pname){}
				std::string portname;
			};
			class PortCapt
			{
			public:
				PortCapt():pleft(0),pright(0),_handle(0),_idx(0),_machinepos(0) {};

				HWAVEIN _handle;
				int _idx;
				byte *pData;
				WAVEHDR *pHeader;
				HANDLE HeaderHandle;
				bool Prepared;
				float *pleft;
				float *pright;
				int _machinepos;
			};

		public:
			WaveOut();
			virtual ~WaveOut() throw();
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void * context);
			virtual void Reset();
			virtual bool Enable(bool e);
			virtual bool Enabled() { return _running; }
			virtual void GetCapturePorts(std::vector<std::string>&ports);
			virtual bool AddCapturePort(int idx);
			virtual bool RemoveCapturePort(int idx);
			virtual bool CreateCapturePort(PortCapt &port);
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);
			virtual void EnumerateCapturePorts();

			virtual int GetInputLatency() { return _numBlocks * _blockSize; }
			virtual int GetOutputLatency() { return _numBlocks * _blockSize; }
			virtual int GetWritePos();
			virtual int GetPlayPos();
			virtual void Configure();
			virtual bool Initialized() { return _initialized; }
			virtual bool Configured() { return _configured; }
			virtual AudioDriverInfo* GetInfo() { return &_info; }
			MMRESULT IsFormatSupported(LPWAVEFORMATEX pwfx, UINT uDeviceID);
			static void PollerThread(void *pWaveOut);
		protected:
			void ReadConfig();
			void WriteConfig();
			void Error(const char msg[]);
			void DoBlocks();
			bool WantsMoreBlocks();
			void DoBlocksRecording(PortCapt& port);
			bool Start();
			bool Stop();

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
//			static CCriticalSection _lock;

			HWAVEOUT _handle;
			int _deviceID;
			int _currentBlock;
			int _writePos;
			int _pollSleep;
			int _dither;
			bool _running;
			bool _stopPolling;
			CBlock _blocks[MAX_WAVEOUT_BLOCKS];
			void* _callbackContext;
			AUDIODRIVERWORKFN _pCallback;


			std::vector<PortEnums> _capEnums;
			std::vector<PortCapt> _capPorts;
			std::map<int,int> _portMapping;

		};
	}
}
