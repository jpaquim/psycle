///\file
///\brief implementation file for psycle::host::AudioDriver.

// Portions of this code belongs to portaudio's WASAPI driver.
// Portable Audio I/O Library WASAPI implementation
// Copyright (c) 2006-2010 David Viens, Dmitry Kostjuchenko

// http://msdn.microsoft.com/en-us/library/dd370844%28v=VS.85%29.aspx
// http://msdn.microsoft.com/en-us/library/dd316756%28v=VS.85%29.aspx
// http://msdn.microsoft.com/en-us/library/dd370876%28v=VS.85%29.aspx
#include "WasapiDriver.hpp"
#include "Constants.hpp"
#include "WasapiConfig.hpp"
#include "Registry.hpp"
#include "Configuration.hpp"
#include <psycle/helpers/dsp.hpp>


#include <universalis/os/thread_name.hpp>
#include <universalis/os/aligned_alloc.hpp>

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
		AudioDriverInfo WasapiDriver::_info = { "Windows WASAPI inteface" };
		AudioDriverEvent WasapiDriver::_event;

		const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
		const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
		const IID IID_IAudioClient = __uuidof(IAudioClient);
		const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
		const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
		const IID IID_IAudioClock = __uuidof(IAudioClock);

		void WasapiDriver::Error(const TCHAR msg[])
		{
			MessageBox(0, msg, _T("DirectSound Output driver"), MB_OK | MB_ICONERROR);
		}

		WasapiDriver::WasapiDriver()
			:_pCallback(0)
			,running(false)
			,_initialized(false)
			,_configured(false)
			,writeMark(0)
			,_callbackContext(0)
			,dwThreadId(0)
			,dither(false)
			,pAudioClock(0)
		{
			ZeroMemory(&out, sizeof(PaWasapiSubStream));
		}

		WasapiDriver::~WasapiDriver()
		{
			Stop();
		}

		void WasapiDriver::Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context)
		{
			_callbackContext = context;
			_pCallback = pCallback;
			running = false;
			ReadConfig();
			HRESULT hr = S_OK;
			IMMDeviceEnumerator *pEnumerator = NULL;

			hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
				IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			EXIT_ON_ERROR(hr)

			RefreshPorts(pEnumerator);

Exit:
			SAFE_RELEASE(pEnumerator)
				_initialized = true;
		}
		void WasapiDriver::Reset(void){
			Stop();
		}

		bool WasapiDriver::Start(){
			HRESULT hr;
			IMMDeviceEnumerator *pEnumerator = NULL;
			out.streamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
			if(running) return true;
			if(!_pCallback) return false;

			hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
				IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			EXIT_ON_ERROR(hr)

			if(wcscmp(out.szDeviceID,L"")==0) {
				hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &out.device);
			}
			else {
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
				_numBlocks = 2;
			}
			else {
				framesPerLatency = GetBufferSamples();
				if(framesPerLatency == 0) {
					hr = out.client->GetDevicePeriod(NULL,&out.period);
					EXIT_ON_ERROR(hr)
					// Add latency frames
					framesPerLatency = portaudio::MakeFramesFromHns(out.period, out.wavex.Format.nSamplesPerSec);
				}
				_numBlocks = 2;
			}
			// Align frames to HD Audio packet size of 128 bytes 
			framesPerLatency = portaudio::AlignFramesPerBuffer(framesPerLatency,
					out.wavex.Format.nSamplesPerSec, out.wavex.Format.nBlockAlign);

			// Calculate period
			out.period = portaudio::MakeHnsPeriod(framesPerLatency, out.wavex.Format.nSamplesPerSec);
			_blockSizeBytes = framesPerLatency * GetSampleSizeBytes();
	
			hr = out.client->Initialize(
				out.shareMode,
				out.streamFlags,
				out.period,
				(out.shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? out.period : 0),
				reinterpret_cast<WAVEFORMATEX*>(&out.wavex),
				NULL);
			EXIT_ON_ERROR(hr)

			hr= out.client->GetBufferSize(&out.bufferFrameCount);
			EXIT_ON_ERROR(hr)
			_blockSizeBytes = out.bufferFrameCount * GetSampleSizeBytes();
			

			hr = out.client->GetService(IID_IAudioClock, (void**)&pAudioClock);
			EXIT_ON_ERROR(hr)
			hr = pAudioClock->GetFrequency(&audioClockFreq);
			EXIT_ON_ERROR(hr)


			for (unsigned int i=0; i<_capPorts.size();i++) {
				//Ignore errors creating the Capture port to allow the playback to work.
				CreateCapturePort(pEnumerator, _capPorts[i]);
			}

			out.flags = 0;
			_event.ResetEvent();
			CreateThread( NULL, 0, EventAudioThread, this, 0, &dwThreadId );

			running = true;
			SAFE_RELEASE(pEnumerator)
			return true;
Exit:
			//For debugging purposes
			if(FAILED(hr)) {
				Error(GetError(hr));
			}
			SAFE_RELEASE(pAudioClock);
			SAFE_RELEASE(out.client)
			SAFE_RELEASE(out.device)
			SAFE_RELEASE(pEnumerator)
			return false;
		}

		bool WasapiDriver::Stop(){
			if (!running) return false;
			out.flags = AUDCLNT_BUFFERFLAGS_SILENT;
			CSingleLock event(&_event, TRUE);
			for (unsigned int i =0; i < _capPorts.size(); i++)
			{
				SAFE_RELEASE(_capPorts[i].client)
				SAFE_RELEASE(_capPorts[i].device)
			}
			SAFE_RELEASE(pAudioClock);
			SAFE_RELEASE(out.client)
			SAFE_RELEASE(out.device)
			return true;
		}

		bool WasapiDriver::AddCapturePort(int idx){ 
			bool isplaying = running;
			if ( idx >= _capEnums.size() ) return false;
			if ( idx < _portMapping.size() && _portMapping[idx] != -1) return true;
			
			if (isplaying) Stop();
			PaWasapiSubStream port;
			ZeroMemory(&port, sizeof(PaWasapiSubStream));
			wcscpy_s(port.szDeviceID, MAX_STR_LEN-1, _capEnums[idx].szDeviceID);
			_capPorts.push_back(port);
			if ( _portMapping.size() <= idx) {
				int oldsize = _portMapping.size();
				_portMapping.resize(idx+1);
				for(int i=oldsize;i<_portMapping.size();i++) _portMapping[i]=-1;
			}
			_portMapping[idx]=(int)(_capPorts.size()-1);
			if (isplaying) return Start();
			return true;
		};

		bool WasapiDriver::RemoveCapturePort(int idx){
			bool isplaying = running;
			int maxSize = 0;
			std::vector<PaWasapiSubStream> newports;
			if ( idx >= _capEnums.size() || 
				 idx >= _portMapping.size() || _portMapping[idx] == -1) return false;
			
			if (isplaying) Stop();
			for (unsigned int i=0;i<_portMapping.size();++i)
			{
				if (i != idx && _portMapping[i] != -1) {
					maxSize=i+1;
					newports.push_back(_capPorts[_portMapping[i]]);
					_portMapping[i]= (int)(newports.size()-1);
				}
			}
			_portMapping[idx] = -1;
			if(maxSize < _portMapping.size()) _portMapping.resize(maxSize);
			_capPorts = newports;
			if (isplaying) Start();
			return true;
		}

		HRESULT WasapiDriver::CreateCapturePort(IMMDeviceEnumerator* pEnumerator, PaWasapiSubStream &port)
		{
			//not try to open a port twice
			if(port.client != NULL) return true;
			port._machinepos=0;
			HRESULT hr;
			if(wcscmp(port.szDeviceID, L"") == 0) {
				hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &port.device);
			}
			else {
				hr = pEnumerator->GetDevice(port.szDeviceID, &port.device);
			}
			EXIT_ON_ERROR(hr)

			hr = port.device->Activate(IID_IAudioClient, CLSCTX_ALL,
				NULL, (void**)&port.client);
			EXIT_ON_ERROR(hr)

			port.shareMode= out.shareMode;
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
			universalis::os::aligned_memory_alloc(16, port.pleft, 2*_blockSizeBytes);
			universalis::os::aligned_memory_alloc(16, port.pright, 2*_blockSizeBytes);
			ZeroMemory(port.pleft, 2*_blockSizeBytes);
			ZeroMemory(port.pright, 2*_blockSizeBytes);
			return hr;
Exit:
			Error("Couldn't open the capture device. Possibly the format is not supported");
			SAFE_RELEASE(port.client)
			SAFE_RELEASE(port.device)
			return hr;
		}

		DWORD WasapiDriver::EventAudioThread(void* pWasapi){
			HRESULT hr;
			HANDLE hEvent = NULL;
			HANDLE hTask = NULL;
			UINT32 bufferFrameCount;
			IAudioRenderClient *pRenderClient = NULL;
			std::vector<IAudioCaptureClient*> capture;

			universalis::os::thread_name thread_name("wasapi sound output");
			universalis::cpu::exceptions::install_handler_in_thread();
			WasapiDriver * pThis = (WasapiDriver*) pWasapi;
			// Ask MMCSS to temporarily boost the thread priority
			// to reduce glitches while the low-latency stream plays.
			DWORD taskIndex = 0;
			hTask = Global::pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
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

			for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
			{
				if(pThis->_capPorts[i].client == NULL)
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
					for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
					{
						if(pThis->_capPorts[i].client == NULL)
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
					if(numFramesAvailable > 0){
						// First, run the capture buffers so that audio is available to wavein machines.
						for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
						{
							if(pThis->_capPorts[i].client == NULL) 
								continue;
							hr = pThis->DoBlockRecording(pThis->_capPorts[i],capture[i],numFramesAvailable);
						}
						hr = pThis->DoBlock(pRenderClient, numFramesAvailable);
						EXIT_ON_ERROR(hr)
					}
				}
				else {
					// First, run the capture buffers so that audio is available to wavein machines.
					for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
					{
						if(pThis->_capPorts[i].client == NULL) 
							continue;
						hr = pThis->DoBlockRecording(pThis->_capPorts[i],capture[i],bufferFrameCount);
					}
					hr = pThis->DoBlock(pRenderClient, bufferFrameCount);
					EXIT_ON_ERROR(hr)
				}
			}

			for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
			{
				if(pThis->_capPorts[i].client == NULL) 
					continue;
				hr = pThis->_capPorts[i].client->Stop(); // Start playing.
			}
			hr = pThis->out.client->Stop(); // Stop playing.
Exit:
			//For debugging purposes
			if(FAILED(hr)) {
				Error(GetError(hr));
			}
			if (hEvent != NULL) { CloseHandle(hEvent); }
			if (hTask != NULL) { Global::pAvRevertMmThreadCharacteristics(hTask); }
			for (unsigned int i =0; i < capture.size(); i++)
			{
				SAFE_RELEASE(capture[i])
			}

			SAFE_RELEASE(pRenderClient)
			pThis->running =false;
			_event.SetEvent();
			return 0;
		}
		HRESULT WasapiDriver::DoBlockRecording(PaWasapiSubStream& port, IAudioCaptureClient *pCaptureClient, int numFramesAvailable)
		{
			BYTE *pData;
			DWORD flags;
			UINT32 packetLength=0;
			UINT32 numf;

			HRESULT hr = pCaptureClient->GetNextPacketSize(&packetLength);
			EXIT_ON_ERROR(hr)

			while (packetLength != 0 && packetLength <= numFramesAvailable)
			{
				///\todo: finish this implementation.
				HRESULT hr = pCaptureClient->GetBuffer(&pData,&numf, &flags,NULL,NULL);
				EXIT_ON_ERROR(hr)
				if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
				{
					if (_sampleValidBits == 32) {
						DeinterlaceFloat(reinterpret_cast<float*>(pData), port.pleft,port.pright, numf);
					}
					else if (_sampleValidBits == 24) {
						DeQuantize32AndDeinterlace(reinterpret_cast<int*>(pData), port.pleft,port.pright, numf);
					}
					else {
						DeQuantize16AndDeinterlace(reinterpret_cast<short int*>(pData), port.pleft,port.pright, numf);
					}
				}
				hr = pCaptureClient->ReleaseBuffer(numf);
				EXIT_ON_ERROR(hr)
				numFramesAvailable -= numf;
				hr = pCaptureClient->GetNextPacketSize(&packetLength);
				EXIT_ON_ERROR(hr)
			}
Exit:
			port._machinepos=0;
			return hr;
		}
		HRESULT WasapiDriver::DoBlock(IAudioRenderClient *pRenderClient, int numFramesAvailable){
			BYTE *pData;
			// Grab the next empty buffer from the audio device.
			HRESULT hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
			EXIT_ON_ERROR(hr)

			float *pFloatBlock = _pCallback(_callbackContext, numFramesAvailable);
			if(_sampleValidBits == 32) {
				dsp::MovMul(pFloatBlock, reinterpret_cast<float*>(pData), numFramesAvailable*2, 1.f/32768.f);
			}
			else if(_sampleValidBits == 24) {
				Quantize24in32Bit(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
			}
			else if (_sampleValidBits == 16) {
				if(dither) Quantize16WithDither(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
				else Quantize16(pFloatBlock, reinterpret_cast<int*>(pData), numFramesAvailable);
			}

			hr = pRenderClient->ReleaseBuffer(numFramesAvailable, out.flags);
			writeMark+=numFramesAvailable;
Exit:
			return hr;
		}
		void WasapiDriver::RefreshAvailablePorts()
		{
			IMMDeviceEnumerator *pEnumerator = NULL;
			HRESULT hr = S_OK;

				hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
					IID_IMMDeviceEnumerator, (void**)&pEnumerator);
				EXIT_ON_ERROR(hr)

				RefreshPorts(pEnumerator);
Exit:
			SAFE_RELEASE(pEnumerator)
		}
		void WasapiDriver::GetPlaybackPorts(std::vector<std::string> &ports)
		{
			ports.resize(0);
			for (unsigned int i=0;i<_playEnums.size();i++) ports.push_back(_playEnums[i].portName);
		}

		void WasapiDriver::GetCapturePorts(std::vector<std::string> &ports)
		{
			ports.resize(0);
			for (unsigned int i=0;i<_capEnums.size();i++) ports.push_back(_capEnums[i].portName);
		}

		void WasapiDriver::GetReadBuffers(int idx, float **pleft, float **pright,int numsamples){
			if (!running || idx >=_portMapping.size() || _portMapping[idx] == -1 
				|| _capPorts[_portMapping[idx]].client == NULL)
			{
				*pleft=0;
				*pright=0;
				return;
			}
			int mpos = _capPorts[_portMapping[idx]]._machinepos;
			*pleft=_capPorts[_portMapping[idx]].pleft+mpos;
			*pright=_capPorts[_portMapping[idx]].pright+mpos;
			_capPorts[_portMapping[idx]]._machinepos+=numsamples;
		}

		void WasapiDriver::ReadConfig(){
			// default configuration
			_channelMode = stereo;
			_samplesPerSec = 44100;
			_sampleBits=32;
			_sampleValidBits=24;
			_blockSizeBytes = 2048;
			_numBlocks = 2;
			out.shareMode = AUDCLNT_SHAREMODE_SHARED;
			dither = true;
			_configured = true;

			// read from registry
			Registry reg;
			reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT);
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\wasapi") != ERROR_SUCCESS) // settings in version 1.8
			{
				reg.CloseRootKey();
				return;
			}
			bool configured(true);
			bool shared;
			int samples;
			configured &= ERROR_SUCCESS == reg.QueryValue("DeviceString", out.szDeviceID);
			configured &= ERROR_SUCCESS == reg.QueryValue("Shared", shared);
			configured &= ERROR_SUCCESS == reg.QueryValue("Dither", dither);
			configured &= ERROR_SUCCESS == reg.QueryValue("BitDepth", _sampleValidBits);
			configured &= ERROR_SUCCESS == reg.QueryValue("BufferSize", samples);
			configured &= ERROR_SUCCESS == reg.QueryValue("SamplesPerSec", _samplesPerSec);
			if (configured) {
				if(_sampleValidBits == 24) _sampleBits=32;
				else _sampleBits=_sampleValidBits;
				out.shareMode = shared ? AUDCLNT_SHAREMODE_SHARED : AUDCLNT_SHAREMODE_EXCLUSIVE;
				_blockSizeBytes = samples * GetSampleSizeBytes();
			}
			reg.CloseKey();
			reg.CloseRootKey();
			_configured = configured;
		}

		void WasapiDriver::WriteConfig(){
			Registry reg;
			if(reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT) != ERROR_SUCCESS)
			{
				Error("Unable to write configuration to the registry");
				return;
			}
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\wasapi") != ERROR_SUCCESS)
			{
				if(reg.CreateKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\wasapi") != ERROR_SUCCESS)
				{
					Error("Unable to write configuration to the registry");
					return;
				}
			}
			bool shared = (out.shareMode == AUDCLNT_SHAREMODE_SHARED) ? true : false;
			int samples = GetBufferSamples();
			reg.SetValue("DeviceString", out.szDeviceID);
			reg.SetValue("Shared", shared);
			reg.SetValue("Dither", dither);
			reg.SetValue("BitDepth", _sampleValidBits);
			reg.SetValue("SamplesPerSec", _samplesPerSec);
			reg.SetValue("BufferSize", samples);
			reg.CloseKey();
			reg.CloseRootKey();
		}
		void WasapiDriver::Configure(void)
		{
			if(!_configured) ReadConfig();

			WasapiConfig dlg;
			dlg.m_driverIndex = GetIdxFromDevice(out.szDeviceID);
			dlg.m_sampleRate = _samplesPerSec;
			dlg.m_bitDepth = _sampleValidBits;
			dlg.m_dither = dither;
			dlg.m_shareMode = (out.shareMode == AUDCLNT_SHAREMODE_SHARED)? 1: 0;
			dlg.m_bufferSize = GetBufferSamples();
			dlg.wasapi = this;

			if(dlg.DoModal() != IDOK) return;

			AUDCLNT_SHAREMODE sharemode = (dlg.m_shareMode == 0)? AUDCLNT_SHAREMODE_EXCLUSIVE : AUDCLNT_SHAREMODE_SHARED;
			if(dlg.m_shareMode == 0) {
				WAVEFORMATPCMEX wf;
				PrepareWaveFormat(wf, 2, dlg.m_sampleRate, (dlg.m_bitDepth==24)?32:dlg.m_bitDepth, dlg.m_bitDepth);
				bool supported = _playEnums[dlg.m_driverIndex].IsFormatSupported(wf,sharemode, false);
				if(!supported) {
					Error("The Format selected is not supported. Keeping the previous configuration");
					return;
				}
			}

			wcscpy_s(out.szDeviceID, MAX_STR_LEN-1, _playEnums[dlg.m_driverIndex].szDeviceID);
			_samplesPerSec = dlg.m_sampleRate;
			_sampleValidBits = dlg.m_bitDepth;
			if(_sampleValidBits == 24) _sampleBits = 32;
			else _sampleBits = _sampleValidBits;
			dither = dlg.m_dither;
			_blockSizeBytes = dlg.m_bufferSize * GetSampleSizeBytes();
			out.shareMode = sharemode;

			_configured = true;
			WriteConfig();
		
		};

		void WasapiDriver::RefreshPorts(IMMDeviceEnumerator *pEnumerator){
			HRESULT hr = S_OK;
			IMMDeviceCollection *pCollection = NULL;
			IMMDevice *pEndpoint = NULL;
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

			FillPortList(_capEnums, pCollection,defaultID);

Exit:
			::CoTaskMemFree(defaultID);
			SAFE_RELEASE(pEndpoint)
			SAFE_RELEASE(pCollection)
		}

		void WasapiDriver::FillPortList(std::vector<PortEnum>& portList, IMMDeviceCollection *pCollection, LPWSTR defaultID)
		{
			IMMDevice *pEndpoint = NULL;
			IPropertyStore *pProps = NULL;
			IAudioClient *client = NULL;
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

					wcscpy_s(pEnum.szDeviceID, MAX_STR_LEN-1, pszDeviceId);
					wcstombs(pEnum.portName, vars.pwszVal,MAX_STR_LEN-1);

					PropVariantClear(&vars);
					if(wcscmp(pEnum.szDeviceID,defaultID) == 0) {
						pEnum.isDefault = true;
					}
				}

				// Default format
				{
					PROPVARIANT vars;
					PropVariantInit(&vars);

					hr = pProps->GetValue(PKEY_AudioEngine_DeviceFormat, &vars);
					EXIT_ON_ERROR(hr);

					ZeroMemory(&pEnum.MixFormat,sizeof(WAVEFORMATEXTENSIBLE));
					memcpy(&pEnum.MixFormat, vars.blob.pBlobData, std::min<ULONG>(sizeof(WAVEFORMATEXTENSIBLE), vars.blob.cbSize));
					PropVariantClear(&vars);
				}
				//lantency
				{
					hr = pEndpoint->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client);
					EXIT_ON_ERROR(hr)

					hr = client->GetDevicePeriod(&pEnum.DefaultDevicePeriod,&pEnum.MinimumDevicePeriod);
					EXIT_ON_ERROR(hr)
				}
				{
					WAVEFORMATEX* pwft;
					hr = client->GetMixFormat(&pwft);
					EXIT_ON_ERROR(hr);
					memcpy(&pEnum.MixFormat,pwft,std::min(sizeof(WAVEFORMATEX) + pwft->cbSize, sizeof(WAVEFORMATEXTENSIBLE)));
					if (pwft->cbSize == 0) pEnum.MixFormat.Samples.wValidBitsPerSample = pwft->wBitsPerSample;
					::CoTaskMemFree(pwft);
				}
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
			if (stream.shareMode == AUDCLNT_SHAREMODE_SHARED)
			{
				WAVEFORMATEX* pwft;
				hr = stream.client->GetMixFormat(&pwft);
				EXIT_ON_ERROR(hr)
				_samplesPerSec = pwft->nSamplesPerSec;
				_sampleBits = pwft->wBitsPerSample;
				if(pwft->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
					WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)pwft;
					_sampleValidBits = wfex->Samples.wValidBitsPerSample;
				}
				else {
					_sampleValidBits = _sampleBits;
				}
				memcpy(wfOut,pwft,std::min(sizeof(WAVEFORMATEX) + pwft->cbSize, sizeof(WAVEFORMATEXTENSIBLE)));
				::CoTaskMemFree(pwft);
			}
			else
			{
				WAVEFORMATPCMEX format;
				PrepareWaveFormat(format, 2, _samplesPerSec, _sampleBits, _sampleValidBits);
				memcpy(wfOut,&format,sizeof(WAVEFORMATPCMEX));
			}
			WAVEFORMATEX* bla = NULL;
			hr = stream.client->IsFormatSupported(stream.shareMode, reinterpret_cast<WAVEFORMATEX*>(wfOut),
				(stream.shareMode == AUDCLNT_SHAREMODE_SHARED)? &bla : NULL);
			if(bla != NULL) { ::CoTaskMemFree(bla); }
			return hr;
Exit:
			return hr;
		}

		std::uint32_t WasapiDriver::GetWritePosInSamples() { return writeMark; }

		std::uint32_t WasapiDriver::GetPlayPosInSamples() {
			UINT64 pos;
			std::uint32_t retVal = 0;
			if(running) {
				HRESULT hr = pAudioClock->GetPosition(&pos,NULL);
				EXIT_ON_ERROR(hr)
				if(audioClockFreq == _samplesPerSec) {
					retVal = pos;
				}
				else {
					//Thus, the stream-relative offset in seconds can always be calculated as p/f.
					retVal = (pos*_samplesPerSec/audioClockFreq);
				}
			}
Exit:
			return retVal;
		}

		std::uint32_t WasapiDriver::GetInputLatencyMs(){
			if(_capPorts.size() > 0) {
				///\todo: The documentation suggests that the period has to be added to the latency. verify it.
				return portaudio::nano100ToMillis(_capPorts[0].device_latency+_capPorts[0].period);
			}
			else return 0;
		}

		std::uint32_t WasapiDriver::GetOutputLatencyMs(){
			///\todo: The documentation suggests that the period has to be added to the latency. verify it.
			return portaudio::nano100ToMillis(out.device_latency+out.period);
		}

		bool WasapiDriver::PortEnum::IsFormatSupported(WAVEFORMATEXTENSIBLE& pwfx, AUDCLNT_SHAREMODE sharemode, bool isInput)
		{
			IMMDeviceEnumerator *pEnumerator = NULL;
			IMMDevice			*device = NULL;
			IAudioClient        *client = NULL;
			bool issuccess=false;
			HRESULT hr;
			hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
				IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			EXIT_ON_ERROR(hr)

			hr = pEnumerator->GetDevice(szDeviceID, &device);
			EXIT_ON_ERROR(hr)

			hr = device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client);
			EXIT_ON_ERROR(hr)

			WAVEFORMATEX* bla = NULL;
			hr = client->IsFormatSupported(sharemode, reinterpret_cast<WAVEFORMATEX*>(&pwfx),
				(sharemode == AUDCLNT_SHAREMODE_SHARED)? &bla : NULL);
			if(bla != NULL) { ::CoTaskMemFree(bla); }
			if(hr==S_OK) issuccess = true;
Exit:
			SAFE_RELEASE(client)
			SAFE_RELEASE(device)
			SAFE_RELEASE(pEnumerator)
			return issuccess;
		}

		std::uint32_t WasapiDriver::GetIdxFromDevice(WCHAR* szDeviceID) {
			for(int i = 0; i < _playEnums.size() ; ++i)
			{
				if(wcscmp(_playEnums[i].szDeviceID,szDeviceID) == 0)
				{
					return i;
				}
			}
			return 0;
		}

		const char* WasapiDriver::GetError(HRESULT hr) {
			const char *text = NULL;
			switch (hr)
			{
			case S_OK:									return "";
			case E_POINTER                              :text ="E_POINTER"; break;
			case E_INVALIDARG                           :text ="E_INVALIDARG"; break;

			case AUDCLNT_E_NOT_INITIALIZED              :text ="AUDCLNT_E_NOT_INITIALIZED"; break;
			case AUDCLNT_E_ALREADY_INITIALIZED          :text ="AUDCLNT_E_ALREADY_INITIALIZED"; break;
			case AUDCLNT_E_WRONG_ENDPOINT_TYPE          :text ="AUDCLNT_E_WRONG_ENDPOINT_TYPE"; break;
			case AUDCLNT_E_DEVICE_INVALIDATED           :text ="AUDCLNT_E_DEVICE_INVALIDATED"; break;
			case AUDCLNT_E_NOT_STOPPED                  :text ="AUDCLNT_E_NOT_STOPPED"; break;
			case AUDCLNT_E_BUFFER_TOO_LARGE             :text ="AUDCLNT_E_BUFFER_TOO_LARGE"; break;
			case AUDCLNT_E_OUT_OF_ORDER                 :text ="AUDCLNT_E_OUT_OF_ORDER"; break;
			case AUDCLNT_E_UNSUPPORTED_FORMAT           :text ="AUDCLNT_E_UNSUPPORTED_FORMAT"; break;
			case AUDCLNT_E_INVALID_SIZE                 :text ="AUDCLNT_E_INVALID_SIZE"; break;
			case AUDCLNT_E_DEVICE_IN_USE                :text ="AUDCLNT_E_DEVICE_IN_USE"; break;
			case AUDCLNT_E_BUFFER_OPERATION_PENDING     :text ="AUDCLNT_E_BUFFER_OPERATION_PENDING"; break;
			case AUDCLNT_E_THREAD_NOT_REGISTERED        :text ="AUDCLNT_E_THREAD_NOT_REGISTERED"; break;
			case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED   :text ="AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"; break;
			case AUDCLNT_E_ENDPOINT_CREATE_FAILED       :text ="AUDCLNT_E_ENDPOINT_CREATE_FAILED"; break;
			case AUDCLNT_E_SERVICE_NOT_RUNNING          :text ="AUDCLNT_E_SERVICE_NOT_RUNNING"; break;
			case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED     :text ="AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"; break;
			case AUDCLNT_E_EXCLUSIVE_MODE_ONLY          :text ="AUDCLNT_E_EXCLUSIVE_MODE_ONLY"; break;
			case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL :text ="AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"; break;
			case AUDCLNT_E_EVENTHANDLE_NOT_SET          :text ="AUDCLNT_E_EVENTHANDLE_NOT_SET"; break;
			case AUDCLNT_E_INCORRECT_BUFFER_SIZE        :text ="AUDCLNT_E_INCORRECT_BUFFER_SIZE"; break;
			case AUDCLNT_E_BUFFER_SIZE_ERROR            :text ="AUDCLNT_E_BUFFER_SIZE_ERROR"; break;
			case AUDCLNT_E_CPUUSAGE_EXCEEDED            :text ="AUDCLNT_E_CPUUSAGE_EXCEEDED"; break;
			case AUDCLNT_E_BUFFER_ERROR					:text ="AUDCLNT_E_BUFFER_ERROR"; break;
			case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED		:text ="AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED"; break;
			case AUDCLNT_E_INVALID_DEVICE_PERIOD		:text ="AUDCLNT_E_INVALID_DEVICE_PERIOD"; break;

			case AUDCLNT_S_BUFFER_EMPTY                 :text ="AUDCLNT_S_BUFFER_EMPTY"; break;
			case AUDCLNT_S_THREAD_ALREADY_REGISTERED    :text ="AUDCLNT_S_THREAD_ALREADY_REGISTERED"; break;
			case AUDCLNT_S_POSITION_STALLED				:text ="AUDCLNT_S_POSITION_STALLED"; break;

			// other windows common errors:
			case CO_E_NOTINITIALIZED                    :text ="CO_E_NOTINITIALIZED"; break;

			default:
				text = "UNKNOWN ERROR";
			}
			return text;
		}
	}
}

