///\file
///\implementation psycle::host::DirectSound.
#include <project.private.hpp>
#include "DirectSound.hpp"
//#include "resources/resources.hpp"
#include "DSoundConfig.hpp"
#include "Registry.hpp"
#include "Configuration.hpp"
#include <operating_system/exception.hpp>
#include <process.h>
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
			device_guid(GUID_NULL), // DSDEVID_DefaultPlayback <-- undersolved external symbol
			_initialized(false),
			_configured(false),
			_running(false),
			_playing(false),
			_threadRun(false),
			_pDs(0),
			_pBuffer(0),
			_pCallback(0)
		{
		}

		void DirectSound::Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void * context)
		{
			_callbackContext = context;
			_pCallback = pCallback;
			_running = false;
			_hwnd = hwnd;
			ReadConfig();
			_initialized = true;
		}

		void DirectSound::Reset()
		{
			Stop();
		}

		DirectSound::~DirectSound() throw()
		{
			Reset();
		}

		bool DirectSound::Start()
		{
//			CSingleLock lock(&_lock, TRUE);
			if(_running) return true;
			if(!_pCallback) return false;
			if( FAILED( ::CoInitialize(NULL) ) )
			{
				Error(_T("(DirectSound) Failed to initialize COM"));
				return false;
			}
			if(FAILED(::DirectSoundCreate8(device_guid != GUID() ? &device_guid : 0, &_pDs, 0)))
			{
				Error(_T("Failed to create DirectSound object"));
				return false;
			}
			if(_exclusive)
			{
				if(FAILED(_pDs->SetCooperativeLevel(_hwnd, DSSCL_WRITEPRIMARY)))
				{
					// Don't report this, since we may have simply have lost focus
					// Error(_T("Failed to set DirectSound cooperative level");
					_pDs->Release();
					_pDs = 0;
					return false;
				}
			}
			else
			{
				if(FAILED(_pDs->SetCooperativeLevel(_hwnd, DSSCL_PRIORITY)))
				{
					Error(_T("Failed to set DirectSound cooperative level"));
					_pDs->Release();
					_pDs = 0;
					return false;
				}
			}

			DSBCAPS caps;
			DSBUFFERDESC desc;
			WAVEFORMATEX format;
			// Set up wave format structure. 
			memset(&format, 0, sizeof(WAVEFORMATEX)); 
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.nChannels = 2;
			format.wBitsPerSample = 16;//_bitDepth;
			format.nSamplesPerSec = _samplesPerSec;
			format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
			format.cbSize = 0;

			_dsBufferSize = _exclusive ? 0 : _bufferSize*_numBuffers;
			// Set up DSBUFFERDESC structure. 
			memset(&desc, 0, sizeof(DSBUFFERDESC));
			desc.dwSize = sizeof(DSBUFFERDESC); 
			desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
			desc.dwFlags |= _exclusive ? DSBCAPS_PRIMARYBUFFER : DSBCAPS_GLOBALFOCUS;
			desc.dwBufferBytes = _dsBufferSize; 
			desc.dwReserved = 0;
			desc.lpwfxFormat = _exclusive ? 0 : &format;
			desc.guid3DAlgorithm = GUID_NULL;
			if(FAILED(_pDs->CreateSoundBuffer(&desc, reinterpret_cast<LPDIRECTSOUNDBUFFER*>(&_pBuffer), 0)))
			{
				Error(_T("Failed to create DirectSound Buffer(s)"));
				_pDs->Release();
				_pDs = 0;
				return false;
			}

			if(_exclusive)
			{
//				_pBuffer->Stop();
				if(FAILED(_pBuffer->SetFormat(&format)))
				{
					Error(_T("Failed to set DirectSound Buffer format"));
					_pBuffer->Release();
					_pBuffer = 0;
					_pDs->Release();
					_pDs = 0;
					return false;
				}
				caps.dwSize = sizeof(caps);
				if(FAILED(_pBuffer->GetCaps(&caps)))
				{
					Error(_T("Failed to get DirectSound Buffer capabilities"));
					_pBuffer->Release();
					_pBuffer = 0;
					_pDs->Release();
					_pDs = 0;
					return false;
				}
				_dsBufferSize = caps.dwBufferBytes;
				//WriteConfig();

				_runningBufSize = _dsBufferSize*0.5f;
				_buffersToDo = 1;
			}
			else
			{
				_runningBufSize = _bufferSize;
				_buffersToDo = _numBuffers;
			}
			_lowMark = 0;
			_highMark = _runningBufSize;

			_pBuffer->Initialize(_pDs,&desc);

			_event.ResetEvent();
			_threadRun = true;
			_playing = false;
			DWORD dwThreadId;
			CreateThread( NULL, 0, PollerThread, this, 0, &dwThreadId );
			_running = true;
			return true;
		}

		DWORD WINAPI DirectSound::PollerThread(void* pDirectSound)
		{
			operating_system::exceptions::translated::new_thread("direct sound");
			DirectSound * pThis = (DirectSound*) pDirectSound;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			SetThreadAffinityMask(GetCurrentThread(), 1);
			//Prefill buffer:
			for(int i=0; i< pThis->_buffersToDo;i++)
				pThis->DoBlocks();

			while(pThis->_threadRun)
			{
				int runs=0;
				while(pThis->WantsMoreBlocks())
				{
					pThis->DoBlocks();
					if (++runs > pThis->_numBuffers)
						break;
				}
				::Sleep(10);
			}
			_event.SetEvent();
//			::_endthread();
			return 0;
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
			_pBuffer->Release();
			_pBuffer = 0;
			_pDs->Release();
			_pDs = 0;
			_running = false;
			// Release COM
			CoUninitialize();

			return true;
		}

		bool DirectSound::WantsMoreBlocks()
		{
			// [_lowMark,_highMark] is the next buffer to be filled.
			// if pos is still inside, we have to wait.
			int pos=0;
			HRESULT hr = _pBuffer->GetCurrentPosition((DWORD*)&pos, 0);
			if(hr == DSERR_BUFFERLOST)
			{
				_playing = false;
				if(FAILED(_pBuffer->Restore()))	return false;
				hr = _pBuffer->GetCurrentPosition((DWORD*)&pos, 0);
				if (FAILED(hr)) return false;
				else return true;
			}
			if (FAILED(hr)) return false;
			if(_highMark < _lowMark)
			{
				if((pos >= _lowMark) || (pos < _highMark)) return false;
			}
			else if((pos >= _lowMark) && (pos < _highMark))	return false;
			return true;
		}

		void DirectSound::DoBlocks()
		{
			int* pBlock1 , *pBlock2;
			unsigned long blockSize1, blockSize2;
			// Obtain write pointer. 
			HRESULT hr = _pBuffer->Lock(_lowMark, _runningBufSize, 
				(void**)&pBlock1, &blockSize1, 
				(void**)&pBlock2, &blockSize2, 0);
			if (DSERR_BUFFERLOST == hr) 
			{ 
				// If DSERR_BUFFERLOST is returned, restore and retry lock. 
				_pBuffer->Restore(); 
				_playing = false;
				hr = _pBuffer->Lock(_lowMark, _runningBufSize, 
					(void**)&pBlock1, &blockSize1, 
					(void**)&pBlock2, &blockSize2, 0);
			} 
			if (SUCCEEDED(hr))
			{ 
				// Generate audio and put it into the buffer
				int numSamples = blockSize1 / GetSampleSize();
				float *pFloatBlock = _pCallback(_callbackContext, numSamples);
				if(_dither) QuantizeWithDither(pFloatBlock, pBlock1, numSamples);
				else Quantize(pFloatBlock, pBlock1, numSamples);
				_lowMark += blockSize1;
				if (blockSize2 > 0)
				{
					numSamples = blockSize2 / GetSampleSize();
					float *pFloatBlock = _pCallback(_callbackContext, numSamples);
					if(_dither) QuantizeWithDither(pFloatBlock, pBlock2, numSamples);
					else Quantize(pFloatBlock, pBlock2, numSamples);
					_lowMark += blockSize2; // Because it wrapped around. 
				}
				// Release the data back to DirectSound. 
				hr = _pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);
				if (SUCCEEDED(hr) && !_playing)
				{
					_playing = true;
					hr = _pBuffer->Play(0, 0, DSBPLAY_LOOPING);
				}
				_highMark = _lowMark + _runningBufSize;
				if(_highMark > _dsBufferSize)
				{
					_highMark -= _dsBufferSize;
					if ( _lowMark >= _dsBufferSize ) _lowMark -= _dsBufferSize;
				}
			}
		}

		void DirectSound::ReadConfig()
		{
			// default configuration
			bool saveatend(false);
			device_guid = GUID(); // DSDEVID_DefaultPlayback <-- unresolved external symbol
			_exclusive = false;
			_dither = false;
			_bitDepth = 16;
			_channelmode = 3;
			_samplesPerSec = 44100;
			_bufferSize = 4096;
			_numBuffers = 4;
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
			configured &= ERROR_SUCCESS == reg.QueryValue("Exclusive", _exclusive);
			configured &= ERROR_SUCCESS == reg.QueryValue("Dither", _dither);
			//configured &= ERROR_SUCCESS == reg.QueryValue("BitDepth", _bitDepth);
			configured &= ERROR_SUCCESS == reg.QueryValue("NumBuffers", _numBuffers);
			configured &= ERROR_SUCCESS == reg.QueryValue("BufferSize", _bufferSize);
			configured &= ERROR_SUCCESS == reg.QueryValue("SamplesPerSec", _samplesPerSec);
			_configured = configured;

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
			reg.SetValue("Exclusive", _exclusive);
			reg.SetValue("Dither", _dither);
			//reg.SetValue("BitDepth", _bitDepth);
			reg.SetValue("SamplesPerSec", _samplesPerSec);
			reg.SetValue("BufferSize", _bufferSize);
			reg.SetValue("NumBuffers", _numBuffers);
			reg.CloseKey();
			reg.CloseRootKey();
		}

		void DirectSound::Configure()
		{
			// 1. reads the config from persistent storage
			// 2. opens the gui to let the user edit the settings
			// 3. writes the config to persistent storage

			ReadConfig();

			CDSoundConfig dlg;
			dlg.device_guid = device_guid;
			dlg.exclusive = _exclusive;
			dlg.dither = _dither;
			dlg.sample_rate = _samplesPerSec;
			dlg.buffer_size = _bufferSize;
			dlg.buffer_count = _numBuffers;

			if(dlg.DoModal() != IDOK) return;

			_configured = true;
			
			// save the settings to be able to rollback if it doesn't work
			GUID device_guid = this->device_guid;
			bool exclusive = _exclusive;
			bool dither = _dither;
			int samplesPerSec = _samplesPerSec;
			int bufferSize = _bufferSize;
			int numBuffers = _numBuffers;
			
			if(_initialized) Stop();

			this->device_guid = dlg.device_guid;
			_exclusive = dlg.exclusive;
			_dither = dlg.dither;
			_samplesPerSec = dlg.sample_rate;
			_bufferSize = dlg.buffer_size;
			_numBuffers = dlg.buffer_count;

			if(_initialized)
			{
				if(Start()) WriteConfig();
				else
				{
					// rollback
					this->device_guid = device_guid;
					_exclusive = exclusive;
					_dither = dither;
					_samplesPerSec = samplesPerSec;
					_bufferSize = bufferSize;
					_numBuffers = numBuffers;
					Start();
				}
			}
			else WriteConfig();
		}

		int DirectSound::GetPlayPos()
		{
			if(!_running) return 0;
			int playPos;
			if(FAILED(_pBuffer->GetCurrentPosition((DWORD*)&playPos, 0)))
			{
				Error(_T("DirectSoundBuffer::GetCurrentPosition failed"));
				return 0;
			}
			return playPos;
		}

		int DirectSound::GetWritePos()
		{
			if(!_running) return 0;
			int writePos;
			if(FAILED(_pBuffer->GetCurrentPosition(0, (DWORD*)&writePos)))
			{
				Error(_T("DirectSoundBuffer::GetCurrentPosition failed"));
				return 0;
			}
			return writePos;
		}

		bool DirectSound::Enable(bool e)
		{
			return e ? Start() : Stop();
		}
	}
}
