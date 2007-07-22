///\file
///\implementation psycle::host::DirectSound.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/audiodrivers/DirectSound.hpp>
//#include <psycle/host/resources/resources.hpp>
//\todo:
#include <psycle/host/audiodrivers/DSoundConfig.hpp>
#include <psycle/engine/registry.hpp>
//\todo:
#include <psycle/host/uiconfiguration.hpp>
#include <universalis/processor/exception.hpp>
#include <process.h>
namespace psycle
{
	namespace host
	{
		AudioDriverInfo DirectSound::_info = { "DirectSound Output" };
		AudioDriverEvent DirectSound::_event;

		void DirectSound::Error(const char msg[])
		{
			MessageBox(0, msg, "DirectSound Output driver", MB_OK | MB_ICONERROR);
		}

		DirectSound::DirectSound()
		:
			device_guid(), // DSDEVID_DefaultPlayback <-- undersolved external symbol
			_initialized(),
			_configured(),
			_running(),
			_playing(),
			_timerActive(),
			_pDs(),
			_pBuffer(),
			_pCallback()
		{
		}

		void DirectSound::Initialize(HWND hwnd, WorkFunction pCallback, void * context)
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
			CSingleLock lock(&_lock, true);
			if(_running) return true;
			if(!_pCallback) return false;
			if(FAILED(::DirectSoundCreate(device_guid != GUID() ? &device_guid : 0, &_pDs, 0)))
			{
				Error("Failed to create DirectSound object");
				return false;
			}
			if(_exclusive)
			{
				if(FAILED(_pDs->SetCooperativeLevel(_hwnd, DSSCL_WRITEPRIMARY)))
				{
					// Don't report this, since we may have simply have lost focus
					// Error("Failed to set DirectSound cooperative level");
					_pDs->Release();
					_pDs = 0;
					return false;
				}
			}
			else
			{
				if(FAILED(_pDs->SetCooperativeLevel(_hwnd, DSSCL_PRIORITY)))
				{
					Error("Failed to set DirectSound cooperative level");
					_pDs->Release();
					_pDs = 0;
					return false;
				}
			}

			_dsBufferSize = _exclusive ? 0 : _bufferSize*_numBuffers;

			DSBCAPS caps;
			DSBUFFERDESC desc;
			WAVEFORMATEX format;

			format.wFormatTag = WAVE_FORMAT_PCM;
			format.nChannels = 2;
			format.wBitsPerSample = 16;//_bitDepth;
			format.nSamplesPerSec = _samplesPerSec;
			format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
			format.cbSize = 0;

			desc.dwSize = sizeof desc;
			desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
			desc.dwFlags |= _exclusive ? DSBCAPS_PRIMARYBUFFER : DSBCAPS_GLOBALFOCUS;
			desc.dwBufferBytes = _dsBufferSize; 
			desc.dwReserved = 0;
			desc.lpwfxFormat = _exclusive ? 0 : &format;
			desc.guid3DAlgorithm = GUID_NULL;
			
			if(FAILED(_pDs->CreateSoundBuffer(&desc, &_pBuffer, 0)))
			{
				Error("Failed to create DirectSound Buffer(s)");
				_pDs->Release();
				_pDs = 0;
				return false;
			}

			if(_exclusive)
			{
				_pBuffer->Stop();
				if(FAILED(_pBuffer->SetFormat(&format)))
				{
					Error("Failed to set DirectSound Buffer format");
					_pBuffer->Release();
					_pBuffer = 0;
					_pDs->Release();
					_pDs = 0;
					return false;
				}
				caps.dwSize = sizeof(caps);
				if(FAILED(_pBuffer->GetCaps(&caps)))
				{
					Error("Failed to get DirectSound Buffer capabilities");
					_pBuffer->Release();
					_pBuffer = 0;
					_pDs->Release();
					_pDs = 0;
					return false;
				}
				_dsBufferSize = caps.dwBufferBytes;
				WriteConfig();
			}
			_pBuffer->Initialize(_pDs,&desc);

			_lowMark = 0;
			_highMark = _bufferSize;
			if(_highMark >= _dsBufferSize) _highMark = _dsBufferSize - 1;
			_currentOffset = 0;
			_buffersToDo = _numBuffers;
			_event.ResetEvent();
			_timerActive = true;
			::_beginthread(PollerThread, 0, this);
			_running = true;
			return true;
		}

		void DirectSound::PollerThread(void * pDirectSound)
		{
			universalis::processor::exception::new_thread("direct sound");
			DirectSound * pThis = (DirectSound*) pDirectSound;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			while(pThis->_timerActive)
			{
				pThis->DoBlocks();
				::Sleep(1);
			}
			_event.SetEvent();
			::_endthread();
		}

		bool DirectSound::Stop()
		{
			CSingleLock lock(&_lock, true);
			if(!_running) return true;
			_running = false;
			_timerActive = false;
			CSingleLock event(&_event, true);
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
			return true;
		}

		void DirectSound::DoBlocks()
		{
			int pos;
			HRESULT hr;
			bool playing = _playing;
			while(true)
			{
				while(true)
				{
					hr = _pBuffer->GetCurrentPosition((DWORD*)&pos, 0);
					if(FAILED(hr))
					{
						if(hr == DSERR_BUFFERLOST)
						{
							playing = false;
							if(FAILED(_pBuffer->Restore()))
							{
								// Don't inform about this error, because it will
								// appear each time the Psycle window loses focus in exclusive mode
								return;
							}
							continue;
						}
						else
						{
							Error("DirectSoundBuffer::GetCurrentPosition failed");
							return;
						}
					}
					break;
				}
				if(_highMark < _lowMark)
				{
					if((pos > _lowMark) || (pos < _highMark)) return;
				}
				else if((pos > _lowMark) && (pos < _highMark)) return;
				int* pBlock1;
				int blockSize1;
				int* pBlock2;
				int blockSize2;
				int currentOffset = _currentOffset;
				while (_buffersToDo != 0)
				{
					while(true)
					{
						hr = _pBuffer->Lock((DWORD)currentOffset, (DWORD)_bufferSize,
											(void**)&pBlock1, (DWORD*)&blockSize1,
											(void**)&pBlock2, (DWORD*)&blockSize2,
											0);
						if(FAILED(hr))
						{
							if(hr == DSERR_BUFFERLOST)
							{
								playing = false;
								if(FAILED(_pBuffer->Restore())) return;
								continue;
							}
							else
							{
								Error("Failed to lock DirectSoundBuffer");
								return;
							}
						}
						break;
					}
					int blockSize = blockSize1 / GetSampleSize();
					int* pBlock = pBlock1;
					while(blockSize > 0)
					{
						int n = blockSize;
						float *pFloatBlock = _pCallback(_callbackContext, n);
						if(_dither) QuantizeWithDither(pFloatBlock, pBlock, n); else Quantize(pFloatBlock, pBlock, n);
						pBlock += n;
						blockSize -= n;
					}
					blockSize = blockSize2 / GetSampleSize();
					pBlock = pBlock2;
					while(blockSize > 0)
					{
						int n = blockSize;
						float *pFloatBlock = _pCallback(_callbackContext, n);
						if(_dither) QuantizeWithDither(pFloatBlock, pBlock, n); else Quantize(pFloatBlock, pBlock, n);
						pBlock += n;
						blockSize -= n;
					}
			
					_pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);
					_currentOffset += _bufferSize;
					if(_currentOffset >= _dsBufferSize) _currentOffset -= _dsBufferSize;
					_lowMark += _bufferSize;
					if(_lowMark >= _dsBufferSize) _lowMark -= _dsBufferSize;
					_highMark += _bufferSize;
					if(_highMark >= _dsBufferSize) _highMark -= _dsBufferSize;
					--_buffersToDo;
				} // while (_buffersToDo != 0)
				_buffersToDo = 1;
				if(!playing)
				{
					_playing = true;
					_pBuffer->Play(0, 0, DSBPLAY_LOOPING);
				}
			} // while (true)
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
				Error("DirectSoundBuffer::GetCurrentPosition failed");
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
				Error("DirectSoundBuffer::GetCurrentPosition failed");
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
