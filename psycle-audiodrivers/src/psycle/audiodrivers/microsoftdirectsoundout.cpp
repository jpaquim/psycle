// -*- mode:c++; indent-tabs-mode:t -*-
/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
#include "microsoftdirectsoundout.h"
#include <cstdint>
#pragma comment(lib, "dsound")
namespace psy
{
	namespace core
	{
		int const SHORT_MIN = -32768;
		int const SHORT_MAX =  32767;      

		AudioDriverInfo MsDirectSound::info( ) const
		{
			return AudioDriverInfo("dsound","Microsoft DirectSound Driver","Microsoft output driver",true);
		}      

		void MsDirectSound::Error(const WCHAR msg[])
		{
			MessageBoxW(0, msg, L"DirectSound Output driver", MB_OK | MB_ICONERROR);
		}

		MsDirectSound::MsDirectSound()
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

		MsDirectSound * MsDirectSound::clone( ) const
		{
			return new MsDirectSound(*this);
		}

		void MsDirectSound::Initialize( AUDIODRIVERWORKFN pCallback, void * context )
		{
			_callbackContext = context;
			_pCallback = pCallback;
			_running = false;
			//_hwnd = NApp::mainWindow()->win();
			ReadConfig();
			_initialized = true;
		}

		void MsDirectSound::Reset()
		{
			Stop();
		}

		MsDirectSound::~MsDirectSound()
		{
			Reset();
		}

		bool MsDirectSound::Start()
		{
			//CSingleLock lock(&_lock, true);
			if(_running) return true;
			if(!_pCallback) return false;
			if(FAILED(::DirectSoundCreate(device_guid != GUID() ? &device_guid : 0, &_pDs, 0)))
			{
				Error(L"Failed to create DirectSound object");
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
					// Error(L"Failed to set DirectSound cooperative level");
					_pDs->Release();
					_pDs = 0;
					return false;
				}
			}
			else
			{
				if(FAILED(_pDs->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
				{
					Error(L"Failed to set DirectSound cooperative level");
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
			format.nSamplesPerSec = settings().samplesPerSec();// _samplesPerSec;
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
				Error(L"Failed to create DirectSound Buffer(s)");
				_pDs->Release();
				_pDs = 0;
				return false;
			}

			if(_exclusive)
			{
				_pBuffer->Stop();
				if(FAILED(_pBuffer->SetFormat(&format)))
				{
					Error(L"Failed to set DirectSound Buffer format");
					_pBuffer->Release();
					_pBuffer = 0;
					_pDs->Release();
					_pDs = 0;
					return false;
				}
				caps.dwSize = sizeof(caps);
				if(FAILED(_pBuffer->GetCaps(&caps)))
				{
					Error(L"Failed to get DirectSound Buffer capabilities");
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
			//_event.ResetEvent();
			_timerActive = true;
			DWORD dwThreadId;
			CreateThread( NULL, 0, PollerThread, this, 0, &dwThreadId );
			_running = true;
			return true;
		}

		DWORD WINAPI MsDirectSound::PollerThread(void * pDirectSound)
		{
			MsDirectSound * pThis = reinterpret_cast<MsDirectSound*>( pDirectSound );
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			while(pThis->_timerActive)
			{
				pThis->DoBlocks();
				::Sleep(1);
			}
			//_event.SetEvent();
			//::_endthread();
			return 0;
		}

		bool MsDirectSound::Stop()
		{
			//CSingleLock lock(&_lock, true);
			if(!_running) return true;
			_running = false;
			_timerActive = false;
			//CSingleLock event(&_event, true);
			// Once we get here, the PollerThread should have stopped
			///\todo: some threadlocking mechanism. For now adding this sleeps
		#if defined _WIN32
			Sleep(1000);
		#else
			sleep(1);
		#endif

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

		void MsDirectSound::DoBlocks()
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
							Error(L"DirectSoundBuffer::GetCurrentPosition failed");
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
								Error(L"Failed to lock DirectSoundBuffer");
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
						// if(_dither) QuantizeWithDither(pFloatBlock, pBlock, n); else 
						quantize(pFloatBlock, pBlock, n);
						pBlock += n;
						blockSize -= n;
					}
					blockSize = blockSize2 / settings().sampleSize();
					pBlock = pBlock2;
					while(blockSize > 0)
					{
						int n = blockSize;
						float *pFloatBlock = _pCallback(_callbackContext, n);
						// if(_dither) QuantizeWithDither(pFloatBlock, pBlock, n); else 
						quantize(pFloatBlock, pBlock, n);
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
					hr = _pBuffer->Play(0, 0, DSBPLAY_LOOPING);
				}

			} // while (true)
		}

		void MsDirectSound::ReadConfig()
		{
			// default configuration
			bool saveatend(false);
			device_guid = GUID(); // DSDEVID_DefaultPlayback <-- unresolved external symbol
			_exclusive = false;
			_dither = false;
			//_bitDepth = 16;
			//_channelmode = 3;
			//_samplesPerSec = 44100;
			_bufferSize = 4096;
			_numBuffers = 4;
			_configured = true;


		}

		void MsDirectSound::WriteConfig()
		{

		}

		void MsDirectSound::Configure()
		{
			// 1. reads the config from persistent storage
			// 2. opens the gui to let the user edit the settings
			// 3. writes the config to persistent storage

			ReadConfig();


		}

		int MsDirectSound::GetPlayPos()
		{
			if(!_running) return 0;
			int playPos;
			if(FAILED(_pBuffer->GetCurrentPosition((DWORD*)&playPos, 0)))
			{
				Error(L"DirectSoundBuffer::GetCurrentPosition failed");
				return 0;
			}
			return playPos;
		}

		int MsDirectSound::GetWritePos()
		{
			if(!_running) return 0;
			int writePos;
			if(FAILED(_pBuffer->GetCurrentPosition(0, (DWORD*)&writePos)))
			{
				Error(L"DirectSoundBuffer::GetCurrentPosition failed");
				return 0;
			}
			return writePos;
		}

		bool MsDirectSound::Enable(bool e)
		{
			return e ? Start() : Stop();
		}

		void MsDirectSound::quantize(float *pin, int *piout, int c)
		{
			do
			{
				int r = static_cast<int>( (pin[1]) );

				if (r < SHORT_MIN)
				{
					r = SHORT_MIN;
				}
				else if (r > SHORT_MAX)
				{
					r = SHORT_MAX;
				}

				int l = static_cast<int>( (pin[0]) );

				if (l < SHORT_MIN)
				{
					l = SHORT_MIN;
				}
				else if (l > SHORT_MAX)
				{
					l = SHORT_MAX;
				}

				*piout++ = (r << 16) | static_cast<std::uint16_t>(l);
				pin += 2;
			}
			while(--c);
		}
	}
}
#endif
