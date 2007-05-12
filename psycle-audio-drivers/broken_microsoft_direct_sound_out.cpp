// copyright 2004-2007 psycledelics http://psycle.sourceforge.net
/*************************************************************************
*  This program is covered by the GNU General Public License:            *
*  http://gnu.org/licenses/gpl.html                                      *
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
*  This program is distributed in the hope that it will be useful,       *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
*  See the GNU General Public License for more details.                  *
*                                                                        *
*  You should have received a copy of the GNU General Public License     *
*  along with this program; if not, write to the                         *
*  Free Software Foundation, Inc.,                                       *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
**************************************************************************/
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_OUT_AVAILABLE
#include "microsoft_direct_sound_out.h"
namespace psycle
{
	namespace audio_drivers
	{
		AudioDriverInfo MicrosoftDirectSoundOut::info( ) const
		{
			return AudioDriverInfo("microsoft-direct-sound", "Microsoft DirectSound"," Microsoft DirectSound output driver",true);
		}

		MicrosoftDirectSoundOut * MicrosoftDirectSoundOut::clone( ) const
		{
			return new MicrosoftDirectSoundOut(*this);
		}


		MicrosoftDirectSoundOut::MicrosoftDirectSoundOut() :
		device_guid(), // DSDEVID_DefaultPlayback <-- undersolved external symbol
			_initialized(0),
			_exclusive(false),
			//			_configured(0),
			_running(false),
			_playing(false),
			//			_timerActive(0),
			_pDs(0),
			_pBuffer(0),
			_pCallback(0)
		{

		}
		MicrosoftDirectSoundOut::~MicrosoftDirectSoundOut()
		{
			Stop();
		}

		void MicrosoftDirectSoundOut::Initialize(AUDIODRIVERWORKFN pCallback, void * context )
		{
			_callbackContext = context;
			_pCallback = pCallback;
			_running = false;
			_playing = false;
			//			_hwnd = hwnd;
			//			ReadConfig();
			_initialized = true;
		}

		void MicrosoftDirectSoundOut::configure()
		{
		}

		bool MicrosoftDirectSoundOut::Enable(bool e)
		{
			return e ? Start() : Stop();
		}

		bool MicrosoftDirectSoundOut::Start()
		{
			//			CSingleLock lock(&_lock, true);
			if(_running) return true;
			if(!_pCallback) return false;
			if(FAILED(::DirectSoundCreate(device_guid != GUID() ? &device_guid : 0, &_pDs, 0)))
			{
				Error("Failed to create DirectSound object");
				return false;
			}
			HWND hwnd = ::GetWindow(NULL, 0);
			if (!hwnd)
			{
				hwnd = ::GetForegroundWindow();
			}
			if (!hwnd)
			{
				hwnd = ::GetDesktopWindow();
			}
			if(_exclusive)
			{
				if(FAILED(_pDs->SetCooperativeLevel(hwnd, DSSCL_WRITEPRIMARY)))
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
				if(FAILED(_pDs->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
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
			format.wBitsPerSample = settings().bitDepth();//_bitDepth;
			format.nSamplesPerSec = settings().samplesPerSec();
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
				//				WriteConfig();
			}
			_pBuffer->Initialize(_pDs,&desc);

			_lowMark = 0;
			_highMark = _bufferSize;
			if(_highMark >= _dsBufferSize) _highMark = _dsBufferSize - 1;
			_currentOffset = 0;
			_buffersToDo = _numBuffers;
			//			_event.ResetEvent();
			//			_timerActive = true;
			_playing = true;

			DWORD dwThreadId;
			_hThread = CreateThread( NULL, 0, audioOutThread, this, 0, &dwThreadId );
			_running = true;
			return true;
		}

		bool MicrosoftDirectSoundOut::Stop()
		{
			//			CSingleLock lock(&_lock, true);
			if(!_running) return true;
			_playing = false;
			while (_running)
			{
				Sleep(1);
			}
			//			CSingleLock event(&_event, true);
			// Once we get here, the PollerThread should have stopped
			_pBuffer->Stop();
			_pBuffer->Release();
			_pBuffer = 0;
			_pDs->Release();
			_pDs = 0;
			TerminateThread( _hThread, 0 );
			return true;
		}

		DWORD WINAPI MicrosoftDirectSoundOut::audioOutThread(void *pDirectSound)
		{
			//			universalis::processor::exception::new_thread("direct sound");
			MicrosoftDirectSoundOut * pThis = reinterpret_cast<MicrosoftDirectSoundOut*>( pDirectSound ) ;
			//			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			pThis->_running = true;
			while(pThis->_playing)
			{
				pThis->DoBlocks();
				Sleep(1);
			}
			//			_event.SetEvent();
			//			::_endthread();
			pThis->_running = false;
			::ExitThread(0);
			return 0;
		}

		void MicrosoftDirectSoundOut::DoBlocks()
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
						int blockSize = blockSize1 / settings().sampleSize();
						int* pBlock = pBlock1;
						while(blockSize > 0)
						{
							int n = blockSize;
							float *pFloatBlock = _pCallback(_callbackContext, n);
							//						if(_dither) QuantizeWithDither(pFloatBlock, pBlock, n); else Quantize(pFloatBlock, pBlock, n);
							while (n-- > 0) {
								*pBlock++ = static_cast<short int>( *pFloatBlock++ );
								*pBlock++ = static_cast<short int>( *pFloatBlock++ );
							}
							pBlock += blockSize;
							blockSize -= blockSize;
						}
						blockSize = blockSize2 / settings().sampleSize();
						pBlock = pBlock2;
						while(blockSize > 0)
						{
							int n = blockSize;
							float *pFloatBlock = _pCallback(_callbackContext, n);
							//						if(_dither) QuantizeWithDither(pFloatBlock, pBlock, n); else Quantize(pFloatBlock, pBlock, n);
							while (n-- > 0) {
								*pBlock++ = static_cast<short int>( *pFloatBlock++ );
								*pBlock++ = static_cast<short int>( *pFloatBlock++ );
							}
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
		}

		void MicrosoftDirectSoundOut::Error(const char msg[])
		{
			std::cout << msg << std::endl;
		}

		#if 0
		void MicrosoftDirectSoundOut::ReadConfig()
		{
		// default configuration
		bool saveatend(false);
		device_guid = GUID(); // DSDEVID_DefaultPlayback <-- unresolved external symbol
		_exclusive = false;
		_dither = false;
		_bitDepth  16;
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

		void MicrosoftDirectSoundOut::WriteConfig()
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
		#endif

		int MicrosoftDirectSoundOut::GetPlayPos()
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

		int MicrosoftDirectSoundOut::GetWritePos()
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
	}
}
#endif
