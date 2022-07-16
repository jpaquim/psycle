// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// linking
#include "../../detail/prefix.h"

#include "../audiodriversettings.h"
#include "avrt.h"
#include <quantize.h>
#include <operations.h>

#include "../audiodriver.h"
#include "../../detail/psydef.h"
#include "clip.hpp"

#include <windows.h>
#include <stdio.h>
#include <hashtbl.h>
#include <list.h>

#include <ks.h>
#include <ksmedia.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "../../detail/portable.h"

#define MAX_STR_LEN 512
#define PSY_AUDIODRIVER_WASAPI_GUID 0x0004

#if !defined SHORT_MIN
#define SHORT_MIN -32768
#endif
#if !defined SHORT_MAX
#define SHORT_MAX 32767
#endif

interface IMMDevice;
interface IMMDeviceEnumerator;
interface IMMDeviceCollection;

// WASAPI
#include <mmreg.h>  // must be before other Wasapi headers
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define COBJMACROS
#include <Audioclient.h>
#include <endpointvolume.h>
#define INITGUID //<< avoid additional linkage of static libs, uneeded code will be optimized out
#include <mmdeviceapi.h>
#include <functiondiscoverykeys.h>
#undef INITGUID
#endif

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) { goto Exit; }

#define SAFE_RELEASE(punk) \
	if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }



namespace psycle {
	namespace helpers {
		namespace math {
			/******************************************************************************************/
			/// C1999 *round* - converts a floating point number to an integer by rounding to the nearest integer.
			/// This function has the same semantic as C1999's *round* series of functions,
			/// but with C++ overload support, we don't need different names for each type.
			/// note: it is unspecified whether rounding x.5 rounds up, down or towards the even integer.
			template<typename IntegralResult, typename Real> // UNIVERSALIS__COMPILER__CONST_FUNCTION
			IntegralResult inline round(Real x) noexcept {
				// check that the Result type is an integral number
				// BOOST_STATIC_ASSERT((std::numeric_limits<IntegralResult>::is_integer));

				return x > 0 ? std::floor(x + Real(0.5)) : std::ceil(x - Real(0.5));
			}
		}
	}
}

namespace psycle
{
	namespace host
	{

		static void PrepareWaveFormat(WAVEFORMATEXTENSIBLE* wf, int channels, int sampleRate, int bits, int validBits);
		//In -> -32768.0..32768.0 , out -32768..32767
		static void Quantize16(float* pin, int* piout, int c);
		//In -> -32768.0..32768.0 , out -32768..32767
		static void Quantize16WithDither(float* pin, int* piout, int c);
		//In -> -8388608.0..8388608.0, out  -2147483648.0 to 2147483648.0
		static void Quantize24in32Bit(float* pin, int* piout, int c);
		//In -> -8388608.0..8388608.0 in 4 bytes, out -8388608..8388608, aligned to 3 bytes, Big endian
		static void Quantize24BE(float* pin, int* piout, int c);
		//In -> -8388608.0..8388608.0 in 4 bytes, out -8388608..8388608, aligned to 3 bytes, little endian
		static void Quantize24LE(float* pin, int* piout, int c);
		//In -> -32768..32767 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
		static void DeQuantize16AndDeinterlace(short int* pin, float* poutleft, float* poutright, int c);
		//In -> -2147483648..2147483647 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
		static void DeQuantize32AndDeinterlace(int* pin, float* poutleft, float* poutright, int c);
		//In -> -1.0..1.0 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
		static void DeinterlaceFloat(float* pin, float* poutleft, float* poutright, int c);
		/// output device interface.

		const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
		const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
		const IID IID_IAudioClient = __uuidof(IAudioClient);
		const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
		const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
		const IID IID_IAudioClock = __uuidof(IAudioClock);
		
		

		class WasapiDriver
		{			

			static void Error(const char msg[])
			{
				::MessageBox(0, msg, "Windows WASAPI driver", MB_OK | MB_ICONERROR);
			}

			class PortEnum
			{
			public:
				PortEnum() {};
				bool IsFormatSupported(WAVEFORMATEXTENSIBLE& pwfx, AUDCLNT_SHAREMODE sharemode) const;
				// from GetId
				WCHAR szDeviceID[MAX_STR_LEN];
				// from PropVariant
				char portName[MAX_STR_LEN];

				REFERENCE_TIME DefaultDevicePeriod;
				REFERENCE_TIME MinimumDevicePeriod;

				WAVEFORMATEXTENSIBLE MixFormat;
				bool isDefault;
			};
			// ------------------------------------------------------------------------------------------
			typedef struct PaWasapiSubStream
			{
				// Device
				WCHAR szDeviceID[MAX_STR_LEN];
				IMMDevice* device;
				IAudioClient* client;
				WAVEFORMATEXTENSIBLE wavex;
				UINT32               bufferFrameCount;
				REFERENCE_TIME       device_latency;
				REFERENCE_TIME       period;
				AUDCLNT_SHAREMODE    shareMode;
				UINT32               streamFlags; // AUDCLNT_STREAMFLAGS_EVENTCALLBACK, ...
				UINT32               flags;
				float* pleft;
				float* pright;
				int _machinepos;
			}
			PaWasapiSubStream;
		public:
			WasapiDriver();
			virtual ~WasapiDriver();
			virtual const psy_AudioDriverSettings* settings() const { return &this->settings_; }

			virtual void Initialize(AUDIODRIVERWORKFN pCallback, void* context);
			virtual bool Enable(bool e) { return e ? Start() : Stop(); }
			virtual void Reset(void);
			virtual void Configure(void);
			psy_List* BufferList(int* prefindex);
			virtual bool Initialized(void) const { return _initialized; }
			virtual bool Enabled() const { return running; }
			virtual void GetPlaybackPorts(std::vector<std::string>& ports) const;
			virtual void GetCapturePorts(std::vector<std::string>& ports) const;
			virtual void GetReadBuffers(int idx, float** pleft, float** pright, int numsamples);
			virtual bool AddCapturePort(int idx);
			virtual bool RemoveCapturePort(int idx);
			virtual	void RefreshAvailablePorts();
			virtual uint32_t GetWritePosInSamples() const;
			virtual uint32_t GetPlayPosInSamples();
			virtual uint32_t GetInputLatencyMs() const;
			virtual uint32_t GetOutputLatencyMs() const;
			virtual uint32_t GetInputLatencySamples() const;
			virtual uint32_t GetOutputLatencySamples() const;
			unsigned int GetBufferSamples()
			{
				return psy_audiodriversettings_blockframes(&settings_);
			}

		private:			
			static const char* GetError(HRESULT hr);
			void RefreshPorts(IMMDeviceEnumerator* pEnumerator);
			void FillPortList(std::vector<PortEnum>& portList, IMMDeviceCollection* pCollection, LPWSTR defaultID);
			uint32_t GetIdxFromDevice(WCHAR* szDeviceID) const;
			bool Start();
			bool Stop();
			static DWORD WINAPI EventAudioThread(void* pWasapi);
			HRESULT CreateCapturePort(IMMDeviceEnumerator* pEnumerator, PaWasapiSubStream& port);
			HRESULT GetStreamFormat(PaWasapiSubStream& stream, WAVEFORMATEXTENSIBLE* pwfx);
			HRESULT DoBlock(IAudioRenderClient* pRenderClient, int numFramesAvailable);
			HRESULT DoBlockRecording(PaWasapiSubStream& port, IAudioCaptureClient* pCaptureClient, int numFramesAvailable);
			
			// static AudioDriverEvent _event;
//			void* _callbackContext;
	//		AUDIODRIVERWORKFN _pCallback;

			bool _initialized;
			bool _configured;
		public:
			std::vector<PortEnum> _playEnums;
			std::vector<PortEnum> _capEnums;
			psy_AudioDriverSettings settings_;
			static AUDIODRIVERWORKFN _pCallback;
			static void* _callbackContext;
			WCHAR szDeviceID[MAX_STR_LEN];
			uintptr_t driverindex;
			bool shared;
			HANDLE hEvent;
		private:
			std::vector<PaWasapiSubStream> _capPorts;
			std::vector<int> _portMapping;
			// output
			PaWasapiSubStream out;
			IAudioClock* pAudioClock;
			UINT64 audioClockFreq;
			uint32_t writeMark;

			// must be volatile to avoid race condition on user query while
			// thread is being started
			volatile bool running;

			DWORD dwThreadId;
		};

		AUDIODRIVERWORKFN WasapiDriver::_pCallback(0);
		void* WasapiDriver::_callbackContext(0);
	}
}


namespace portaudio {
	// ------------------------------------------------------------------------------------------
	// Aligns v backwards
	static inline UINT32 ALIGN_BWD(UINT32 v, UINT32 align)
	{
		return ((v - (align ? v % align : 0)));
	}
	// ------------------------------------------------------------------------------------------
	static inline double nano100ToMillis(const REFERENCE_TIME& ref)
	{
		// 1 nano = 0.000000001 seconds
		//100 nano = 0.0000001 seconds
		//100 nano = 0.0001 milliseconds
		return ((double)ref) * 0.0001;
	}
	// ------------------------------------------------------------------------------------------
	static inline REFERENCE_TIME MillisTonano100(const double& ref)
	{
		// 1 nano = 0.000000001 seconds
		//100 nano = 0.0000001 seconds
		//100 nano = 0.0001 milliseconds
		return (REFERENCE_TIME)(ref / 0.0001);
	}
	// ------------------------------------------------------------------------------------------
	// Makes Hns period from frames and sample rate
	static inline REFERENCE_TIME MakeHnsPeriod(UINT32 nFrames, DWORD nSamplesPerSec)
	{
		return (REFERENCE_TIME)((10000.0 * 1000 / nSamplesPerSec * nFrames) + 0.5);
	}
	// ------------------------------------------------------------------------------------------
	// Aligns WASAPI buffer to 128 byte packet boundary. HD Audio will fail to play if buffer
	// is misaligned. This problem was solved in Windows 7 were AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED
	// is thrown although we must align for Vista anyway.
	static inline UINT32 AlignFramesPerBuffer(UINT32 nFrames, UINT32 nSamplesPerSec, UINT32 nBlockAlign)
	{
#define HDA_PACKET_SIZE 128

		long packets_total = 10000 * (nSamplesPerSec * nBlockAlign / HDA_PACKET_SIZE);
		long frame_bytes = nFrames * nBlockAlign;

		if (frame_bytes % HDA_PACKET_SIZE != 0) {
			//Ensure we round up, not down
			frame_bytes = ALIGN_BWD(frame_bytes + HDA_PACKET_SIZE, HDA_PACKET_SIZE);
		}

		// align to packet size
		nFrames = frame_bytes / nBlockAlign;
		long packets = frame_bytes / HDA_PACKET_SIZE;

		// align to packets count
		while (packets && ((packets_total % packets) != 0))
		{
			//round up
			++packets;
		}

		frame_bytes = packets * HDA_PACKET_SIZE;
		nFrames = frame_bytes / nBlockAlign;

		return nFrames;
	}
	// ------------------------------------------------------------------------------------------
	// Converts Hns period into number of frames
	static inline UINT32 MakeFramesFromHns(REFERENCE_TIME hnsPeriod, UINT32 nSamplesPerSec)
	{
		UINT32 nFrames = (UINT32)( // frames =
			1.0 * hnsPeriod * // hns *
			nSamplesPerSec / // (frames / s) /
			1000 / // (ms / s) /
			10000 // (hns / s) /
			+ 0.5 // rounding
			);
		return nFrames;
	}
}

///\file
///\brief implementation file for psycle::host::AudioDriver.

// Portions of this code belongs to portaudio's WASAPI driver.
// Portable Audio I/O Library WASAPI implementation
// Copyright (c) 2006-2010 David Viens, Dmitry Kostjuchenko

// http://msdn.microsoft.com/en-us/library/dd370844%28v=VS.85%29.aspx
// http://msdn.microsoft.com/en-us/library/dd316756%28v=VS.85%29.aspx
// http://msdn.microsoft.com/en-us/library/dd370876%28v=VS.85%29.aspx


#define INITGUID //<< avoid additional linkage of static libs, uneeded code will be optimized out
#include <mmdeviceapi.h>
#include <functiondiscoverykeys.h>
#undef INITGUID

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) { goto Exit; }

#define SAFE_RELEASE(punk) \
	if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }


namespace psycle
{
	namespace host
	{

		bool WasapiDriver::PortEnum::IsFormatSupported(WAVEFORMATEXTENSIBLE& pwfx, AUDCLNT_SHAREMODE sharemode) const
		{
			IMMDeviceEnumerator* pEnumerator = NULL;
			IMMDevice* device = NULL;
			IAudioClient* client = NULL;
			bool issuccess = false;
			HRESULT hr;
			WAVEFORMATEX* bla = NULL;

			hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
				IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			EXIT_ON_ERROR(hr)

				hr = pEnumerator->GetDevice(szDeviceID, &device);
			EXIT_ON_ERROR(hr)

				hr = device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client);
			EXIT_ON_ERROR(hr)

				hr = client->IsFormatSupported(sharemode, reinterpret_cast<WAVEFORMATEX*>(&pwfx),
					(sharemode == AUDCLNT_SHAREMODE_SHARED) ? &bla : NULL);
			if (bla != NULL) { ::CoTaskMemFree(bla); }
			if (hr == S_OK) issuccess = true;
		Exit:
			SAFE_RELEASE(client)
				SAFE_RELEASE(device)
				SAFE_RELEASE(pEnumerator)
				return issuccess;
		}
		////////////////////////////////////////////////////
		WasapiDriver::WasapiDriver() :
			running(false)
			, _initialized(false)
			, _configured(false)
			, writeMark(0)
			, dwThreadId(0)
			, pAudioClock(0)
			, shared(TRUE)
			, driverindex(0)
		{
			ZeroMemory(&out, sizeof(PaWasapiSubStream));
			psy_audiodriversettings_init(&settings_);
			psy_audiodriversettings_setvalidbitdepth(&settings_, 24);
			psy_audiodriversettings_setblockcount(&settings_, 2);
			psy_audiodriversettings_setblockframes(&settings_, 2048);
			shared = true;
			wcscpy_s(szDeviceID, MAX_STR_LEN - 1, L"");
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		}

		WasapiDriver::~WasapiDriver()
		{
			Stop();
			CloseHandle(hEvent);
		}

		void WasapiDriver::Initialize(AUDIODRIVERWORKFN pCallback, void* context)
		{
			WasapiDriver::_callbackContext = context;
			_pCallback = pCallback;
			running = false;
			RefreshAvailablePorts();
			_initialized = true;
		}
		void WasapiDriver::Reset(void) {
			Stop();
		}

		bool WasapiDriver::Start() {
			HRESULT hr;
			IMMDeviceEnumerator* pEnumerator = NULL;
			wcscpy_s(out.szDeviceID, MAX_STR_LEN - 1, szDeviceID);
			out.streamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
			out.shareMode = shared ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE;
			if (running) return true;
			if (!_pCallback) return false;

			hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
				IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			EXIT_ON_ERROR(hr)

				if (wcscmp(out.szDeviceID, L"") == 0) {
					hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &out.device);
				} else {
					hr = pEnumerator->GetDevice(out.szDeviceID, &out.device);
				}
			EXIT_ON_ERROR(hr)

				hr = out.device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&out.client);
			EXIT_ON_ERROR(hr)

				hr = GetStreamFormat(out, &out.wavex);
			EXIT_ON_ERROR(hr)

				UINT32 framesPerLatency;
			if (out.shareMode == AUDCLNT_SHAREMODE_SHARED)
			{
				hr = out.client->GetDevicePeriod(&out.period, NULL);
				EXIT_ON_ERROR(hr)

					framesPerLatency = portaudio::MakeFramesFromHns(out.period, out.wavex.Format.nSamplesPerSec);
				psy_audiodriversettings_setblockcount(&settings_, 2);
			} else {
				framesPerLatency = GetBufferSamples();
				if (framesPerLatency == 0) {
					hr = out.client->GetDevicePeriod(NULL, &out.period);
					EXIT_ON_ERROR(hr)
						// Add latency frames
						framesPerLatency = portaudio::MakeFramesFromHns(out.period, out.wavex.Format.nSamplesPerSec);
				}
				psy_audiodriversettings_setblockcount(&settings_, 2);
			}
			// Align frames to HD Audio packet size of 128 bytes 
			framesPerLatency = portaudio::AlignFramesPerBuffer(framesPerLatency,
				out.wavex.Format.nSamplesPerSec, out.wavex.Format.nBlockAlign);

			// Calculate period
			out.period = portaudio::MakeHnsPeriod(framesPerLatency, out.wavex.Format.nSamplesPerSec);
			psy_audiodriversettings_setblockframes(&settings_, framesPerLatency);

			hr = out.client->Initialize(
				out.shareMode,
				out.streamFlags,
				out.period,
				(out.shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? out.period : 0),
				reinterpret_cast<WAVEFORMATEX*>(&out.wavex),
				NULL);
			EXIT_ON_ERROR(hr)

				hr = out.client->GetBufferSize(&out.bufferFrameCount);
			EXIT_ON_ERROR(hr)
				psy_audiodriversettings_setblockframes(&settings_, out.bufferFrameCount);


			hr = out.client->GetService(IID_IAudioClock, (void**)&pAudioClock);
			EXIT_ON_ERROR(hr)
				hr = pAudioClock->GetFrequency(&audioClockFreq);
			EXIT_ON_ERROR(hr)


				for (unsigned int i = 0; i < _capPorts.size(); i++) {
					//Ignore errors creating the Capture port to allow the playback to work.
					CreateCapturePort(pEnumerator, _capPorts[i]);
				}

			out.flags = 0;
			ResetEvent(hEvent);
			CreateThread(NULL, 0, EventAudioThread, this, 0, &dwThreadId);

			running = true;
			SAFE_RELEASE(pEnumerator)
				return true;
		Exit:
			//For debugging purposes
			if (FAILED(hr)) {
				Error(GetError(hr));
			}
			SAFE_RELEASE(pAudioClock);
			SAFE_RELEASE(out.client)
				SAFE_RELEASE(out.device)
				SAFE_RELEASE(pEnumerator)
				return false;
		}

		bool WasapiDriver::Stop() {
			if (!running) return false;
			out.flags = AUDCLNT_BUFFERFLAGS_SILENT;
			WaitForSingleObject(hEvent, INFINITE);
			for (unsigned int i = 0; i < _capPorts.size(); i++)
			{
				SAFE_RELEASE(_capPorts[i].client)
					SAFE_RELEASE(_capPorts[i].device)
			}
			SAFE_RELEASE(pAudioClock);
			SAFE_RELEASE(out.client)
				SAFE_RELEASE(out.device)
				return true;
		}

		bool WasapiDriver::AddCapturePort(int idx) {
			bool isplaying = running;
			if (idx >= _capEnums.size()) return false;
			if (idx < _portMapping.size() && _portMapping[idx] != -1) return true;

			if (isplaying) Stop();
			PaWasapiSubStream port;
			ZeroMemory(&port, sizeof(PaWasapiSubStream));
			wcscpy_s(port.szDeviceID, MAX_STR_LEN - 1, _capEnums[idx].szDeviceID);
			_capPorts.push_back(port);
			if (_portMapping.size() <= idx) {
				int oldsize = _portMapping.size();
				_portMapping.resize(idx + 1);
				for (int i = oldsize; i < _portMapping.size(); i++) _portMapping[i] = -1;
			}
			_portMapping[idx] = (int)(_capPorts.size() - 1);
			if (isplaying) return Start();
			return true;
		};

		bool WasapiDriver::RemoveCapturePort(int idx) {
			bool isplaying = running;
			int maxSize = 0;
			std::vector<PaWasapiSubStream> newports;
			if (idx >= _capEnums.size() ||
				idx >= _portMapping.size() || _portMapping[idx] == -1) return false;

			if (isplaying) Stop();
			for (unsigned int i = 0; i < _portMapping.size(); ++i)
			{
				if (i != idx && _portMapping[i] != -1) {
					maxSize = i + 1;
					newports.push_back(_capPorts[_portMapping[i]]);
					_portMapping[i] = (int)(newports.size() - 1);
				}
			}
			_portMapping[idx] = -1;
			if (maxSize < _portMapping.size()) _portMapping.resize(maxSize);
			_capPorts = newports;
			if (isplaying) Start();
			return true;
		}

		HRESULT WasapiDriver::CreateCapturePort(IMMDeviceEnumerator* pEnumerator, PaWasapiSubStream& port)
		{
			//avoid opening a port twice
			if (port.client != NULL) return true;
			port._machinepos = 0;
			HRESULT hr;
			if (wcscmp(port.szDeviceID, L"") == 0) {
				hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &port.device);
			} else {
				hr = pEnumerator->GetDevice(port.szDeviceID, &port.device);
			}
			EXIT_ON_ERROR(hr)

				hr = port.device->Activate(IID_IAudioClient, CLSCTX_ALL,
					NULL, (void**)&port.client);
			EXIT_ON_ERROR(hr)

				port.shareMode = out.shareMode;
			hr = GetStreamFormat(port, &port.wavex);
			EXIT_ON_ERROR(hr)
				port.streamFlags = 0;
			port.period = out.period;

			hr = port.client->Initialize(port.shareMode, port.streamFlags, port.period,
				(out.shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? port.period : 0),
				reinterpret_cast<WAVEFORMATEX*>(&port.wavex), NULL);
			port.flags = 0;
			EXIT_ON_ERROR(hr)
				// 2* is a safety measure (Haven't been able to dig out why it crashes if it is exactly the size)
				port.pleft = (float*)dsp.memory_alloc(2 * psy_audiodriversettings_blockbytes(&settings_), 16);
			port.pright = (float*)dsp.memory_alloc(2 * psy_audiodriversettings_blockbytes(&settings_), 16);
			ZeroMemory(port.pleft, 2 * psy_audiodriversettings_blockbytes(&settings_));
			ZeroMemory(port.pright, 2 * psy_audiodriversettings_blockbytes(&settings_));
			return hr;
		Exit:
			Error("Couldn't open the capture device. Possibly the format is not supported");
			SAFE_RELEASE(port.client)
				SAFE_RELEASE(port.device)
				return hr;
		}

		DWORD WasapiDriver::EventAudioThread(void* pWasapi) {
			HRESULT hr;
			HANDLE hEvent = NULL;
			HANDLE hTask = NULL;
			UINT32 bufferFrameCount;
			IAudioRenderClient* pRenderClient = NULL;
			std::vector<IAudioCaptureClient*> capture;

			// universalis::os::thread_name thread_name("wasapi sound output");
			// universalis::cpu::exceptions::install_handler_in_thread();
			WasapiDriver* pThis = reinterpret_cast<WasapiDriver*>(pWasapi);
			// Ask MMCSS to temporarily boost the thread priority
			// to reduce glitches while the low-latency stream plays.
			DWORD taskIndex = 0;
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			hTask = pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
			if (hTask == NULL)
			{
				hr = E_FAIL;
				EXIT_ON_ERROR(hr)
			}
			// Create an event handle and register it for buffer-event notifications.
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if (hEvent == NULL)
			{
				hr = E_FAIL;
				goto Exit;
			}
			hr = pThis->out.client->SetEventHandle(hEvent);
			EXIT_ON_ERROR(hr);

			// Get the actual size of the two allocated buffers.
			hr = pThis->out.client->GetBufferSize(&bufferFrameCount);
			EXIT_ON_ERROR(hr)
				pThis->out.bufferFrameCount = bufferFrameCount;

			// Get the lantency.
			hr = pThis->out.client->GetStreamLatency(&pThis->out.device_latency);
			EXIT_ON_ERROR(hr)

				hr = pThis->out.client->GetService(IID_IAudioRenderClient,
					(void**)&pRenderClient);
			EXIT_ON_ERROR(hr)

				// To reduce latency, load the first buffer with data
				// from the audio source before starting the stream.
				hr = pThis->DoBlock(pRenderClient, bufferFrameCount);
			EXIT_ON_ERROR(hr)

				pThis->writeMark = 0;
			hr = pThis->out.client->Start(); // Start playing.
			EXIT_ON_ERROR(hr)

				for (unsigned int i = 0; i < pThis->_capPorts.size(); i++)
				{
					if (pThis->_capPorts[i].client == NULL)
						continue;
					IAudioCaptureClient* captClient = NULL;
					hr = pThis->_capPorts[i].client->GetBufferSize(&pThis->_capPorts[i].bufferFrameCount);
					EXIT_ON_ERROR(hr)
						hr = pThis->_capPorts[i].client->GetStreamLatency(&pThis->_capPorts[i].device_latency);
					EXIT_ON_ERROR(hr)
						hr = pThis->_capPorts[i].client->GetService(IID_IAudioCaptureClient,
							(void**)&captClient);
					EXIT_ON_ERROR(hr)
						hr = pThis->_capPorts[i].client->Start(); // Start playing.
					EXIT_ON_ERROR(hr)
						capture.push_back(captClient);
				}

			// Each loop fills one of the two buffers.
			while (pThis->out.flags != AUDCLNT_BUFFERFLAGS_SILENT)
			{
				// Wait for next buffer event to be signaled.
				DWORD retval = WaitForSingleObject(hEvent, 2000);
				if (retval != WAIT_OBJECT_0)
				{
					// Event handle timed out after a 2-second wait.
					pThis->out.client->Stop();
					for (unsigned int i = 0; i < pThis->_capPorts.size(); i++)
					{
						if (pThis->_capPorts[i].client == NULL)
							continue;
						hr = pThis->_capPorts[i].client->Stop();
					}
					hr = ERROR_TIMEOUT;
					goto Exit;
				}

				if (pThis->out.shareMode == AUDCLNT_SHAREMODE_SHARED)
				{
					// See how much buffer space is available.
					UINT32 numFramesPadding, numFramesAvailable;
					hr = pThis->out.client->GetCurrentPadding(&numFramesPadding);
					EXIT_ON_ERROR(hr)
						numFramesAvailable = bufferFrameCount - numFramesPadding;
					if (numFramesAvailable > 0) {
						// First, run the capture buffers so that audio is available to wavein machines.
						for (unsigned int i = 0; i < pThis->_capPorts.size(); i++)
						{
							if (pThis->_capPorts[i].client == NULL)
								continue;
							hr = pThis->DoBlockRecording(pThis->_capPorts[i], capture[i], numFramesAvailable);
						}
						hr = pThis->DoBlock(pRenderClient, numFramesAvailable);
						EXIT_ON_ERROR(hr)
					}
				} else {
					// First, run the capture buffers so that audio is available to wavein machines.
					for (unsigned int i = 0; i < pThis->_capPorts.size(); i++)
					{
						if (pThis->_capPorts[i].client == NULL)
							continue;
						hr = pThis->DoBlockRecording(pThis->_capPorts[i], capture[i], bufferFrameCount);
					}
					hr = pThis->DoBlock(pRenderClient, bufferFrameCount);
					EXIT_ON_ERROR(hr)
				}
			}

			for (unsigned int i = 0; i < pThis->_capPorts.size(); i++)
			{
				if (pThis->_capPorts[i].client == NULL)
					continue;
				hr = pThis->_capPorts[i].client->Stop(); // Start playing.
			}
			hr = pThis->out.client->Stop(); // Stop playing.
		Exit:
			//For debugging purposes
			if (FAILED(hr)) {
				Error(GetError(hr));
			}
			if (hEvent != NULL) { CloseHandle(hEvent); }
			if (hTask != NULL) { pAvRevertMmThreadCharacteristics(hTask); }
			for (unsigned int i = 0; i < capture.size(); i++)
			{
				SAFE_RELEASE(capture[i])
			}

			SAFE_RELEASE(pRenderClient)
				pThis->running = false;
			SetEvent(pThis->hEvent);
			return 0;
		}
		HRESULT WasapiDriver::DoBlockRecording(PaWasapiSubStream& port, IAudioCaptureClient* pCaptureClient, int numFramesAvailable)
		{
			BYTE* pData;
			DWORD flags;
			UINT32 packetLength = 0;
			UINT32 numf;

			HRESULT hr = pCaptureClient->GetNextPacketSize(&packetLength);
			EXIT_ON_ERROR(hr)

				while (packetLength != 0 && packetLength <= numFramesAvailable)
				{
					///\todo: finish this implementation.
					HRESULT hr = pCaptureClient->GetBuffer(&pData, &numf, &flags, NULL, NULL);
					EXIT_ON_ERROR(hr)
						if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
						{
							unsigned int _sampleValidBits = psy_audiodriversettings_validbitdepth(&settings_);
							if (_sampleValidBits == 32) {
								DeinterlaceFloat(reinterpret_cast<float*>(pData), port.pleft, port.pright, numf);
							} else if (_sampleValidBits == 24) {
								DeQuantize32AndDeinterlace(reinterpret_cast<int*>(pData), port.pleft, port.pright, numf);
							} else {
								DeQuantize16AndDeinterlace(reinterpret_cast<short int*>(pData), port.pleft, port.pright, numf);
							}
						}
					hr = pCaptureClient->ReleaseBuffer(numf);
					EXIT_ON_ERROR(hr)
						numFramesAvailable -= numf;
					hr = pCaptureClient->GetNextPacketSize(&packetLength);
					EXIT_ON_ERROR(hr)
				}
		Exit:
			port._machinepos = 0;
			return hr;
		}
		HRESULT WasapiDriver::DoBlock(IAudioRenderClient* pRenderClient, int numFramesAvailable) {
			BYTE* pData;
			// Grab the next empty buffer from the audio device.
			HRESULT hr;
			int hostisplaying;
			float* pFloatBlock;

			unsigned int _sampleValidBits = psy_audiodriversettings_validbitdepth(&settings_);
			hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
			EXIT_ON_ERROR(hr)

				pFloatBlock =
				_pCallback(
					_callbackContext, &numFramesAvailable, &hostisplaying);
			if (_sampleValidBits == 32) {
				dsp.movmul(pFloatBlock, reinterpret_cast<float*>(pData), numFramesAvailable * 2, 1.f / 32768.f);
			} else if (_sampleValidBits == 24) {
				Quantize24in32Bit(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
			} else if (_sampleValidBits == 16) {
				if (psy_audiodriversettings_dither(&settings_)) Quantize16WithDither(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
				else Quantize16(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
			}

			hr = pRenderClient->ReleaseBuffer(numFramesAvailable, out.flags);
			writeMark += numFramesAvailable;
		Exit:
			return hr;
		}
		void WasapiDriver::RefreshAvailablePorts()
		{
			IMMDeviceEnumerator* pEnumerator = NULL;
			HRESULT hr = S_OK;

			hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
				IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			EXIT_ON_ERROR(hr)

				RefreshPorts(pEnumerator);
		Exit:
			SAFE_RELEASE(pEnumerator)
		}
		void WasapiDriver::GetPlaybackPorts(std::vector<std::string>& ports) const
		{
			ports.resize(0);
			for (unsigned int i = 0; i < _playEnums.size(); i++) ports.push_back(_playEnums[i].portName);
		}

		void WasapiDriver::GetCapturePorts(std::vector<std::string>& ports) const
		{
			ports.resize(0);
			for (unsigned int i = 0; i < _capEnums.size(); i++) ports.push_back(_capEnums[i].portName);
		}

		void WasapiDriver::GetReadBuffers(int idx, float** pleft, float** pright, int numsamples) {
			if (!running || idx >= _portMapping.size() || _portMapping[idx] == -1
				|| _capPorts[_portMapping[idx]].client == NULL)
			{
				*pleft = 0;
				*pright = 0;
				return;
			}
			int mpos = _capPorts[_portMapping[idx]]._machinepos;
			*pleft = _capPorts[_portMapping[idx]].pleft + mpos;
			*pright = _capPorts[_portMapping[idx]].pright + mpos;
			_capPorts[_portMapping[idx]]._machinepos += numsamples;
		}

		void WasapiDriver::Configure(void)
		{
			/*WasapiConfig dlg;
			dlg.m_driverIndex = GetIdxFromDevice(out.szDeviceID);
			dlg.m_sampleRate = settings_->samplesPerSec();
			dlg.m_bitDepth = settings_->validBitDepth();
			dlg.m_dither = settings_->dither();
			dlg.m_shareMode = settings_->shared;
			dlg.m_bufferSize = settings_->blockFrames();
			dlg.wasapi = this;

			if (dlg.DoModal() != IDOK) return;
			Enable(false);
			AUDCLNT_SHAREMODE sharemode = (dlg.m_shareMode == 0) ? AUDCLNT_SHAREMODE_EXCLUSIVE : AUDCLNT_SHAREMODE_SHARED;
			if (dlg.m_shareMode == 0) {
				WAVEFORMATPCMEX wf;
				PrepareWaveFormat(wf, 2, dlg.m_sampleRate, (dlg.m_bitDepth == 24) ? 32 : dlg.m_bitDepth, dlg.m_bitDepth);
				bool supported = _playEnums[dlg.m_driverIndex].IsFormatSupported(wf, sharemode);
				if (!supported) {
					Error("The Format selected is not supported. Keeping the previous configuration");
					return;
				}
			}

			wcscpy_s(settings_->szDeviceID, MAX_STR_LEN - 1, _playEnums[dlg.m_driverIndex].szDeviceID);
			settings_->setSamplesPerSec(dlg.m_sampleRate);
			settings_->setValidBitDepth(dlg.m_bitDepth);
			settings_->setDither(dlg.m_dither);
			settings_->setBlockFrames(dlg.m_bufferSize);
			settings_->shared = dlg.m_shareMode;
			Enable(true);*/
		}

		psy_List* WasapiDriver::BufferList(int* prefindex)
		{
			psy_List* rv;
			char buf[8];			
			int g = 64;

			rv = NULL;
			*prefindex = 0;
			if (_playEnums.size() <= driverindex) {
				return NULL;
			}
			if (this->shared)
			{
				int samples = portaudio::MakeFramesFromHns(_playEnums[driverindex].DefaultDevicePeriod, settings_.samplespersec_);
				sprintf(buf, "%d", samples);
				psy_list_append(&rv, psy_strdup(buf));
			} else {
				int samples = portaudio::MakeFramesFromHns(_playEnums[driverindex].MinimumDevicePeriod, settings_.samplespersec_);
				samples = portaudio::AlignFramesPerBuffer(samples, settings_.samplespersec_, settings_.bitdepth_ == 24 ? 32 : settings_.bitdepth_);
				int samples2 = portaudio::MakeFramesFromHns(_playEnums[driverindex].DefaultDevicePeriod, settings_.samplespersec_);
				samples2 = portaudio::AlignFramesPerBuffer(samples2, settings_.samplespersec_, settings_.bitdepth_ == 24 ? 32 : settings_.bitdepth_);
				samples2 += g;
				if (samples2 < 2048) samples2 = 2048;

				sprintf(buf, "%d", samples);
				psy_list_append(&rv, psy_strdup(buf));

				for (int i = ((samples / g) + 1) * g; i <= samples2; i += g)
				{
					if (i <= settings_.blockframes_)
					{
						*prefindex++;
					}
					sprintf(buf, "%d", i);
					psy_list_append(&rv, psy_strdup(buf));
				}
				if (*prefindex >= psy_list_size(rv))
				{
					*prefindex = psy_list_size(rv) - 1;
				}
			}
			return rv;
		}


		void WasapiDriver::RefreshPorts(IMMDeviceEnumerator* pEnumerator) {
			HRESULT hr = S_OK;
			IMMDeviceCollection* pCollection = NULL;
			IMMDevice* pEndpoint = NULL;
			LPWSTR defaultID = NULL;
			_playEnums.resize(0);
			_capEnums.resize(0);

			hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
			EXIT_ON_ERROR(hr)
				hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pEndpoint);
			EXIT_ON_ERROR(hr)
				hr = pEndpoint->GetId(&defaultID);
			EXIT_ON_ERROR(hr)

				FillPortList(_playEnums, pCollection, defaultID);
			::CoTaskMemFree(defaultID);
			defaultID = NULL;
			SAFE_RELEASE(pCollection)

				hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE,
					&pCollection);
			EXIT_ON_ERROR(hr)
				hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pEndpoint);
			EXIT_ON_ERROR(hr)
				hr = pEndpoint->GetId(&defaultID);
			EXIT_ON_ERROR(hr)

				FillPortList(_capEnums, pCollection, defaultID);

		Exit:
			::CoTaskMemFree(defaultID);
			SAFE_RELEASE(pEndpoint)
				SAFE_RELEASE(pCollection)
		}

		void WasapiDriver::FillPortList(std::vector<PortEnum>& portList, IMMDeviceCollection* pCollection, LPWSTR defaultID)
		{
			IMMDevice* pEndpoint = NULL;
			IPropertyStore* pProps = NULL;
			IAudioClient* client = NULL;
			WCHAR* pszDeviceId = NULL;
			HRESULT hr = S_OK;
			UINT count;

			hr = pCollection->GetCount(&count);
			EXIT_ON_ERROR(hr)

				// Each loop prints the name of an endpoint device.
				for (ULONG i = 0; i < count; i++)
				{
					// Get pointer to endpoint number i.
					hr = pCollection->Item(i, &pEndpoint);
					EXIT_ON_ERROR(hr)

						// Get the endpoint ID string.
						hr = pEndpoint->GetId(&pszDeviceId);
					EXIT_ON_ERROR(hr)

						hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
					EXIT_ON_ERROR(hr)

						PortEnum pEnum;
					// "Friendly" Name
					{
						PROPVARIANT vars;
						PropVariantInit(&vars);

						hr = pProps->GetValue(PKEY_Device_FriendlyName, &vars);
						EXIT_ON_ERROR(hr)

							wcscpy_s(pEnum.szDeviceID, MAX_STR_LEN - 1, pszDeviceId);
						wcstombs(pEnum.portName, vars.pwszVal, MAX_STR_LEN - 1);

						PropVariantClear(&vars);
						if (wcscmp(pEnum.szDeviceID, defaultID) == 0) {
							pEnum.isDefault = true;
						}
					}

					// Default format
					{
						PROPVARIANT vars;
						PropVariantInit(&vars);

						hr = pProps->GetValue(PKEY_AudioEngine_DeviceFormat, &vars);
						EXIT_ON_ERROR(hr);

						ZeroMemory(&pEnum.MixFormat, sizeof(WAVEFORMATEXTENSIBLE));
						memcpy(&pEnum.MixFormat, vars.blob.pBlobData, std::min<ULONG>(sizeof(WAVEFORMATEXTENSIBLE), vars.blob.cbSize));
						PropVariantClear(&vars);
					}
					//lantency
					{
						hr = pEndpoint->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client);
						EXIT_ON_ERROR(hr)

							hr = client->GetDevicePeriod(&pEnum.DefaultDevicePeriod, &pEnum.MinimumDevicePeriod);
						EXIT_ON_ERROR(hr)
					}
					{
						WAVEFORMATEX* pwft;
						hr = client->GetMixFormat(&pwft);
						EXIT_ON_ERROR(hr);
						memcpy(&pEnum.MixFormat, pwft, std::min(sizeof(WAVEFORMATEX) + pwft->cbSize, sizeof(WAVEFORMATEXTENSIBLE)));
						if (pwft->cbSize == 0) pEnum.MixFormat.Samples.wValidBitsPerSample = pwft->wBitsPerSample;
						::CoTaskMemFree(pwft);
					}
					// Align frames to HD Audio packet size of 128 bytes 
					int samples = portaudio::MakeFramesFromHns(pEnum.DefaultDevicePeriod, pEnum.MixFormat.Format.nSamplesPerSec);
					samples = portaudio::AlignFramesPerBuffer(samples,
						pEnum.MixFormat.Format.nSamplesPerSec, pEnum.MixFormat.Format.nBlockAlign);
					pEnum.DefaultDevicePeriod = portaudio::MakeHnsPeriod(samples, pEnum.MixFormat.Format.nSamplesPerSec);

					portList.push_back(pEnum);

					CoTaskMemFree(pszDeviceId);
					pszDeviceId = NULL;
					SAFE_RELEASE(client)
						SAFE_RELEASE(pProps)
						SAFE_RELEASE(pEndpoint)
				}
		Exit:
			CoTaskMemFree(pszDeviceId);
			SAFE_RELEASE(client)
				SAFE_RELEASE(pProps)
				SAFE_RELEASE(pEndpoint)
		}
		HRESULT WasapiDriver::GetStreamFormat(PaWasapiSubStream& stream, WAVEFORMATEXTENSIBLE* wfOut)
		{
			HRESULT hr;
			WAVEFORMATEX* bla = NULL;

			if (stream.shareMode == AUDCLNT_SHAREMODE_SHARED)
			{
				WAVEFORMATEX* pwft;
				hr = stream.client->GetMixFormat(&pwft);
				
				EXIT_ON_ERROR(hr)

				psy_audiodriversettings_setsamplespersec(&settings_, pwft->nSamplesPerSec);					
				psy_audiodriversettings_setvalidbitdepth(&settings_, pwft->wBitsPerSample);
				if (pwft->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
					WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)pwft;
					psy_audiodriversettings_setvalidbitdepth(&settings_, wfex->Samples.wValidBitsPerSample);					
				}
				memcpy(wfOut, pwft, std::min(sizeof(WAVEFORMATEX) + pwft->cbSize, sizeof(WAVEFORMATEXTENSIBLE)));
				::CoTaskMemFree(pwft);
			} 			else
			{
				WAVEFORMATPCMEX format;

				PrepareWaveFormat((WAVEFORMATEXTENSIBLE*)&format,
					psy_audiodriversettings_numchannels(&settings_),
					psy_audiodriversettings_samplespersec(&settings_),
					psy_audiodriversettings_bitdepth(&settings_),
					psy_audiodriversettings_validbitdepth(&settings_));
				memcpy(wfOut, &format, sizeof(WAVEFORMATPCMEX));
			}			
			hr = stream.client->IsFormatSupported(stream.shareMode, reinterpret_cast<WAVEFORMATEX*>(wfOut),
				(stream.shareMode == AUDCLNT_SHAREMODE_SHARED) ? &bla : NULL);
			if (bla != NULL) { ::CoTaskMemFree(bla); }
			return hr;
		Exit:
			return hr;
		}

		uint32_t WasapiDriver::GetWritePosInSamples() const { return writeMark; }

		uint32_t WasapiDriver::GetPlayPosInSamples() {
			UINT64 pos;
			uint32_t retVal = 0;
			if (running) {
				HRESULT hr = pAudioClock->GetPosition(&pos, NULL);
				EXIT_ON_ERROR(hr)
					if (audioClockFreq == psy_audiodriversettings_samplespersec(&settings_)) {
						retVal = pos;
					} 				else {
						//Thus, the stream-relative offset in seconds can always be calculated as p/f.
						retVal = (pos * psy_audiodriversettings_samplespersec(&settings_) / audioClockFreq);
					}
			}
		Exit:
			return retVal;
		}

		uint32_t WasapiDriver::GetInputLatencyMs() const {
			if (_capPorts.size() > 0) {
				///\todo: The documentation suggests that the period has to be added to the latency. verify it.
				return portaudio::nano100ToMillis(_capPorts[0].device_latency + _capPorts[0].period);
			} 			else return 0;
		}

		uint32_t WasapiDriver::GetOutputLatencyMs() const {
			///\todo: The documentation suggests that the period has to be added to the latency. verify it.
			return portaudio::nano100ToMillis(out.device_latency + out.period);
		}
		uint32_t WasapiDriver::GetInputLatencySamples() const { return GetInputLatencyMs() * psy_audiodriversettings_samplespersec(&settings_) * 0.001f; }
		uint32_t WasapiDriver::GetOutputLatencySamples() const { return GetOutputLatencyMs() * psy_audiodriversettings_samplespersec(&settings_) * 0.001f; }

		uint32_t WasapiDriver::GetIdxFromDevice(WCHAR* szDeviceID) const {
			for (int i = 0; i < _playEnums.size(); ++i)
			{
				if (wcscmp(_playEnums[i].szDeviceID, szDeviceID) == 0)
				{
					return i;
				}
			}
			return 0;
		}

		const char* WasapiDriver::GetError(HRESULT hr) {
			const char* text = NULL;
			switch (hr)
			{
			case S_OK:									return "";
			case E_POINTER:text = "E_POINTER"; break;
			case E_INVALIDARG:text = "E_INVALIDARG"; break;

			case AUDCLNT_E_NOT_INITIALIZED:text = "AUDCLNT_E_NOT_INITIALIZED"; break;
			case AUDCLNT_E_ALREADY_INITIALIZED:text = "AUDCLNT_E_ALREADY_INITIALIZED"; break;
			case AUDCLNT_E_WRONG_ENDPOINT_TYPE:text = "AUDCLNT_E_WRONG_ENDPOINT_TYPE"; break;
			case AUDCLNT_E_DEVICE_INVALIDATED:text = "AUDCLNT_E_DEVICE_INVALIDATED"; break;
			case AUDCLNT_E_NOT_STOPPED:text = "AUDCLNT_E_NOT_STOPPED"; break;
			case AUDCLNT_E_BUFFER_TOO_LARGE:text = "AUDCLNT_E_BUFFER_TOO_LARGE"; break;
			case AUDCLNT_E_OUT_OF_ORDER:text = "AUDCLNT_E_OUT_OF_ORDER"; break;
			case AUDCLNT_E_UNSUPPORTED_FORMAT:text = "AUDCLNT_E_UNSUPPORTED_FORMAT"; break;
			case AUDCLNT_E_INVALID_SIZE:text = "AUDCLNT_E_INVALID_SIZE"; break;
			case AUDCLNT_E_DEVICE_IN_USE:text = "AUDCLNT_E_DEVICE_IN_USE"; break;
			case AUDCLNT_E_BUFFER_OPERATION_PENDING:text = "AUDCLNT_E_BUFFER_OPERATION_PENDING"; break;
			case AUDCLNT_E_THREAD_NOT_REGISTERED:text = "AUDCLNT_E_THREAD_NOT_REGISTERED"; break;
			case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:text = "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"; break;
			case AUDCLNT_E_ENDPOINT_CREATE_FAILED:text = "AUDCLNT_E_ENDPOINT_CREATE_FAILED"; break;
			case AUDCLNT_E_SERVICE_NOT_RUNNING:text = "AUDCLNT_E_SERVICE_NOT_RUNNING"; break;
			case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED:text = "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"; break;
			case AUDCLNT_E_EXCLUSIVE_MODE_ONLY:text = "AUDCLNT_E_EXCLUSIVE_MODE_ONLY"; break;
			case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:text = "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"; break;
			case AUDCLNT_E_EVENTHANDLE_NOT_SET:text = "AUDCLNT_E_EVENTHANDLE_NOT_SET"; break;
			case AUDCLNT_E_INCORRECT_BUFFER_SIZE:text = "AUDCLNT_E_INCORRECT_BUFFER_SIZE"; break;
			case AUDCLNT_E_BUFFER_SIZE_ERROR:text = "AUDCLNT_E_BUFFER_SIZE_ERROR"; break;
			case AUDCLNT_E_CPUUSAGE_EXCEEDED:text = "AUDCLNT_E_CPUUSAGE_EXCEEDED"; break;

#ifdef AUDCLNT_E_BUFFER_ERROR
			case AUDCLNT_E_BUFFER_ERROR:text = "AUDCLNT_E_BUFFER_ERROR"; break;
#endif

#ifdef AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED
			case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:text = "AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED"; break;
#endif

#ifdef AUDCLNT_E_INVALID_DEVICE_PERIOD
			case AUDCLNT_E_INVALID_DEVICE_PERIOD:text = "AUDCLNT_E_INVALID_DEVICE_PERIOD"; break;
#endif

			case AUDCLNT_S_BUFFER_EMPTY:text = "AUDCLNT_S_BUFFER_EMPTY"; break;
			case AUDCLNT_S_THREAD_ALREADY_REGISTERED:text = "AUDCLNT_S_THREAD_ALREADY_REGISTERED"; break;
			case AUDCLNT_S_POSITION_STALLED:text = "AUDCLNT_S_POSITION_STALLED"; break;

				// other windows common errors:
			case CO_E_NOTINITIALIZED:text = "CO_E_NOTINITIALIZED"; break;

			default:
				text = "UNKNOWN ERROR";
			}
			return text;
		}

		void PrepareWaveFormat(WAVEFORMATEXTENSIBLE* wf, int channels, int sampleRate, int bits, int validBits)
		{
			// Set up wave format structure. 
			ZeroMemory(wf, sizeof(WAVEFORMATEX));
			wf->Format.nChannels = channels;
			wf->Format.wBitsPerSample = bits;
			wf->Format.nSamplesPerSec = sampleRate;
			wf->Format.nBlockAlign = wf->Format.nChannels * wf->Format.wBitsPerSample / 8;
			wf->Format.nAvgBytesPerSec = wf->Format.nSamplesPerSec * wf->Format.nBlockAlign;
			if (bits <= 16) {
				wf->Format.wFormatTag = WAVE_FORMAT_PCM;
				wf->Format.cbSize = 0;
			} 	else {
				wf->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
				wf->Format.cbSize = 0x16;
				wf->Samples.wValidBitsPerSample = validBits;
				if (channels == 2) {
					wf->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
				}
				if (validBits == 32) {
					wf->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
				} 		else {
					wf->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
				}
			}
		}

		// returns random value between 0 and 1
		// i got the magic numbers from csound so they should be ok but 
		// I haven't checked them myself
		inline double frand()
		{
			static long stat = 0x16BA2118;
			stat = (stat * 1103515245 + 12345) & 0x7fffffff;
			return (double)stat * (1.0 / 0x7fffffff);
		}
		//In -> -32768.0..32768.0 , out -32768..32767
		void Quantize16WithDither(float* pin, int* piout, int c)
		{
			double const d2i = (1.5 * (1 << 26) * (1 << 26));
			do
			{
				double res = ((double)pin[1] + frand()) + d2i;
				int r = *(int*)&res;

				if (r < SHORT_MIN)
				{
					r = SHORT_MIN;
				} 				else if (r > SHORT_MAX)
				{
					r = SHORT_MAX;
				}
				res = ((double)pin[0] + frand()) + d2i;
				int l = *(int*)&res;

				if (l < SHORT_MIN)
				{
					l = SHORT_MIN;
				} 				else if (l > SHORT_MAX)
				{
					l = SHORT_MAX;
				}
				*piout++ = (r << 16) | static_cast<uint16_t>(l);
				pin += 2;
			} 			while (--c);
		}
		//In -> -32768.0..32768.0 , out -32768..32767
		void Quantize16(float* pin, int* piout, int c)
		{
			do
			{
				int r = helpers::math::round<int, float>(pin[1]);
				if (r < SHORT_MIN)
				{
					r = SHORT_MIN;
				} 				else if (r > SHORT_MAX)
				{
					r = SHORT_MAX;
				}

				int l = helpers::math::round<int, float>(pin[0]);
				if (l < SHORT_MIN)
				{
					l = SHORT_MIN;
				} 				else if (l > SHORT_MAX)
				{
					l = SHORT_MAX;
				}
				*piout++ = (r << 16) | static_cast<uint16_t>(l);
				pin += 2;
			} 			while (--c);
		}
		//In -> -8388608.0..8388608.0, out  -2147483648.0 to 2147483648.0
		void Quantize24in32Bit(float* pin, int* piout, int c)
		{
			// TODO Don't really know why, but the -100 is what made the clipping work correctly.
			int const max((1u << ((sizeof(int32_t) << 3) - 1)) - 100);
			int const min(-max - 1);
			for (int i = 0; i < c; ++i) {
				*piout++ = psycle::helpers::math::rint<int32_t>(psycle::helpers::math::clip(float(min), (*pin++) * 65536.0f, float(max)));
				*piout++ = psycle::helpers::math::rint<int32_t>(psycle::helpers::math::clip(float(min), (*pin++) * 65536.0f, float(max)));
			}
		}
		//In -> -8388608.0..8388608.0 in 4 bytes, out -8388608..8388608, aligned to 3 bytes, Big endian
		void Quantize24BE(float* pin, int* piout, int c)
		{
			unsigned char* cptr = (unsigned char*)piout;
			for (int i = 0; i < c; ++i) {
				int outval = psycle::helpers::math::rint_clip<int32_t, 24>((*pin++) * 256.0f);
				*cptr++ = (unsigned char)((outval >> 16) & 0xFF);
				*cptr++ = (unsigned char)((outval >> 8) & 0xFF);
				*cptr++ = (unsigned char)(outval & 0xFF);

				outval = psycle::helpers::math::rint_clip<int32_t, 24>((*pin++) * 256.0f);
				*cptr++ = (unsigned char)((outval >> 16) & 0xFF);
				*cptr++ = (unsigned char)((outval >> 8) & 0xFF);
				*cptr++ = (unsigned char)(outval & 0xFF);
			}
		}
		//In -> -8388608.0..8388608.0 in 4 bytes, out -8388608..8388608, aligned to 3 bytes, little endian
		void Quantize24LE(float* pin, int* piout, int c)
		{
			unsigned char* cptr = (unsigned char*)piout;
			for (int i = 0; i < c; ++i) {
				int outval = psycle::helpers::math::rint_clip<int32_t, 24>((*pin++) * 256.0f);
				*cptr++ = (unsigned char)(outval & 0xFF);
				*cptr++ = (unsigned char)((outval >> 8) & 0xFF);
				*cptr++ = (unsigned char)((outval >> 16) & 0xFF);

				outval = psycle::helpers::math::rint_clip<int32_t, 24>((*pin++) * 256.0f);
				*cptr++ = (unsigned char)(outval & 0xFF);
				*cptr++ = (unsigned char)((outval >> 8) & 0xFF);
				*cptr++ = (unsigned char)((outval >> 16) & 0xFF);
			}
		}
		//In -> -32768..32767 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
		void DeQuantize16AndDeinterlace(short int* pin, float* poutleft, float* poutright, int c)
		{
			do
			{
				*poutleft++ = *pin++;
				*poutright++ = *pin++;
			} 			while (--c);
		}
		//In -> -2147483648..2147483647 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
		void DeQuantize32AndDeinterlace(int* pin, float* poutleft, float* poutright, int c)
		{
			do
			{
				*poutleft++ = (*pin++) * 0.0000152587890625;
				*poutright++ = (*pin++) * 0.0000152587890625;
			} 			while (--c);
		}
		//In -> -1.0..1.0 stereo interlaced, out -32768.0..32767.0 stereo deinterlaced
		void DeinterlaceFloat(float* pin, float* poutleft, float* poutright, int c)
		{
			do
			{
				*poutleft++ = (*pin++) * 32768.f;
				*poutright++ = (*pin++) * 32768.f;
			} 			while (--c);
		}
	}
}

using namespace psycle::host;

/* DLL Wrapper for Psycle Wasapi Driver */
typedef struct {
	psy_AudioDriver driver;
	psy_Property* configuration;
	HWND m_hWnd;
	int _dither;
	unsigned int pollSleep_;

	int _initialized;
	int _configured;
	// controls if the driver is supposed to be running or not
	int _running;
	// informs the real state of the DSound buffer (see the control of buffer play in DoBlocks())
	int _playing;
	// Controls if we want the thread to be running or not
	int _threadRun;

	psy_List* _playEnums;
	psy_List* _capEnums;
	psy_List* _capPorts;
	psy_Table _portMapping; // <int, int>

	HANDLE hEvent;
	int (*error)(int, const char*);
	psycle::host::WasapiDriver* wasapiif;
} CWasapiDriver;

static int driver_init(psy_AudioDriver*);
static int driver_open(psy_AudioDriver*);
static bool driver_opened(const psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, const psy_Property*);
static const psy_Property* driver_configuration(const psy_AudioDriver*);
static psy_dsp_big_hz_t driver_samplerate(psy_AudioDriver*);
static const char* capturename(psy_AudioDriver*, int index);
static int numcaptures(psy_AudioDriver*);
static const char* playbackname(psy_AudioDriver*, int index);
static int numplaybacks(psy_AudioDriver*);
static int addcaptureport(CWasapiDriver* self, int idx);
static int removecaptureport(CWasapiDriver* self, int idx);
static bool start(CWasapiDriver*);
static bool stop(CWasapiDriver*);
static void driver_deallocate(psy_AudioDriver*);
static void driver_makeconfig(psy_AudioDriver*);
static void readbuffers(CWasapiDriver* self, int idx,
	float** left, float** right, uintptr_t numsamples);
static const psy_AudioDriverInfo* driver_info(psy_AudioDriver*);
static uintptr_t playposinsamples(psy_AudioDriver*);

static psy_AudioDriverVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.opened = driver_opened;
		vtable.deallocate = driver_deallocate;		
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.samplerate = driver_samplerate;
		vtable.addcapture = (psy_audiodriver_fp_addcapture)addcaptureport;
		vtable.removecapture = (psy_audiodriver_fp_removecapture)removecaptureport;
		vtable.readbuffers = (psy_audiodriver_fp_readbuffers)readbuffers;
		vtable.capturename = (psy_audiodriver_fp_capturename)capturename;
		vtable.numcaptures = (psy_audiodriver_fp_numcaptures)numcaptures;
		vtable.playbackname = (psy_audiodriver_fp_playbackname)playbackname;
		vtable.numplaybacks = (psy_audiodriver_fp_numplaybacks)numplaybacks;
		vtable.playposinsamples = playposinsamples;
		vtable.info = (psy_audiodriver_fp_info)driver_info;
		vtable_initialized = 1;
	}
}

int on_error(int err, const char* msg)
{	
//	MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT psy_AudioDriverInfo const* __cdecl GetPsycleDriverInfo(void)
{
	static psy_AudioDriverInfo info;

	info.guid = PSY_AUDIODRIVER_WASAPI_GUID;
	info.Flags = 0;
	info.Name = "Asio Audio Driver";
	info.ShortName = "Asio";
	info.Version = 0;
	return &info;
}

EXPORT psy_AudioDriver* __cdecl driver_create(void)
{
	CWasapiDriver* asio;

	asio = (CWasapiDriver*)malloc(sizeof(CWasapiDriver));
	if (asio != NULL) {
		driver_init(&asio->driver);
		return &asio->driver;
	}
	return NULL;
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver_dispose(driver);
	free(driver);
}

int driver_init(psy_AudioDriver* driver)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;

	memset(self, 0, sizeof(CWasapiDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->error = on_error;
#ifdef PSYCLE_USE_SSE
	psy_dsp_sse2_init(&dsp);
#else
	psy_dsp_noopt_init(&dsp);
#endif
	SetupAVRT();	
	
	self->wasapiif = new WasapiDriver();
	driver_makeconfig(&self->driver);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;
	self->wasapiif->Enable(false);
	delete self->wasapiif;
	self->wasapiif = 0;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	CloseAVRT();
	return 0;
}

void driver_makeconfig(psy_AudioDriver* driver)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;
	char key[256];
	psy_Property* property;
	psy_Property* access;
	psy_Property* devices;
	psy_Property* indevices;
	psy_Property* bitdepth;
	psy_Property* buffers;
	
	std::vector<std::string> playbackports;
	std::vector<std::string> captureports;

	psy_snprintf(key, 256, "wasapi-guid-%d", PSY_AUDIODRIVER_WASAPI_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_set_text(
		psy_property_allocinit_key(key), "Wasapi"));
	psy_property_hide(psy_property_append_int(self->configuration,
		"guid", PSY_AUDIODRIVER_WASAPI_GUID, 0, 0));
	psy_property_set_text(
		psy_property_setreadonly(
			psy_property_append_str(self->configuration, "name", "Wasapi Driver"),
			TRUE),
		"Name");
	psy_property_setreadonly(
		psy_property_append_str(self->configuration, "vendor", "Psycledelics"),
		TRUE);
	psy_property_setreadonly(
		psy_property_append_str(self->configuration, "version", "1.0"),
		TRUE);
	access = psy_property_set_text(
		psy_property_append_choice(self->configuration, "access", self->wasapiif->shared),
		"Device access");
	psy_property_set_text(
		psy_property_append_int(access, "exclusive",
			!self->wasapiif->shared, 0, 1),
		"Exclusive");
	psy_property_set_text(
		psy_property_append_int(access, "shared",
			self->wasapiif->shared, 0, 1),
		"Shared");	
	psy_property_set_text(
		psy_property_append_int(self->configuration, "samplerate",
		(intptr_t)psy_audiodriversettings_samplespersec(
			&self->wasapiif->settings_), 0, 0),
		"Samplerate");	
	bitdepth = psy_property_set_text(
		psy_property_append_choice(self->configuration, "bitdepth", 2),
		"BitDepth");
	psy_property_set_hint(bitdepth, PSY_PROPERTY_HINT_COMBO);
	psy_property_set_text(
		psy_property_append_int(bitdepth, "16",
			0, 0, 0),
		"16bits");
	psy_property_set_text(
		psy_property_append_int(bitdepth, "16dither",
			0, 0, 0),
		"16bits dither");
	psy_property_set_text(
		psy_property_append_int(bitdepth, "24bits",
			0, 0, 0),
		"24bits");
	psy_property_set_text(
		psy_property_append_int(bitdepth, "32bits",
			0, 0, 0),
		"32bits float");

	//psy_property_set_text(
	//	psy_property_append_int(self->configuration, "numbuf",
	//		psy_audiodriversettings_blockcount(&ASIOInterface::settings_), 1, 8),
	//	"Buffer Number");
	//psy_property_set_text(
	//	psy_property_append_int(self->configuration, "numsamples",
	//		psy_audiodriversettings_blockframes(&ASIOInterface::settings_),
	//		64, 8193),
	//	"Buffer Samples");
	self->wasapiif->RefreshAvailablePorts();
	self->wasapiif->GetPlaybackPorts(playbackports);
	buffers = psy_property_set_text(
		psy_property_append_choice(self->configuration, "buffers", 0),
		"Buffer Samples");
	devices = psy_property_set_text(
		psy_property_append_choice(self->configuration, "device", 0),
		"Output Device");
	psy_property_set_hint(devices, PSY_PROPERTY_HINT_COMBO);	
	intptr_t i = 0;
	for (std::vector<std::string>::iterator it = playbackports.begin();
		it != playbackports.end(); ++it, ++i) {
		psy_property_append_int(devices, (*it).c_str(), i, 0, 0);
	}
	indevices = psy_property_set_text(
		psy_property_append_choice(self->configuration, "indevice", 0),
		"Standard Input Device(Select different in Recorder)");
	psy_property_set_hint(indevices, PSY_PROPERTY_HINT_COMBO);
	i = 0;
	self->wasapiif->GetCapturePorts(captureports);
	for (std::vector<std::string>::iterator it = captureports.begin();
		it != captureports.end(); ++it, ++i) {
		psy_property_append_int(indevices, (*it).c_str(), i, 0, 0);
	}
}

void driver_configure(psy_AudioDriver* driver, const psy_Property* config)
{
	CWasapiDriver* self;
	psy_Property* property;

	self = (CWasapiDriver*)driver;
	if (config) {
		psy_property_sync(self->configuration, config);
	}
	property = psy_property_at(self->configuration, "device",
		PSY_PROPERTY_TYPE_CHOICE);
	if (property) {
		intptr_t playenumindex;
		psy_List* portnode;

		portnode = NULL;
		playenumindex = psy_property_item_int(property);
		if (playenumindex >= 0) {
			self->wasapiif->driverindex = playenumindex;
			wcscpy_s(self->wasapiif->szDeviceID, MAX_STR_LEN - 1,
				self->wasapiif->_playEnums[playenumindex].szDeviceID);			
		}
	}
	property = psy_property_at(self->configuration, "access",
		PSY_PROPERTY_TYPE_CHOICE);
	if (property) {		
		self->wasapiif->shared = psy_property_item_int(property) != 0;		
	}	
	property = psy_property_at(self->configuration, "bitdepth",
		PSY_PROPERTY_TYPE_CHOICE);
	if (self->wasapiif->shared) {
		psy_property_hide(property);
	} else {
		psy_property_set_hint(property, PSY_PROPERTY_HINT_COMBO);
	}
	if (property) {
		intptr_t choice;

		choice = psy_property_item_int(property);
		switch (choice) {
		case 0:			
			psy_audiodriversettings_setvalidbitdepth(&self->wasapiif->settings_, 16);
			self->wasapiif->settings_.dither_ = FALSE;
			break;
		case 1:
			psy_audiodriversettings_setvalidbitdepth(&self->wasapiif->settings_, 16);
			self->wasapiif->settings_.dither_ = TRUE;
			break;
		case 2:
			psy_audiodriversettings_setvalidbitdepth(&self->wasapiif->settings_, 24);
			self->wasapiif->settings_.dither_ = FALSE;
			break;
		case 3:
			psy_audiodriversettings_setvalidbitdepth(&self->wasapiif->settings_, 32);
			self->wasapiif->settings_.dither_ = FALSE;
			break;
		default:
			psy_audiodriversettings_setvalidbitdepth(&self->wasapiif->settings_, 24);
			self->wasapiif->settings_.dither_ = FALSE;
			break;
		}		
	}
	property = psy_property_at(self->configuration, "samplerate",
		PSY_PROPERTY_TYPE_INTEGER);
	if (property) {
		psy_audiodriversettings_setsamplespersec(&self->wasapiif->settings_,
			(psy_dsp_big_hz_t)psy_property_item_int(property));
		if (self->wasapiif->shared) {
			psy_property_hide(property);
		} else {
			psy_property_set_hint(property, PSY_PROPERTY_HINT_EDIT);
		}
	}	
	property = psy_property_at(self->configuration, "buffers",
		PSY_PROPERTY_TYPE_CHOICE);
	if (self->wasapiif->shared) {
		psy_property_hide(property);
	} else {
		psy_property_set_hint(property, PSY_PROPERTY_HINT_COMBO);
	}
	if (property) {
		psy_List* p;
		psy_List* bufferlist;
		int bufferindex;
		int framesize;
		int i;
		bufferlist = self->wasapiif->BufferList(&bufferindex);
		bufferindex = psy_property_item_int(property);
		framesize = 512;		
		psy_property_clear(property);
		for (i = 0,  p = bufferlist; p != NULL; p = p->next, ++i) {
			const char* str;

			str = (const char*)p->entry;
			psy_property_append_int(property, str, atoi(str), 0, 0);
			if (bufferindex == i) {
				framesize = atoi(str);
			}
		}
		psy_list_deallocate(&bufferlist, NULL);
		psy_audiodriversettings_setblockframes(&self->wasapiif->settings_, framesize);
	}	
}

psy_dsp_big_hz_t driver_samplerate(psy_AudioDriver* driver)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;

	return psy_audiodriversettings_samplespersec(&self->wasapiif->settings_);
}

int driver_open(psy_AudioDriver* driver)
{
	int status;

	CWasapiDriver* self = (CWasapiDriver*)driver;
	if (!self->wasapiif->Initialized()) {		
		self->wasapiif->Initialize(driver->callback, driver->callbackcontext);
	}	
	status = self->wasapiif->Enable(true);
	return status;
}

bool driver_opened(const psy_AudioDriver* driver)
{
	CWasapiDriver* self;

	self = (CWasapiDriver*)driver;
	return (self->_running != FALSE);
}

int driver_close(psy_AudioDriver* driver)
{
	int status;
	CWasapiDriver* self = (CWasapiDriver*)driver;
	status = self->wasapiif->Enable(false);	
	return status;
}

void readbuffers(CWasapiDriver* self, int idx, float** left, float** right,
	uintptr_t numsamples)
{
	self->wasapiif->GetReadBuffers(idx, left, right, (int)numsamples);
}

const char* capturename(psy_AudioDriver* driver, int index)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;

	std::vector<std::string> ports;
	self->wasapiif->GetCapturePorts(ports);
	return ports[index].c_str();
}

int numcaptures(psy_AudioDriver* driver)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;
	std::vector<std::string> ports;
	self->wasapiif->GetCapturePorts(ports);
	return ports.size();
}

const char* playbackname(psy_AudioDriver* driver, int index)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;

	std::vector<std::string> ports;
	self->wasapiif->GetPlaybackPorts(ports);
	return ports[index].c_str();
}

int numplaybacks(psy_AudioDriver* driver)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;
	std::vector<std::string> ports;
	self->wasapiif->GetPlaybackPorts(ports);
	return ports.size();
}

int removecaptureport(CWasapiDriver* self, int idx)
{
	return self->wasapiif->RemoveCapturePort(idx);
}

int addcaptureport(CWasapiDriver* self, int idx)
{
	return self->wasapiif->AddCapturePort(idx);
}

const psy_AudioDriverInfo* driver_info(psy_AudioDriver* self)
{
	return GetPsycleDriverInfo();
}

const psy_Property* driver_configuration(const psy_AudioDriver* driver)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;

	return self->configuration;
}

uintptr_t playposinsamples(psy_AudioDriver* driver)
{
	CWasapiDriver* self = (CWasapiDriver*)driver;

	return self->wasapiif->GetPlayPosInSamples();
}
