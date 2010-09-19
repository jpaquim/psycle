///\file
///\brief interface file for psycle::host::AudioDriver.
#pragma once
#include "Global.hpp"
#include "AudioDriver.hpp"
#include <MMReg.h>  // must be before other Wasapi headers
#include <audioclient.h>


#define MAX_STR_LEN 512

interface IMMDevice;
interface IMMDeviceEnumerator;
interface IMMDeviceCollection;
namespace psycle
{
	namespace host
	{
		/// output device interface.
		class WasapiDriver : public AudioDriver
		{
			class PortEnum
			{
			public:
				PortEnum() {};

				// from GetId
				WCHAR szDeviceID[MAX_STR_LEN];
			    
				// from PropVariant
				char portName[MAX_STR_LEN];

				// Fields filled from IMMEndpoint'sGetDataFlow
				//EDataFlow flow;

				// Fields filled from IAudioDevice (_prior_ to Initialize)
				// from GetDevicePeriod(
				REFERENCE_TIME DefaultDevicePeriod;
				REFERENCE_TIME MinimumDevicePeriod;

				// from GetMixFormat
				// WAVEFORMATEX *MixFormat;//needs to be CoTaskMemFree'd after use!

				// Default format (setup through Control Panel by user)
				WAVEFORMATEXTENSIBLE DefaultFormat;
			};
			// ------------------------------------------------------------------------------------------
			/* PaWasapiStream - a stream data structure specifically for this implementation */
			typedef struct PaWasapiSubStream
			{
				// Device
				IMMDevice			*device;
				IAudioClient        *client;
				WAVEFORMATEXTENSIBLE wavex;
				UINT32               bufferSize;
				REFERENCE_TIME       device_latency;
				REFERENCE_TIME       period;
				double				 latency_seconds;
				UINT32				 framesPerHostCallback;
				AUDCLNT_SHAREMODE    shareMode;
				UINT32               streamFlags; // AUDCLNT_STREAMFLAGS_EVENTCALLBACK, ...
				UINT32               flags;

				// Used by blocking interface:
				UINT32               prevTime;  // time ms between calls of WriteStream
				UINT32               prevSleep; // time ms to sleep from frames written in previous call
			}
			PaWasapiSubStream;
		public:
			WasapiDriver();
			virtual ~WasapiDriver();
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context);
			virtual void Reset(void);
			virtual bool Enable(bool e) { return e ? Start() : Stop(); }
			virtual bool Enabled() { return running; }
			virtual void GetCapturePorts(std::vector<std::string> &ports);
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);
			virtual bool AddCapturePort(int idx);
			virtual bool RemoveCapturePort(int idx);
			virtual int GetWritePos();
			virtual int GetPlayPos();
			virtual int GetInputLatency();
			virtual int GetOutputLatency();
			virtual void Configure(void);
			virtual bool Initialized(void) {return _initialized; }
			virtual bool Configured(void);
			virtual AudioDriverInfo* GetInfo() { return &_info; }
		private:
			void RefreshPorts(IMMDeviceEnumerator *pEnumerator);
			void FillPortList(std::vector<PortEnum> portList, IMMDeviceCollection *pCollection);
			void ReadConfig();
			void WriteConfig();
			bool Start();
			bool Stop();
			static DWORD WINAPI AudioThread(void* pWasapi);
			HRESULT GetStreamFormat(WAVEFORMATEXTENSIBLE* pwfx);

			static AudioDriverInfo _info;

			void* _callbackContext;
			AUDIODRIVERWORKFN _pCallback;

			// input
			PaWasapiSubStream in;
			
			// output
			PaWasapiSubStream out;

			// must be volatile to avoid race condition on user query while
			// thread is being started
			volatile bool running;

			DWORD dwThreadId;
			HANDLE hThread;
			HANDLE hCloseRequest;
			HANDLE hThreadDone;
			HANDLE hBlockingOpStreamRD;
			HANDLE hBlockingOpStreamWR;

			// Defines blocking/callback interface used
			bool bBlocking;

			// Av Task (MM thread management)
			HANDLE hAvTask;

			bool _initialized;

			std::vector<PortEnum> _playEnums;
			std::vector<PortEnum> _capEnums;
//			std::vector<PortCapt> _capPorts;
			std::vector<int> _portMapping;

		};
	}
}