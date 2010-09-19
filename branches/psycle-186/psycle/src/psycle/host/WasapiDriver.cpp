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

#include "Player.hpp"
#include <universalis/os/thread_name.hpp>

#define INITGUID //<< avoid additional linkage of static libs, uneeded code will be optimized out
#include <mmdeviceapi.h>
#include <functiondiscoverykeys.h>
#undef INITGUID

#define EXIT_ON_ERROR(hres)  \
  if (FAILED(hres)) { goto Exit; }

#define SAFE_RELEASE(punk)  \
  if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }


namespace portaudio {

	// ------------------------------------------------------------------------------------------
	// Aligns v backwards
	static inline UINT32 ALIGN_BWD(UINT32 v, UINT32 align)
	{
		return ((v - (align ? v % align : 0)));
	}
	// ------------------------------------------------------------------------------------------
	static inline double nano100ToMillis(const REFERENCE_TIME &ref)
	{
		//  1 nano = 0.000000001 seconds
		//100 nano = 0.0000001   seconds
		//100 nano = 0.0001   milliseconds
		return ((double)ref)*0.0001;
	}
	// ------------------------------------------------------------------------------------------
	static inline REFERENCE_TIME MillisTonano100(const double &ref)
	{
		//  1 nano = 0.000000001 seconds
		//100 nano = 0.0000001   seconds
		//100 nano = 0.0001   milliseconds
		return (REFERENCE_TIME)(ref/0.0001);
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
	static UINT32 AlignFramesPerBuffer(UINT32 nFrames, UINT32 nSamplesPerSec, UINT32 nBlockAlign)
	{
	#define HDA_PACKET_SIZE 128

		long packets_total = 10000 * (nSamplesPerSec * nBlockAlign / HDA_PACKET_SIZE);
		long frame_bytes   = nFrames * nBlockAlign;
		
		// align to packet size
		frame_bytes        = ALIGN_BWD(frame_bytes, HDA_PACKET_SIZE);
		nFrames            = frame_bytes / nBlockAlign;
		long packets       = frame_bytes / HDA_PACKET_SIZE;
		
		// align to packets count
		while (packets && ((packets_total % packets) != 0))
		{
			--packets;
		}
		
		frame_bytes = packets * HDA_PACKET_SIZE;
		nFrames     = frame_bytes / nBlockAlign;

		return nFrames;
	}
	// ------------------------------------------------------------------------------------------
	// Converts Hns period into number of frames
	static inline UINT32 MakeFramesFromHns(REFERENCE_TIME hnsPeriod, UINT32 nSamplesPerSec)
	{
		UINT32 nFrames = (UINT32)(	// frames =
			1.0 * hnsPeriod *		// hns *
			nSamplesPerSec /		// (frames / s) /
			1000 /					// (ms / s) /
			10000					// (hns / s) /
			+ 0.5					// rounding
		);
		return nFrames;
	}
}
namespace psycle
{
	namespace host
	{
		AudioDriverInfo WasapiDriver::_info = { "Windows WASAPI inteface" };

		const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
		const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
		const IID IID_IAudioClient = __uuidof(IAudioClient);
		const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

		WasapiDriver::WasapiDriver()
		:_pCallback(0)
		,running(false)
		,_initialized(false)
		{
			HRESULT hr = S_OK;
			IMMDeviceEnumerator *pEnumerator = NULL;

			hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
				IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			EXIT_ON_ERROR(hr)

			RefreshPorts(pEnumerator);
		Exit:
			SAFE_RELEASE(pEnumerator)
		}

		WasapiDriver::~WasapiDriver()
		{
		}

		void WasapiDriver::RefreshPorts(IMMDeviceEnumerator *pEnumerator) {
			HRESULT hr = S_OK;
			IMMDeviceCollection *pCollection = NULL;
			_playEnums.resize(0);
			_capEnums.resize(0);

		    hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE,
				&pCollection);
			EXIT_ON_ERROR(hr)

			FillPortList(_playEnums, pCollection);
			SAFE_RELEASE(pCollection)
				
		    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE,
				&pCollection);
			EXIT_ON_ERROR(hr)

			FillPortList(_capEnums, pCollection);

		Exit:
			SAFE_RELEASE(pCollection)
		}

		void WasapiDriver::FillPortList(std::vector<PortEnum> portList, IMMDeviceCollection *pCollection)
		{
			IMMDevice *pEndpoint = NULL;
			IPropertyStore *pProps = NULL;
            WCHAR* pszDeviceId = NULL;
			HRESULT hr = S_OK;
			UINT  count;

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
					wcstombs(pEnum.portName,   vars.pwszVal,MAX_STR_LEN-1);

					PropVariantClear(&vars);
				}

                // Default format
                {
                    PROPVARIANT vars;
                    PropVariantInit(&vars);

					hr = pProps->GetValue(PKEY_AudioEngine_DeviceFormat, &vars);
                    EXIT_ON_ERROR(hr);

					WAVEFORMATEXTENSIBLE format = { 0 };
					memcpy(&format, vars.blob.pBlobData, std::min<ULONG>(sizeof(format), vars.blob.cbSize));
					pEnum.DefaultFormat = format;

					PropVariantClear(&vars);
                }
				portList.push_back(pEnum);

				CoTaskMemFree(pszDeviceId);
				pszDeviceId = NULL;
				SAFE_RELEASE(pProps)
				SAFE_RELEASE(pEndpoint)
			}
		Exit:
			CoTaskMemFree(pszDeviceId);
			SAFE_RELEASE(pProps)
			SAFE_RELEASE(pEndpoint)
		}
		 void WasapiDriver::Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context)
		 {
 			_callbackContext = context;
			_pCallback = pCallback;
 			running = false;
			//ReadConfig();
			_initialized = true;
		 }
		 void WasapiDriver::Reset(void) {
			Stop();
		 }
// ------------------------------------------------------------------------------------------
/*static PaError GetClosestFormat(IAudioClient *myClient, double sampleRate, 
	const PaStreamParameters *params, AUDCLNT_SHAREMODE shareMode, WAVEFORMATEXTENSIBLE *outWavex)
{

    MakeWaveFormatFromParams(outWavex,params,sampleRate);
	WAVEFORMATEX *sharedClosestMatch = 0;
	HRESULT hResult = !S_OK;

	hResult = myClient->IsFormatSupported(shareMode, &outWavex->Format, (shareMode == AUDCLNT_SHAREMODE_SHARED ? &sharedClosestMatch : NULL));
	if (hResult == S_OK)
		answer = paFormatIsSupported;
    else
	if (sharedClosestMatch)
	{
        WAVEFORMATEXTENSIBLE *ext = (WAVEFORMATEXTENSIBLE*)sharedClosestMatch;

		GUID subf_guid = GUID_NULL;
		if (sharedClosestMatch->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			memcpy(outWavex, sharedClosestMatch, sizeof(WAVEFORMATEXTENSIBLE));
			subf_guid = ext->SubFormat;
		}
		else
			memcpy(outWavex, sharedClosestMatch, sizeof(WAVEFORMATEX));

        CoTaskMemFree(sharedClosestMatch);

		// Make supported by default
		answer = paFormatIsSupported;

		// Validate SampleRate
		if ((DWORD)sampleRate != outWavex->Format.nSamplesPerSec)
			return paInvalidSampleRate;

		// Validate Channel count
		if ((WORD)params->channelCount != outWavex->Format.nChannels)
			return paInvalidChannelCount;

		// Validate Sample format
		WORD bitsPerSample;
		if ((bitsPerSample = PaSampleFormatToBitsPerSample(params->sampleFormat)) == 0)
			return paSampleFormatNotSupported;

		// Validate Sample format: bit size (WASAPI does not limit 'bit size')
		//if (bitsPerSample != outWavex->Format.wBitsPerSample)
		//	return paSampleFormatNotSupported;

		// Validate Sample format: paFloat32 (WASAPI does not limit 'bit type')
		//if ((params->sampleFormat == paFloat32) && (subf_guid != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
		//	return paSampleFormatNotSupported;

		// Validate Sample format: paInt32 (WASAPI does not limit 'bit type')
		//if ((params->sampleFormat == paInt32) && (subf_guid != KSDATAFORMAT_SUBTYPE_PCM))
		//	return paSampleFormatNotSupported;
	}
	else
	{
		//it doesnt suggest anything?? ok lets show it the MENU!

#define FORMATTESTS 3
static const int BestToWorst[FORMATTESTS]={ paFloat32, paInt24, paInt16 };

		//ok fun time as with pa_win_mme, we know only a refusal of the user-requested
		//sampleRate+num Channel is disastrous, as the portaudio buffer processor converts between anything
		//so lets only use the number
		for (int i = 0; i < FORMATTESTS; ++i)
		{
			WAVEFORMATEXTENSIBLE ext = { 0 };
			wasapiFillWFEXT(&ext,BestToWorst[i],sampleRate,params->channelCount);
			
			hResult = myClient->IsFormatSupported(shareMode, &ext.Format, (shareMode == AUDCLNT_SHAREMODE_SHARED ? &sharedClosestMatch : NULL));
			if (hResult == S_OK)
			{
				memcpy(outWavex,&ext,sizeof(WAVEFORMATEXTENSIBLE));
				answer = paFormatIsSupported;
				break;
			}
		}

		if (answer != paFormatIsSupported)
		{
			// try MIX format?
			// why did it HAVE to come to this ....
			WAVEFORMATEX pcm16WaveFormat = { 0 };
			pcm16WaveFormat.wFormatTag		= WAVE_FORMAT_PCM;
			pcm16WaveFormat.nChannels		= 2;
			pcm16WaveFormat.nSamplesPerSec	= (DWORD)sampleRate;
			pcm16WaveFormat.nBlockAlign		= 4;
			pcm16WaveFormat.nAvgBytesPerSec = pcm16WaveFormat.nSamplesPerSec*pcm16WaveFormat.nBlockAlign;
			pcm16WaveFormat.wBitsPerSample	= 16;
			pcm16WaveFormat.cbSize			= 0;

			hResult = myClient->IsFormatSupported(shareMode, &pcm16WaveFormat, (shareMode == AUDCLNT_SHAREMODE_SHARED ? &sharedClosestMatch : NULL));
			if (hResult == S_OK)
			{
				memcpy(outWavex,&pcm16WaveFormat,sizeof(WAVEFORMATEX));
				answer = paFormatIsSupported;
			}
		}

		LogHostError(hResult);
	}

	return answer;
}
*/
		 HRESULT WasapiDriver::GetStreamFormat(WAVEFORMATEXTENSIBLE* pwfx) {
			/*HRESULT hr = pAudioClient->GetMixFormat(pwfx);
			_samplesPerSec = (*pwfx)->nSamplesPerSec;1
			this->_bitDepth = (*pwfx)->wBitsPerSample;
			this->_numBlocks = 2;
			this->_channelmode = (*pwfx)->nChannels;
			return hr;
			// Set up wave format structure. 
*/
			 _samplesPerSec = 48000;
			WAVEFORMATEXTENSIBLE format;
			memset(&format, 0, sizeof(WAVEFORMATEXTENSIBLE)); 
			format.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);
			format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			format.Format.nChannels = 2;
			format.Format.wBitsPerSample = 16;
			format.Format.nSamplesPerSec = _samplesPerSec;
			format.Format.nBlockAlign = format.Format.nChannels * format.Format.wBitsPerSample / 8;
			format.Format.nAvgBytesPerSec = format.Format.nSamplesPerSec * format.Format.nBlockAlign;
			format.Samples.wValidBitsPerSample = 16;
			format.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
			format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

			memcpy(pwfx,&format,sizeof(WAVEFORMATEXTENSIBLE));

			return out.client->IsFormatSupported(
				AUDCLNT_SHAREMODE_EXCLUSIVE,
				reinterpret_cast<WAVEFORMATEX*>(pwfx), NULL);
		 }

		 bool WasapiDriver::Start() {
			HRESULT hr;
			IMMDeviceEnumerator *pEnumerator = NULL;
			UINT32 framesPerLatency = 0;
			out.shareMode = AUDCLNT_SHAREMODE_EXCLUSIVE;
			out.streamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
			UINT32 millis = 20;
			if(running) return true;
			if(!_pCallback) return false;

			hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL,
				   CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
			EXIT_ON_ERROR(hr)

			///\todo: Select the appropiate endpoint.
			hr = pEnumerator->GetDefaultAudioEndpoint(
								eRender, eMultimedia, &out.device);
			EXIT_ON_ERROR(hr)

			hr = out.device->Activate(IID_IAudioClient, CLSCTX_ALL,
							NULL, (void**)&out.client);
			EXIT_ON_ERROR(hr)
/*
			// Get closest format
			if ((error = GetClosestFormat(params, pSubStream->shareMode, &pSubStream->wavex)) != paFormatIsSupported)
			{
				if (pa_error)
					(*pa_error) = error;
				return AUDCLNT_E_UNSUPPORTED_FORMAT;
			}
*/
			hr =GetStreamFormat(&out.wavex);
			EXIT_ON_ERROR(hr)

			// Add latency frames
			framesPerLatency += portaudio::MakeFramesFromHns(portaudio::MillisTonano100(millis), out.wavex.Format.nSamplesPerSec);

			// Align frames to HD Audio packet size of 128 bytes for Exclusive mode only.
			// Not aligning on Windows Vista will cause Event timeout, although Windows 7 will
			// return AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED error to realign buffer. Aligning is necessary
			// for Exclusive mode only! when audio data is feeded directly to hardware.
			if (out.shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE)
			{
				framesPerLatency = portaudio::AlignFramesPerBuffer(framesPerLatency, 
					out.wavex.Format.nSamplesPerSec, out.wavex.Format.nBlockAlign);
			}

			// Calculate period
			out.period = portaudio::MakeHnsPeriod(framesPerLatency, out.wavex.Format.nSamplesPerSec);
			
			// Enforce min/max period for device in Shared mode to avoid distorted sound.
			// Avoid doing so for Exclusive mode as alignment will suffer. Exclusive mode processes
			// big buffers without problem. Push Exclusive beyond limits if possible.
/*			if (out.shareMode == AUDCLNT_SHAREMODE_SHARED)
			{
				if (out.period < info->DefaultDevicePeriod)
					out.period = info->DefaultDevicePeriod;
			}
*/
			hr = out.client->Initialize(
				out.shareMode,
				out.streamFlags,
				out.period,
				(out.shareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? out.period : 0),
				reinterpret_cast<WAVEFORMATEX*>(&out.wavex),
				NULL);
			EXIT_ON_ERROR(hr)

			out.flags = 0;
			CreateThread( NULL, 0, AudioThread, this, 0, &dwThreadId );

			running = true;
			SAFE_RELEASE(pEnumerator)
			return true;
		Exit:
			SAFE_RELEASE(out.client)
			SAFE_RELEASE(out.device)
			SAFE_RELEASE(pEnumerator)
			return false;
		 }

		 bool WasapiDriver::Stop() {
			 if (!running) return false;
			 out.flags = AUDCLNT_BUFFERFLAGS_SILENT;
 			///\todo lock/unlock
			Sleep(256);
			SAFE_RELEASE(out.client)
			SAFE_RELEASE(out.device)
			return true;
		 }

		 DWORD WasapiDriver::AudioThread(void* pWasapi) {
			universalis::os::thread_name thread_name("wasapi sound output");
			universalis::cpu::exceptions::install_handler_in_thread();
			WasapiDriver * pThis = (WasapiDriver*) pWasapi;

			HRESULT hr;
			IAudioRenderClient *pRenderClient = NULL;
			HANDLE hEvent = NULL;
			HANDLE hTask = NULL;
			UINT32 bufferFrameCount;
			BYTE *pData;

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

			hr = pThis->out.client->GetService(
								 IID_IAudioRenderClient,
								 (void**)&pRenderClient);
			EXIT_ON_ERROR(hr)

			// To reduce latency, load the first buffer with data
			// from the audio source before starting the stream.
			hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
			EXIT_ON_ERROR(hr)

			float *pFloatBlock = pThis->_pCallback(pThis->_callbackContext, bufferFrameCount);
			/*if(_dither) QuantizeWithDither(pFloatBlock, pData, bufferFrameCount);
			else*/ Quantize(pFloatBlock, reinterpret_cast<int*>(pData), bufferFrameCount);


			hr = pRenderClient->ReleaseBuffer(bufferFrameCount, pThis->out.flags);
			EXIT_ON_ERROR(hr)

			// Ask MMCSS to temporarily boost the thread priority
			// to reduce glitches while the low-latency stream plays.
			DWORD taskIndex = 0;
			hTask = Global::pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
			if (hTask == NULL)
			{
				hr = E_FAIL;
				EXIT_ON_ERROR(hr)
			}

			hr = pThis->out.client->Start();  // Start playing.
			EXIT_ON_ERROR(hr)

			// Each loop fills one of the two buffers.
			while (pThis->out.flags != AUDCLNT_BUFFERFLAGS_SILENT)
			{
				// Wait for next buffer event to be signaled.
				DWORD retval = WaitForSingleObject(hEvent, 2000);
				if (retval != WAIT_OBJECT_0)
				{
					// Event handle timed out after a 2-second wait.
					pThis->out.client->Stop();
					hr = ERROR_TIMEOUT;
					goto Exit;
				}

				// Grab the next empty buffer from the audio device.
				hr = pRenderClient->GetBuffer(bufferFrameCount, &pData);
				//EXIT_ON_ERROR(hr)
				  if (FAILED(hr))
				  {
					  goto Exit;
				  }

				float *pFloatBlock = pThis->_pCallback(pThis->_callbackContext, bufferFrameCount);
				/*if(_dither) QuantizeWithDither(pFloatBlock, pData, bufferFrameCount);
				else*/ Quantize(pFloatBlock, reinterpret_cast<int*>(pData), bufferFrameCount);

				hr = pRenderClient->ReleaseBuffer(bufferFrameCount, pThis->out.flags);
				//EXIT_ON_ERROR(hr)
				  if (FAILED(hr)) 
				  {
					  goto Exit;
				  }
			}
			hr = pThis->out.client->Stop();  // Stop playing.
			EXIT_ON_ERROR(hr)

		Exit:
			if (hEvent != NULL) { CloseHandle(hEvent); }
			if (hTask != NULL) { Global::pAvRevertMmThreadCharacteristics(hTask); }
			SAFE_RELEASE(pRenderClient)
			pThis->running =false;
			return 0;
		 }

		 void WasapiDriver::ReadConfig() {
		 }
		 void WasapiDriver::WriteConfig() {
		 }
		 void WasapiDriver::GetCapturePorts(std::vector<std::string> &ports)
		 {
			ports.resize(0);
			//for (unsigned int i=0;i<_capEnums.size();i++) ports.push_back(_capEnums[i].portName);
		 }
		//todo
		 void WasapiDriver::GetReadBuffers(int idx, float **pleft, float **pright,int numsamples) { pleft=0; pright=0; return; }
		//todo
		 bool WasapiDriver::AddCapturePort(int idx){ return false; };
		//todo
		 bool WasapiDriver::RemoveCapturePort(int idx){ return false; }
		//todo
		 int WasapiDriver::GetWritePos() { return 0; }
		//todo
		 int WasapiDriver::GetPlayPos() { return 0; }
		//todo
		 int WasapiDriver::GetInputLatency() { return 0; }
		//todo
		 int WasapiDriver::GetOutputLatency() { return 0; }
		//todo
		 void WasapiDriver::Configure(void) {};
		//todo
		 bool WasapiDriver::Configured(void) { return true; }
	}
}

