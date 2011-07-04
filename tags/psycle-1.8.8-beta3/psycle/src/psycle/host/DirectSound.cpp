///\file
///\implementation psycle::host::DirectSound.


#include "DirectSound.hpp"
#include "DSoundConfig.hpp"
#include "Registry.hpp"
#include "Configuration.hpp"
#include "MidiInput.hpp"
#include <universalis.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/os/aligned_alloc.hpp>
#include <process.h>
#include <psycle/helpers/dsp.hpp>
#include "cpu_time_clock.hpp"

#define SAFE_RELEASE(punk) \
	if ((punk) != NULL) { (punk)->Release(); (punk) = NULL; }

namespace psycle
{
	namespace host
	{
		AudioDriverInfo DirectSound::_info = { "DirectSound Output" };
		AudioDriverEvent DirectSound::_event;

		void DirectSound::Error(const TCHAR msg[])
		{
			MessageBox(0, msg, _T("DirectSound Output driver"), MB_OK | MB_ICONERROR);
		}

		DirectSound::DirectSound()
		:
			device_guid(DSDEVID_DefaultPlayback),
			_initialized(false),
			_configured(false),
			_running(false),
			_playing(false),
			_threadRun(false),
			_pDs(0),
			_pBuffer(0),
			_pCallback(0)
		{
			_capEnums.resize(0);
			_capPorts.resize(0);
		}

		DirectSound::~DirectSound() throw()
		{
			Reset();
		}

		void DirectSound::Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void * context)
		{
			_callbackContext = context;
			_pCallback = pCallback;
			_running = false;
			_hwnd = hwnd;
			ReadConfig();
			if( FAILED( ::CoInitialize(NULL) ) )
			{
				Error(_T("(DirectSound) Failed to initialize COM"));
				return;
			}
			RefreshAvailablePorts();
			_initialized = true;
		}

		void DirectSound::RefreshAvailablePorts()
		{
			_playEnums.resize(0);
			_capEnums.resize(0);
			DirectSoundEnumerate(DSEnumCallback, &_playEnums);
			DirectSoundCaptureEnumerate(DSCaptureEnumCallback,&_capEnums);
		}
		void DirectSound::Reset()
		{
			Stop();
			// Release COM
			CoUninitialize();
		}

		bool DirectSound::Start()
		{
//			CSingleLock lock(&_lock, TRUE);
			if(_running) return true;
			if(!_pCallback) return false;

			if(FAILED(::DirectSoundCreate8(&device_guid, &_pDs, NULL)))
			{
				Error(_T("Failed to create DirectSound object"));
				return false;
			}
			if(FAILED(_pDs->SetCooperativeLevel(_hwnd, DSSCL_PRIORITY)))
			{
				Error(_T("Failed to set DirectSound cooperative level"));
				SAFE_RELEASE(_pDs)
				return false;
			}

			_dsBufferSize = _blockSizeBytes*_numBlocks;

			WAVEFORMATPCMEX wf;
			PrepareWaveFormat(wf, (_channelMode == stereo) ? 2 : 1, _samplesPerSec, _sampleBits, _sampleValidBits);

			DSBUFFERDESC desc;
			ZeroMemory( &desc, sizeof(DSBUFFERDESC) );
			desc.dwSize = sizeof(DSBUFFERDESC); 
			desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
			desc.dwBufferBytes = _dsBufferSize; 
			desc.lpwfxFormat = reinterpret_cast<LPWAVEFORMATEX>(&wf);
			desc.guid3DAlgorithm = GUID_NULL;
			LPDIRECTSOUNDBUFFER pBufferGen;
			if(FAILED(_pDs->CreateSoundBuffer(&desc, &pBufferGen, NULL)))
			{
				Error(_T("Failed to create DirectSound Buffer"));
				SAFE_RELEASE(_pDs)
				return false;
			}
			HRESULT hr = pBufferGen->QueryInterface(IID_IDirectSoundBuffer8, (void**)&_pBuffer) ;
			// Release the temporary buffer.
			SAFE_RELEASE(pBufferGen)
			if( FAILED( hr ) )
			{
				Error(_T("Failed to obtain version 8 interface for Buffer"));
				_pBuffer = 0;
				_pDs->Release();
				_pDs = 0;
				return false;
			}

//			_pBuffer->Initialize(_pDs,&desc);
			for (unsigned int i=0; i<_capPorts.size();i++)
				CreateCapturePort(_capPorts[i]);

			_event.ResetEvent();
			_threadRun = true;
			_playing = false;
			DWORD dwThreadId;

			#define DIRECTSOUND_POLLING P
			#ifdef DIRECTSOUND_POLLING
				CreateThread( NULL, 0, PollerThread, this, 0, &dwThreadId );
			#else
				CreateThread( NULL, 0, NotifyThread, this, 0, &dwThreadId );
			#endif
			_running = true;
			return true;
		}

		bool DirectSound::Stop()
		{
//			CSingleLock lock(&_lock, TRUE);
			if(!_running) return true;
			_threadRun = false;
			CSingleLock event(&_event, TRUE);
			// Once we get here, the PollerThread should have stopped
			if(_playing)
			{
				_pBuffer->Stop();
				_playing = false;
			}
			SAFE_RELEASE(_pBuffer)
			SAFE_RELEASE(_pDs)
			for(unsigned int i=0; i<_capPorts.size(); i++)
			{
				if ( _capPorts[i]._pDs == NULL )
					continue;
				_capPorts[i]._pBuffer->Stop();
				SAFE_RELEASE(_capPorts[i]._pBuffer)
				SAFE_RELEASE(_capPorts[i]._pDs)
				universalis::os::aligned_memory_dealloc(_capPorts[i].pleft);
				universalis::os::aligned_memory_dealloc(_capPorts[i].pright);
			}
			_running = false;
			return true;
		}

		void DirectSound::GetPlaybackPorts(std::vector<std::string>&ports)
		{
			ports.resize(0);
			for (unsigned int i=0;i<_playEnums.size();i++) ports.push_back(_playEnums[i].portname);
		}
		void DirectSound::GetCapturePorts(std::vector<std::string>&ports)
		{
			ports.resize(0);
			for (unsigned int i=0;i<_capEnums.size();i++) ports.push_back(_capEnums[i].portname);
		}
		BOOL CALLBACK DirectSound::DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
		{
			std::vector<PortEnums>* ports=static_cast<std::vector<PortEnums>*>(lpContext);
			PortEnums port(lpGuid == NULL? const_cast<GUID*>(&DSDEVID_DefaultPlayback) : lpGuid , lpcstrDescription);
			ports->push_back(port);
			return TRUE;
		}
		BOOL CALLBACK DirectSound::DSCaptureEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
		{
			std::vector<PortEnums>* ports=static_cast<std::vector<PortEnums>*>(lpContext);
			PortEnums port(lpGuid == NULL? const_cast<GUID*>(&DSDEVID_DefaultCapture) : lpGuid,lpcstrDescription);
			ports->push_back(port);
			return TRUE;
		}
		bool DirectSound::AddCapturePort(int idx)
		{
			bool isplaying = _running;
			if ( idx >= _capEnums.size() ) return false;
			if ( idx < _portMapping.size() && _portMapping[idx] != -1) return true;

			if (isplaying) Stop();
			PortCapt port;
			port._pGuid = _capEnums[idx].guid;
			_capPorts.push_back(port);
			if ( _portMapping.size() <= idx) {
				int oldsize = _portMapping.size();
				_portMapping.resize(idx+1);
				for(int i=oldsize;i<_portMapping.size();i++) _portMapping[i]=-1;
			}
			_portMapping[idx]=(int)(_capPorts.size()-1);
			if (isplaying) return Start();

			return true;
		}
		bool DirectSound::RemoveCapturePort(int idx)
		{
			bool isplaying = _playing;
			int maxSize = 0;
			std::vector<PortCapt> newports;
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
		bool DirectSound::CreateCapturePort(PortCapt &port)
		{
			HRESULT hr;
			//not try to open a port twice
			if (port._pDs) return true;
			port._machinepos=0;

			// Create IDirectSoundCapture using the selected capture device
			if( FAILED( hr = DirectSoundCaptureCreate8( port._pGuid, &port._pDs, NULL ) ) )
			{
				Error(_T("Failed to create Capture DirectSound Device"));
				return false;
			}

			// Create the capture buffer
			WAVEFORMATPCMEX wf;
			PrepareWaveFormat(wf, (_channelMode == stereo) ? 2 : 1, _samplesPerSec, _sampleBits, _sampleValidBits);

			DSCBUFFERDESC dscbd;
			ZeroMemory( &dscbd, sizeof(DSCBUFFERDESC) );
			dscbd.dwSize        = sizeof(DSCBUFFERDESC);
			dscbd.dwBufferBytes = _dsBufferSize;
			dscbd.lpwfxFormat   = reinterpret_cast<LPWAVEFORMATEX>(&wf);

			LPDIRECTSOUNDCAPTUREBUFFER cb;
			if( FAILED( hr = port._pDs->CreateCaptureBuffer( &dscbd, &cb , NULL ) ) )
			{
				Error(_T("Failed to create Capture DirectSound Buffer"));
				SAFE_RELEASE(port._pDs)
				return false;
			}
			if( FAILED( hr = cb->QueryInterface(IID_IDirectSoundCaptureBuffer8, (void**)&port._pBuffer) ) )
			{
				Error(_T("Failed to create Interface for Capture DirectSound Buffer(s)"));
				SAFE_RELEASE(cb)
				SAFE_RELEASE(port._pDs)
				return false;
			}
			// 2* is a safety measure (Haven't been able to dig out why it crashes if it is exactly the size)
			universalis::os::aligned_memory_alloc(16, port.pleft, 2*_blockSizeBytes);
			universalis::os::aligned_memory_alloc(16, port.pright, 2*_blockSizeBytes);
			ZeroMemory(port.pleft, 2*_blockSizeBytes);
			ZeroMemory(port.pright, 2*_blockSizeBytes);
			return true;
		}

		void DirectSound::GetReadBuffers(int idx,float **pleft, float **pright,int numsamples)
		{
			if (!_running || idx >=_portMapping.size() || _portMapping[idx] == -1 
				|| _capPorts[_portMapping[idx]]._pDs == NULL)
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
/*
		HRESULT DirectSound::SetStopNotification(HANDLE hMyEvent, 
				LPDIRECTSOUNDBUFFER8 lpDsbSecondary)
		{
		  LPDIRECTSOUNDNOTIFY8 lpDsNotify; 
		  DSBPOSITIONNOTIFY PositionNotify;
		  HRESULT hr;

		  hr = lpDsbSecondary->QueryInterface(IID_IDirectSoundNotify8,
					 (LPVOID*)&lpDsNotify);
		  if (SUCCEEDED(hr)) 
		  { 
			PositionNotify.dwOffset = DSBPN_OFFSETSTOP;
			PositionNotify.hEventNotify = hMyEvent;
			hr = lpDsNotify->SetNotificationPositions(1, &PositionNotify);
			lpDsNotify->Release();
		  }
		  return hr;
		}
*/

		DWORD WINAPI DirectSound::NotifyThread(void* pDirectSound)
		{
/*
			universalis::os::thread_name thread_name("direct sound");
			universalis::cpu::exceptions::install_handler_in_thread();
			DirectSound * pThis = (DirectSound*) pDirectSound;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			// Ask MMCSS to temporarily boost the thread priority
			// to reduce glitches while the low-latency stream plays.
			HANDLE hTask = NULL;
			if(Is_Vista_or_Later()) 
			{
				DWORD taskIndex = 0;
				hTask = Global::pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
			}

			pThis->_lowMark = 0;
			pThis->m_readPosWraps = 0;
			pThis->_highMark = pThis->_blockSizeBytes;
			pThis->_pBuffer->SetCurrentPosition(0);
			//Prefill buffer:
			for(int i=0; i< pThis->_numBlocks;i++)
			{
				//Directsound playback buffer is started here.
				pThis->DoBlocks();
			}
			for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
			{
				if(pThis->_capPorts[i]._pDs == NULL)
					continue;
				RepositionMarks(pThis->_capPorts[i]._lowMark, 0);
				pThis->_capPorts[i]._pBuffer->Start(DSCBSTART_LOOPING);
			}
			while(pThis->_threadRun)
			{
				dwResult = ::WaitForMultipleObjects(m_nEvents, m_pEvents, FALSE, INFINITE);
				switch(dwResult)
				{
				case WAIT_OBJECT_0 + 0: //fallthrough
				case WAIT_OBJECT_0 + 1:	//fallthrough
				case WAIT_OBJECT_0 + 2:
					// First, do the capture buffers so that audio is available to wavein machines.
					for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
					{
						if(pThis->_capPorts[i]._pDs == NULL)
							continue;
						pThis->DoBlocksRecording(pThis->_capPorts[i]);
					}
					// Next, proceeed with the generation of audio
					pThis->DoBlocks();
					break;
				}
			}
			_event.SetEvent();
			if (hTask != NULL) { Global::pAvRevertMmThreadCharacteristics(hTask); }
*/
			return 0;
		}

		DWORD WINAPI DirectSound::PollerThread(void* pDirectSound)
		{
			universalis::os::thread_name thread_name("direct sound");
			universalis::cpu::exceptions::install_handler_in_thread();
			DirectSound * pThis = (DirectSound*) pDirectSound;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			// Ask MMCSS to temporarily boost the thread priority
			// to reduce glitches while the low-latency stream plays.
			HANDLE hTask = NULL;
			if(Is_Vista_or_Later()) 
			{
				DWORD taskIndex = 0;
				hTask = Global::pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
			}

			//Prefill buffer:
			pThis->_lowMark = 0;
			pThis->m_readPosWraps = 0;
			pThis->_highMark = pThis->_blockSizeBytes;
			pThis->_pBuffer->SetCurrentPosition(0);
			for(int i=0; i< pThis->_numBlocks;i++)
			{
				//Directsound playback buffer is started here.
				pThis->DoBlocks();
			}
			for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
			{
				if(pThis->_capPorts[i]._pDs == NULL)
					continue;
				pThis->RepositionMark(pThis->_capPorts[i]._lowMark, 0);
				pThis->_capPorts[i]._pBuffer->Start(DSCBSTART_LOOPING);
			}

			while(pThis->_threadRun)
			{
				int runs=0;

				while(pThis->WantsMoreBlocks())
				{
					// First, run the capture buffers so that audio is available to wavein machines.
					for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
					{
						if(pThis->_capPorts[i]._pDs == NULL)
							continue;
						pThis->DoBlocksRecording(pThis->_capPorts[i]);
					}
					// Next, proceeed with the generation of audio
					pThis->DoBlocks();
					if (++runs > pThis->_numBlocks)
						break;
				}
				::Sleep(1);
			}
			_event.SetEvent();
			if (hTask != NULL) { Global::pAvRevertMmThreadCharacteristics(hTask); }
			return 0;
		}

		bool DirectSound::WantsMoreBlocks()
		{
			// [_lowMark,_highMark) is the next buffer to be filled.
			// if (play) pos is still inside, we have to wait.
			int pos=0;
			HRESULT hr = _pBuffer->GetCurrentPosition((DWORD*)&pos, 0);
			if (FAILED(hr)) return false;
			if(_lowMark <= pos && pos < _highMark)	return false;
			return true;
		}

		void DirectSound::DoBlocksRecording(PortCapt& port)
		{
			//If directsound capture fails
			if(port._pBuffer == NULL) {
				return;
			}

			int* pBlock1 , *pBlock2;
			unsigned long blockSize1, blockSize2;
			HRESULT hr = port._pBuffer->Lock(port._lowMark, _blockSizeBytes, 
				(void**)&pBlock1, &blockSize1, (void**)&pBlock2, &blockSize2, 0);
			if (SUCCEEDED(hr))
			{ 
				// Put the audio in our float buffers.
				int numSamples = blockSize1 / GetSampleSizeBytes();
				if (_sampleValidBits == 32) {
					DeinterlaceFloat(reinterpret_cast<float*>(pBlock1), port.pleft,port.pright, numSamples);
				}
				else if (_sampleValidBits == 24) {
					DeQuantize32AndDeinterlace(pBlock1, port.pleft,port.pright, numSamples);
				}
				else {
					DeQuantize16AndDeinterlace(reinterpret_cast<short int*>(pBlock1), port.pleft,port.pright, numSamples);
				}
				port._lowMark += blockSize1;
				if (blockSize2 > 0)
				{
					if (_sampleValidBits == 32) {
						DeinterlaceFloat(reinterpret_cast<float*>(pBlock1), port.pleft,port.pright, numSamples);
					}
					else if (_sampleValidBits == 24) {
						DeQuantize32AndDeinterlace(pBlock1, port.pleft,port.pright, numSamples);
					}
					else {
						DeQuantize16AndDeinterlace(reinterpret_cast<short int*>(pBlock2),  port.pleft+numSamples,port.pright+numSamples, blockSize2 / GetSampleSizeBytes());
					}
					port._lowMark += blockSize2;
				}
				// Release the data back to DirectSound. 
				hr = port._pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);

				if(port._lowMark >= _dsBufferSize) port._lowMark = 0;
			}
			port._machinepos=0;
		}
		void DirectSound::DoBlocks()
		{
			int* pBlock1 , *pBlock2;
			unsigned long blockSize1, blockSize2;
			HRESULT hr = _pBuffer->Lock(_lowMark, _blockSizeBytes, 
				(void**)&pBlock1, &blockSize1, (void**)&pBlock2, &blockSize2, 0);
			if (DSERR_BUFFERLOST == hr) 
			{ 
				// If DSERR_BUFFERLOST is returned, restore and retry lock. 
				_playing = false;
				_pBuffer->Restore(); 
				if (_highMark < _dsBufferSize) _pBuffer->SetCurrentPosition(_highMark);
				else _pBuffer->SetCurrentPosition(0);

				hr = _pBuffer->Lock(_lowMark, _blockSizeBytes, 
					(void**)&pBlock1, &blockSize1, (void**)&pBlock2, &blockSize2, 0);
			}
			if (SUCCEEDED(hr))
			{
				// Generate audio and put it into the buffer
				int numSamples = blockSize1 / GetSampleSizeBytes();
				float *pFloatBlock = _pCallback(_callbackContext, numSamples);
				if(_sampleValidBits == 32) {
					dsp::MovMul(pFloatBlock, reinterpret_cast<float*>(pBlock1), numSamples*2, 1.f/32768.f);
				}
				else if(_sampleValidBits == 24) {
					Quantize24in32Bit(pFloatBlock, pBlock1, numSamples);
				}
				else if (_sampleValidBits == 16) {
					if(_dither) Quantize16WithDither(pFloatBlock, pBlock1, numSamples);
					else Quantize16(pFloatBlock, pBlock1, numSamples);
				}
				_lowMark += blockSize1;
				if (blockSize2 > 0)
				{
					numSamples = blockSize2 / GetSampleSizeBytes();
					float *pFloatBlock = _pCallback(_callbackContext, numSamples);
					if(_sampleValidBits == 32) {
						dsp::MovMul(pFloatBlock, reinterpret_cast<float*>(pBlock2), numSamples*2, 1.f/32768.f);
					}
					else if(_sampleValidBits == 24) {
						Quantize24in32Bit(pFloatBlock, pBlock2, numSamples);
					}
					else if (_sampleValidBits == 16) {
						if(_dither) Quantize16WithDither(pFloatBlock, pBlock2, numSamples);
						else Quantize16(pFloatBlock, pBlock2, numSamples);
					}
					_lowMark += blockSize2;
				}
				// Release the data back to DirectSound. 
				hr = _pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);
				if(_lowMark >= _dsBufferSize) {
					_lowMark = 0;
					m_readPosWraps++;
					if((std::uint64_t)m_readPosWraps * (std::uint64_t)_dsBufferSize >= 0x100000000LL)
					{
						m_readPosWraps = 0;
						CMidiInput::Instance()->ReSync();	// MIDI IMPLEMENTATION
					}
				}
				_highMark = _lowMark + _blockSizeBytes;
				if (SUCCEEDED(hr) && !_playing)
				{
					_playing = true;
					_pBuffer->SetCurrentPosition(_highMark);
					hr = _pBuffer->Play(0, 0, DSBPLAY_LOOPING);
					CMidiInput::Instance()->ReSync(); // MIDI IMPLEMENTATION
				}
			}
		}

		void DirectSound::ReadConfig()
		{
			// default configuration
			bool saveatend(false);
			device_guid = DSDEVID_DefaultPlayback;
			_dither = false;
			_sampleBits = 32;
			_sampleValidBits = 32;
			_channelMode = stereo;
			_samplesPerSec = 44100;
			_blockSizeBytes = 4096;
			_numBlocks = 4;
			_configured = true;

			// read from registry
			Registry reg;
			reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT);
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\direct-sound") != ERROR_SUCCESS) // settings in version 1.8
			{
				reg.CloseRootKey();
				reg.OpenRootKey(HKEY_CURRENT_USER,PSYCLE__PATH__REGISTRY__ROOT "--1.7"); // settings in version 1.7 alpha
				if(reg.OpenKey("configuration\\devices\\direct-sound") != ERROR_SUCCESS)
				{
					reg.CloseRootKey();
					reg.OpenRootKey(HKEY_CURRENT_USER,"Software\\AAS\\Psycle\\CurrentVersion");
					if(reg.OpenKey("DirectSound") != ERROR_SUCCESS)
					{
						reg.CloseRootKey();
						return;
					}
				}
				saveatend=true;
			}
			bool configured(true);
			configured &= ERROR_SUCCESS == reg.QueryValue("DeviceGuid", device_guid);
			configured &= ERROR_SUCCESS == reg.QueryValue("Dither", _dither);
			configured &= ERROR_SUCCESS == reg.QueryValue("BitDepth", _sampleValidBits);
			if (configured) { if(_sampleValidBits == 24) _sampleBits=32;
				else _sampleBits=_sampleValidBits;
			}
			configured &= ERROR_SUCCESS == reg.QueryValue("NumBuffers", _numBlocks);
			configured &= ERROR_SUCCESS == reg.QueryValue("BufferSize", _blockSizeBytes);
			configured &= ERROR_SUCCESS == reg.QueryValue("SamplesPerSec", _samplesPerSec);
			_configured = configured;
			if(device_guid == GUID()) {device_guid = DSDEVID_DefaultPlayback; }
			reg.CloseKey();
			reg.CloseRootKey();
			if(saveatend) WriteConfig();
		}

		void DirectSound::WriteConfig()
		{
			Registry reg;
			if(reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT) != ERROR_SUCCESS)
			{
				Error("Unable to write configuration to the registry");
				return;
			}
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\direct-sound") != ERROR_SUCCESS)
			{
				if(reg.CreateKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\direct-sound") != ERROR_SUCCESS)
				{
					Error("Unable to write configuration to the registry");
					return;
				}
			}
			reg.SetValue("DeviceGuid", device_guid);
			reg.SetValue("Dither", _dither);
			reg.SetValue("BitDepth", _sampleValidBits);
			reg.SetValue("SamplesPerSec", _samplesPerSec);
			reg.SetValue("BufferSize", _blockSizeBytes);
			reg.SetValue("NumBuffers", _numBlocks);
			reg.CloseKey();
			reg.CloseRootKey();
		}

		void DirectSound::Configure()
		{
			// 1. reads the config from persistent storage
			// 2. opens the gui to let the user edit the settings
			// 3. writes the config to persistent storage

			if(!_configured) ReadConfig();

			CDSoundConfig dlg;
			dlg.device_idx = GetIdxFromDevice(&device_guid);
			dlg.dither = _dither;
			dlg.bit_depth  = _sampleValidBits;
			dlg.sample_rate = _samplesPerSec;
			dlg.buffer_samples = GetBufferSamples();
			dlg.buffer_count = _numBlocks;
			dlg.directsound = this;

			if(dlg.DoModal() != IDOK) return;
			
			WAVEFORMATPCMEX wf;
			PrepareWaveFormat(wf, 2, dlg.sample_rate, (dlg.bit_depth==24)?32:dlg.bit_depth, dlg.bit_depth);
			bool supported = _playEnums[dlg.device_idx].IsFormatSupported(wf,false);
			if(!supported) {
				Error("The Format selected is not supported. Keeping the previous configuration");
				return;
			}

			this->device_guid = *_playEnums[dlg.device_idx].guid;
			_samplesPerSec = dlg.sample_rate;
			_sampleValidBits = dlg.bit_depth;
			if(_sampleValidBits == 24) _sampleBits = 32;
			else _sampleBits = _sampleValidBits;
			_dither = dlg.dither;
			_numBlocks = dlg.buffer_count;
			_blockSizeBytes = dlg.buffer_samples*GetSampleSizeBytes();

			_configured = true;
			WriteConfig();
		}

		std::uint32_t DirectSound::GetIdxFromDevice(GUID* device) {
			for(int i = 0; i < _playEnums.size() ; ++i)
			{
				if(device == _playEnums[i].guid)
				{
					return i;
				}
			}
			return 0;
		}

		std::uint32_t DirectSound::GetPlayPosInSamples()
		{
			//http://msdn.microsoft.com/en-us/library/ee418744%28v=VS.85%29.aspx
			//When a buffer is created, the play cursor is set to 0. As the buffer is played,
			//the cursor moves and always points to the next byte of data to be played.
			//When the buffer is stopped, the cursor remains where it is.
			if(!_threadRun) return 0;
			DWORD playPos;
			DWORD writePos;
			if(FAILED(_pBuffer->GetCurrentPosition(&playPos, &writePos)))
			{
				Error(_T("DirectSoundBuffer::GetCurrentPosition failed"));
				return 0;
			}
			nanoseconds pcns = cpu_time_clock();
			std::ostringstream asdf;
			asdf << "PlayPos: "<< playPos << "\twritepos: " << writePos << "\tlowMark: " << _lowMark << "\tm_readPosWraps: " << m_readPosWraps;

			if(playPos < writePos && _lowMark > writePos) {
				asdf << " (" << (playPos + m_readPosWraps*_dsBufferSize)/GetSampleSizeBytes() << ") at " << ((pcns.get_count()/1000000)&0xFFFF) << ")\n";
				TRACE(asdf.str().c_str());
				return (playPos + m_readPosWraps*_dsBufferSize)/GetSampleSizeBytes();
			}
			else {
				asdf << " (" << (playPos + (m_readPosWraps-1)*_dsBufferSize)/GetSampleSizeBytes() << ") at " << ((pcns.get_count()/1000000)&0xFFFF) << ")\n";
				TRACE(asdf.str().c_str());
				return (playPos + (m_readPosWraps-1)*_dsBufferSize)/GetSampleSizeBytes();
			}
		}

		std::uint32_t DirectSound::GetWritePosInSamples()
		{
			//http://msdn.microsoft.com/en-us/library/ee418744%28v=VS.85%29.aspx
			//The write cursor is the point after which it is safe to write data into the buffer.
			//The block between the play cursor and the write cursor is already committed to be played, and cannot be changed safely.
			//The write cursor moves with the play cursor, not with data written to the buffer. If you're streaming data,
			//you are responsible for maintaining your own pointer into the buffer to indicate where the next block of data should be written.
			if(!_threadRun) return 0;
			return (_lowMark + m_readPosWraps*_dsBufferSize)/GetSampleSizeBytes();
		}

		bool DirectSound::Enable(bool e)
		{
			return e ? Start() : Stop();
		}

		//Reposition the write block before the play cursor.
		void DirectSound::RepositionMark(int &low, int pos)
		{
			if (pos < _blockSizeBytes) {
				low = _dsBufferSize-_blockSizeBytes;
			}
			else {
				low = (pos-_blockSizeBytes)/_blockSizeBytes*_blockSizeBytes;
			}
		}

		bool DirectSound::PortEnums::IsFormatSupported(WAVEFORMATEXTENSIBLE& pwfx, bool isInput)
		{
			///\todo: Implement
			return true;
		}
	}
}
