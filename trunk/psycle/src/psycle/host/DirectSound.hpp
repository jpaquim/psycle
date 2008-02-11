/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\interface psycle::host::DirectSound.
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

#include <dsound.h>
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "dsound")
#endif

#include "AudioDriver.hpp"
#include <map>
namespace psycle
{
	namespace host
	{
		/// output device interface implemented by direct sound.
		class DirectSound : public AudioDriver
		{
			class PortEnums
			{
			public:
				PortEnums():guid(0) {};
				PortEnums(LPGUID _guid,std::string _pname):guid(_guid),portname(_pname){}
				std::string portname;
				LPGUID guid;
			};
			class PortCapt
			{
			public:
				PortCapt():pleft(0),pright(0),_pGuid(0),_pDs(0),_pBuffer(0),_lowMark(0),_machinepos(0) {};

				LPGUID _pGuid;
				LPDIRECTSOUNDCAPTURE8 _pDs;
				LPDIRECTSOUNDCAPTUREBUFFER8  _pBuffer;
				int _lowMark;
				float *pleft;
				float *pright;
				int _machinepos;
			};
		public:
			DirectSound();
			virtual ~DirectSound() throw();
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void * context);
			virtual void Reset();
			virtual bool Enable(bool e);
			virtual bool Enabled() { return _running; }
			virtual void GetCapturePorts(std::vector<std::string>&ports);
			virtual bool AddCapturePort(int idx);
			virtual bool RemoveCapturePort(int idx);
			virtual bool CreateCapturePort(PortCapt &port);
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);
			static BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
			virtual int GetInputLatency() { return _dsBufferSize; }
			virtual int GetOutputLatency() { return _dsBufferSize; }
			virtual int GetWritePos();
			virtual int GetPlayPos();
//			int virtual GetMaxLatencyInSamples() { return settings().sampleSize() * _dsBufferSize; }
			virtual void Configure();
			virtual bool Initialized() { return _initialized; }
			virtual bool Configured() { return _configured; }
			virtual AudioDriverInfo* GetInfo() { return &_info; }
		protected:
			void ReadConfig();
			void WriteConfig();
			void Error(const TCHAR msg[]);

			static DWORD WINAPI PollerThread(void* pDirectSound);
//	static void TimerCallback(UINT uTimerID, UINT uMsg, DWORD pDirectSound, DWORD dw1, DWORD dw2);
			void DoBlocks();
			bool WantsMoreBlocks();
			void DoBlocksRecording(PortCapt& port);
			bool WantsMoreBlocksRecording(PortCapt& port);
			bool Start();
			bool Stop();

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


			std::vector<PortEnums> _capEnums;
			std::vector<PortCapt> _capPorts;
			std::vector<int> _portMapping;

			LPDIRECTSOUND8 _pDs;
			LPDIRECTSOUNDBUFFER8 _pBuffer;
			void* _callbackContext;
			AUDIODRIVERWORKFN _pCallback;
		};
	}
}
