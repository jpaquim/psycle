///\file
///\interface psycle::host::DirectSound.
#pragma once
#include "Global.hpp"
#include "AudioDriver.hpp"

#define DIRECTSOUND_VERSION 0x8000
#include <dsound.h>
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "dsound")
	#pragma comment(lib, "dxguid")
#endif

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
				bool IsFormatSupported(WAVEFORMATEXTENSIBLE& pwfx, bool isInput);
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
			virtual void RefreshAvailablePorts();
			virtual void GetPlaybackPorts(std::vector<std::string> &ports);
			virtual void GetCapturePorts(std::vector<std::string> &ports);
			virtual bool AddCapturePort(int idx);
			virtual bool RemoveCapturePort(int idx);
			virtual bool CreateCapturePort(PortCapt &port);
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);
			static BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
			static BOOL CALLBACK DSCaptureEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
			virtual std::uint32_t GetInputLatencySamples() { return _dsBufferSize/GetSampleSizeBytes(); }
			virtual std::uint32_t GetOutputLatencySamples() { return _dsBufferSize/GetSampleSizeBytes(); }
			virtual std::uint32_t GetWritePosInSamples();
			virtual std::uint32_t GetPlayPosInSamples();
//			int virtual GetMaxLatencyInSamples() { return settings().sampleSize() * _dsBufferSize; }
			virtual void Configure();
			virtual bool Initialized() { return _initialized; }
			virtual bool Configured() { return _configured; }
			virtual AudioDriverInfo* GetInfo() { return &_info; }
		protected:
			std::uint32_t DirectSound::GetIdxFromDevice(GUID* device);
			void ReadConfig();
			void WriteConfig();
			void Error(const TCHAR msg[]);

			bool Start();
			bool Stop();
			static DWORD WINAPI NotifyThread(void* pDirectSound);
			static DWORD WINAPI PollerThread(void* pDirectSound);
			void DoBlocks();
			bool WantsMoreBlocks();
			void DoBlocksRecording(PortCapt& port);

			//Reposition the write block before the play cursor.
			void RepositionMark(int &low, int pos);


		private:
			bool _initialized;
			bool _configured;

			HWND _hwnd;
//			MMRESULT _timer;
			//controls if the driver is supposed to be running or not
			bool _running;
			//informs the real state of the DSound buffer (see the control of buffer play in DoBlocks())
			bool _playing;
			//Controls if we want the thread to be running or not
			bool _threadRun;
			static AudioDriverInfo _info;
			static AudioDriverEvent _event;
			CCriticalSection _lock;

			GUID device_guid;
			bool _dither;

			std::uint32_t _dsBufferSize;
			std::uint32_t _lowMark;
			std::uint32_t _highMark;
			/// number of "wraparounds" to compensate the GetCurrentPosition() call.
			int m_readPosWraps;

			std::vector<PortEnums> _playEnums;
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