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
#include <psycle/helpers/math/fast_unspecified_round_to_integer.hpp>
#include <universalis/cpu/exception.hpp>
#include <universalis/os/aligned_memory_alloc.hpp>
#include <universalis/os/thread_name.hpp>
#include <cstdint>

///\todo: needed?
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "winmm")
#endif

#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "dsound")
#endif


namespace psy { namespace core {

		AudioDriverInfo MsDirectSound::info( ) const {
			return AudioDriverInfo("dsound","Microsoft DirectSound Driver","Microsoft output driver",true);
		}

		void MsDirectSound::Error(const WCHAR msg[]) {
			MessageBoxW(0, msg, L"DirectSound Output driver", MB_OK | MB_ICONERROR);
		}

		MsDirectSound::MsDirectSound()
		:
			device_guid(), // DSDEVID_DefaultPlayback <- unresolved external.
			_initialized(),
			_configured(),
			_running(),
			_playing(),
			_threadRun(),
			_pDs(),
			_pBuffer(),
			_pCallback()
		{
			_capEnums.resize(0);
			_capPorts.resize(0);
		}

		void MsDirectSound::Initialize( AUDIODRIVERWORKFN pCallback, void * context ) {
			_callbackContext = context;
			_pCallback = pCallback;
			_running = false;
			//_hwnd = NApp::mainWindow()->win();
			ReadConfig();
			DirectSoundCaptureEnumerate(DSEnumCallback,&_capEnums);
			_initialized = true;
		}

		MsDirectSound::~MsDirectSound() {
			Stop();
		}

		bool MsDirectSound::Start() {
			//CSingleLock lock(&_lock, true);
			if(_running) return true;
			if(!_pCallback) return false;
			if( FAILED( ::CoInitialize(NULL) ) ) {
				Error(L"(DirectSound) Failed to initialize COM");
				return false;
			}
			if(FAILED(::DirectSoundCreate8(device_guid != GUID() ? &device_guid : 0, &_pDs, 0)))
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

			DSBCAPS caps;
			DSBUFFERDESC desc;
			WAVEFORMATEX format;
			// Set up wave format structure. 
			memset(&format, 0, sizeof(WAVEFORMATEX)); 
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.nChannels = 2;
			format.wBitsPerSample = 16;//_bitDepth;
			format.nSamplesPerSec = settings().samplesPerSec();
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
				Error(L"Failed to create DirectSound Buffer(s)");
				_pDs->Release();
				_pDs = 0;
				return false;
			}

			if(_exclusive)
			{
//				_pBuffer->Stop();
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
//				WriteConfig();
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
			for (unsigned int i=0; i<_capPorts.size();i++)
				CreateCapturePort(_capPorts[i]);

			///\todo: what's this?
			//_event.ResetEvent();
			_threadRun = true;
			_playing = false;
			DWORD dwThreadId;
			CreateThread( NULL, 0, PollerThread, this, 0, &dwThreadId );
			_running = true;
			return true;
		}

		bool MsDirectSound::Stop()
		{
			//CSingleLock lock(&_lock, true);
			if(!_running) return true;
			_threadRun = false;
			//CSingleLock event(&_event, true);
			// Once we get here, the PollerThread should have stopped
			///\todo: some threadlocking mechanism. For now adding this sleeps
			Sleep(1000);

			if(_playing)
			{
				_pBuffer->Stop();
				_playing = false;
			}
			_pBuffer->Release();
			_pBuffer = 0;
			_pDs->Release();
			_pDs = 0;
			for(unsigned int i=0; i<_capPorts.size(); i++)
			{
				_capPorts[i]._pBuffer->Stop();
				_capPorts[i]._pBuffer->Release();
				_capPorts[i]._pDs->Release();
				_capPorts[i]._pDs=0;
				universalis::os::aligned_memory_dealloc(_capPorts[i].pleft);
				universalis::os::aligned_memory_dealloc(_capPorts[i].pright);
			}
			_capPorts.resize(0);
			_running = false;
			// Release COM
			CoUninitialize();

			return true;
		}

		void MsDirectSound::GetCapturePorts(std::vector<std::string>&ports)
		{
			for (unsigned int i=0;i<_capEnums.size();i++) ports.push_back(_capEnums[i].portname);
		}
		BOOL CALLBACK MsDirectSound::DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
		{
			std::vector<PortEnums>* ports=static_cast<std::vector<PortEnums>*>(lpContext);
			PortEnums port(lpGuid,lpcstrDescription);
			ports->push_back(port);
			return TRUE;
		}
		bool MsDirectSound::AddCapturePort(int idx)
		{
			bool isplaying = _running;
			if ( idx >= _capEnums.size() ) return false;
			for (unsigned int i=0;i<_capPorts.size();++i)
			{
				if (_capPorts[i]._pGuid == _capEnums[idx].guid ) return false;
			}
			PortCapt port;
			port._pGuid = _capEnums[idx].guid;
			if (isplaying)
			{
				Stop();
			}
			_capPorts.push_back(port);
			if ( _portMapping.size() <= idx) _portMapping.resize(idx+1);
			_portMapping[idx]=_capPorts.size()-1;
			if (isplaying)
			{
				return Start();
			}
			return true;
		}
		bool MsDirectSound::RemoveCapturePort(int idx)
		{
			bool restartplayback = false;
			std::vector<PortCapt> newports;
			if ( idx >= _capEnums.size() ) return false;
			for (unsigned int i=0;i<_capPorts.size();++i)
			{
				if (_capPorts[i]._pGuid == _capEnums[idx].guid )
				{
					if (_playing)
					{
						Stop();
						restartplayback=true;
					}
				}
				else 
				{
					///\todo: this assignation is probably wrong. should be checked.
					_portMapping[newports.size()]=_portMapping[i];
					newports.push_back(_capPorts[i]);
				}
			}
			_portMapping.resize(newports.size());
			_capPorts = newports;
			if (restartplayback) Start();
			return true;
		}
		bool MsDirectSound::CreateCapturePort(PortCapt &port)
		{
			HRESULT hr;
			//not try to open a port twice
			if (port._pDs) return true;

			// Create IDirectSoundCapture using the preferred capture device
			if( FAILED( hr = DirectSoundCaptureCreate8( port._pGuid, &port._pDs, NULL ) ) )
			{
				Error(L"Failed to create Capture DirectSound Device");
				return false;
			}

			// Create the capture buffer
			DSCBUFFERDESC dscbd;
			WAVEFORMATEX format;
			// Set up wave format structure. 
			ZeroMemory(&format, sizeof(WAVEFORMATEX));
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.nChannels = 2;
			format.wBitsPerSample = 16;//_bitDepth;
			format.nSamplesPerSec = settings().samplesPerSec();
			format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
			format.cbSize = 0;

			ZeroMemory( &dscbd, sizeof(dscbd) );
			dscbd.dwSize        = sizeof(dscbd);
			dscbd.dwBufferBytes = _dsBufferSize;
			dscbd.lpwfxFormat   = &format;

			if( FAILED( hr = port._pDs->CreateCaptureBuffer( &dscbd, reinterpret_cast<LPDIRECTSOUNDCAPTUREBUFFER*>(&port._pBuffer), NULL ) ) )
			{
				Error(L"Failed to create Capture DirectSound Buffer(s)");
				return false;
			}
			hr = port._pBuffer->Start(DSCBSTART_LOOPING);
			universalis::os::aligned_memory_alloc(16, port.pleft, _dsBufferSize);
			universalis::os::aligned_memory_alloc(16, port.pright, _dsBufferSize);
			return true;
		}

		void MsDirectSound::GetReadBuffers(int idx,float **pleft, float **pright,int numsamples)
		{
			if (_running)
			{
				if (idx >=_capPorts.size()) return;
				*pleft=_capPorts[_portMapping[idx]].pleft+_capPorts[_portMapping[idx]]._machinepos;
				*pright=_capPorts[_portMapping[idx]].pright+_capPorts[_portMapping[idx]]._machinepos;
				_capPorts[_portMapping[idx]]._machinepos+=numsamples;
			}
		}

		DWORD WINAPI MsDirectSound::PollerThread(void * pDirectSound)
		{
			universalis::os::thread_name thread_name("direct sound");
			universalis::processor::exception::install_handler_in_thread();
			MsDirectSound * pThis = reinterpret_cast<MsDirectSound*>( pDirectSound );
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			SetThreadAffinityMask(GetCurrentThread(), 1);
			//Prefill buffer:
			for(int i=0; i< pThis->_buffersToDo;i++)
			{
				//CSingleLock lock(&pThis->_lock, TRUE);
				for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
				{
					pThis->DoBlocksRecording(pThis->_capPorts[i]);
				}
				pThis->DoBlocks();
			}

			while(pThis->_threadRun)
			{
				int runs=0;

				while(pThis->WantsMoreBlocks())
				{
					for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
					{
						pThis->DoBlocksRecording(pThis->_capPorts[i]);
					}

					pThis->DoBlocks();
					if (++runs > pThis->_numBuffers)
						break;
				}
				::Sleep(10);
			}
			//_event.SetEvent();
			//::_endthread();
			return 0;
		}
		bool MsDirectSound::WantsMoreBlocks()
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
		bool MsDirectSound::WantsMoreBlocksRecording(PortCapt& port)
		{
			// [_lowMark,_highMark] is the next buffer to be filled.
			// if pos is still inside, we have to wait.
/*			int pos=0;
			HRESULT hr = port._pBuffer->GetCurrentPosition(0,(DWORD*)&pos);
			if(hr == DSERR_BUFFERLOST)
			{
				_playing = false;
				if(FAILED(port._pBuffer->Start(DSCBSTART_LOOPING)))	return false;
				hr = port._pBuffer->GetCurrentPosition(0,(DWORD*)&pos);
				if (FAILED(hr)) return false;
				else return true;
			}
			if (FAILED(hr)) return false;
			int _highMark= _lowMark+_runningBufSize;
			if(_highMark < port._lowMark)
			{
				if((pos >= port._lowMark) || (pos < _highMark)) return false;
			}
			else if((pos >= port._lowMark) && (pos < _highMark))	return false;
*/
			return true;
		}

		// First, do the capture buffers so that audio is available to wavein machines.
		void MsDirectSound::DoBlocksRecording(PortCapt& port)
		{
			int* pBlock1 , *pBlock2;
			unsigned long blockSize1, blockSize2;
			HRESULT hr = port._pBuffer->Lock(port._lowMark, _runningBufSize, 
				(void**)&pBlock1, &blockSize1, 
				(void**)&pBlock2, &blockSize2, 0);
			if (DSERR_BUFFERLOST == hr) 
			{ 
				port._lowMark=0;
				hr = port._pBuffer->Lock(0, _runningBufSize, 
					(void**)&pBlock1, &blockSize1, 
					(void**)&pBlock2, &blockSize2, 0);
			} 
			if (SUCCEEDED(hr))
			{ 
				// Put the audio in our float buffers.
				int numSamples = blockSize1 / settings().sampleSize();
				DeQuantize16AndDeinterlace(pBlock1, port.pleft,port.pright, numSamples);
				port._lowMark += blockSize1;
				if (blockSize2 > 0)
				{
					DeQuantize16AndDeinterlace(pBlock2, port.pleft+numSamples,port.pright+numSamples, blockSize2 / settings().sampleSize());
					_lowMark += blockSize2;
				}
				// Release the data back to DirectSound. 
				hr = port._pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);

				if ( port._lowMark >= _dsBufferSize ) port._lowMark -= _dsBufferSize;
			}
			port._machinepos=0;
		}
		void MsDirectSound::DoBlocks()
		{
			// Next, proceeed with the generation of audio
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
				int numSamples = blockSize1 / settings().sampleSize();
				float *pFloatBlock = _pCallback(_callbackContext, numSamples);
				// if(_dither) QuantizeWithDither(pFloatBlock, pBlock1, numSamples); else 
				quantize(pFloatBlock, pBlock1, numSamples);
				_lowMark += blockSize1;

				while(blockSize2 > 0)
				{
					numSamples = blockSize2 / settings().sampleSize();
					float *pFloatBlock = _pCallback(_callbackContext, numSamples);
					// if(_dither) QuantizeWithDither(pFloatBlock, pBlock2, numSamples); else 
					quantize(pFloatBlock, pBlock2, numSamples);
					_lowMark += blockSize2;
				}
				// Release the data back to DirectSound. 
				hr = _pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);
				if(SUCCEEDED(hr) && !_playing)
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
			_bufferSize = 8192;
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
			DWORD playPos;
			if(FAILED(_pBuffer->GetCurrentPosition(&playPos, 0)))
			{
				Error(L"DirectSoundBuffer::GetCurrentPosition failed");
				return 0;
			}
			return playPos;
		}

		int MsDirectSound::GetWritePos()
		{
			if(!_running) return 0;
			DWORD writePos;
			if(FAILED(_pBuffer->GetCurrentPosition(0, &writePos)))
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

		void MsDirectSound::quantize(float *pin, int *piout, int c) {
			///\todo use the functions already available in the base class.
			
			using psycle::helpers::math::furti;

			int const SHORT_MIN = -32768;
			int const SHORT_MAX = +32767;
			do {
				int l = furti<int>((pin[0]));
				if(l < SHORT_MIN) l = SHORT_MIN;
				else if(l > SHORT_MAX) l = SHORT_MAX;

				int r = furti<int>((pin[1]));
				if(r < SHORT_MIN) r = SHORT_MIN;
				else if(r > SHORT_MAX) r = SHORT_MAX;

				*piout++ = (r << 16) | static_cast<std::uint16_t>(l);
				pin += 2;
			} while(--c);
		}
}}
#endif // defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
