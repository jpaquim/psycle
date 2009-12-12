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
#include <psycle/helpers/math/rint.hpp>
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


namespace psycle { namespace core {

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
			dsBufferPlaying_(),
			threadRunning_(),
			stopRequested_(),
			_pDs(),
			_pBuffer(),
			_pCallback(),
			ui_(0)
		{
			_capEnums.resize(0);
			_capPorts.resize(0);
		}

		MsDirectSound::MsDirectSound(DSoundUiInterface* ui)
		:
			device_guid(), // DSDEVID_DefaultPlayback <- unresolved external.
			_initialized(),
			_configured(),
			dsBufferPlaying_(),
			threadRunning_(),
			stopRequested_(),
			_pDs(),
			_pBuffer(),
			_pCallback(),
			ui_(ui)
		{
			_capEnums.resize(0);
			_capPorts.resize(0);
		}

		void MsDirectSound::Initialize( AUDIODRIVERWORKFN pCallback, void * context ) {
			_callbackContext = context;
			_pCallback = pCallback;
			//_hwnd = NApp::mainWindow()->win();
			ReadConfig();
			_capEnums.resize(0);
			_capPorts.resize(0);
			DirectSoundCaptureEnumerate(DSEnumCallback,&_capEnums);
			_initialized = true;
		}

		MsDirectSound::~MsDirectSound() {
			Stop();
		}

		bool MsDirectSound::Start() {
			// return immediatly if the thread is already running
			if(threadRunning_) return true;
			if(!_initialized || !_pCallback) {
				Error(L"(DirectSound) Failed to start: Not initialized or callback is null");
				return false;
			}
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
			format.nChannels = playbackSettings().numChannels();
			format.wBitsPerSample = playbackSettings().bitDepth();
			format.nSamplesPerSec = playbackSettings().samplesPerSec();
			format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
			format.cbSize = 0;

			int dsBufferSize = _exclusive ? 0 : playbackSettings().blockBytes()*playbackSettings().blockCount();
			// Set up DSBUFFERDESC structure. 
			memset(&desc, 0, sizeof(DSBUFFERDESC));
			desc.dwSize = sizeof(DSBUFFERDESC); 
			desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
			desc.dwFlags |= _exclusive ? DSBCAPS_PRIMARYBUFFER : DSBCAPS_GLOBALFOCUS;
			desc.dwBufferBytes = dsBufferSize; 
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
				//_pBuffer->Stop();
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
				dsBufferSize = caps.dwBufferBytes;
				_runningBufSize = dsBufferSize*0.5f;
				_buffersToDo = 1;
			}
			else
			{
				_runningBufSize = playbackSettings().blockBytes();
				_buffersToDo = playbackSettings().blockCount();
			}
			_lowMark = 0;
			_highMark = _runningBufSize;
			playbackSettings_.setTotalBufferBytes(dsBufferSize);

			_pBuffer->Initialize(_pDs,&desc);
			for (unsigned int i=0; i<_capPorts.size();i++)
				CreateCapturePort(_capPorts[i]);

			dsBufferPlaying_ = false;
			DWORD dwThreadId;
			CreateThread( NULL, 0, PollerThreadStatic, this, 0, &dwThreadId );
			// wait for the thread to be running
			{ scoped_lock lock(mutex_);
				while(!threadRunning_) condition_.wait(lock);
			}
			return true;
		}

		bool MsDirectSound::Stop()
		{
			if(!threadRunning_) return true;
			// ask the thread to terminate
			{
				scoped_lock lock(mutex_);
				stopRequested_ = true;
			}
			condition_.notify_one();

			/// join the thread
			{
				scoped_lock lock(mutex_);
				while(threadRunning_) condition_.wait(lock);
				stopRequested_ = false;
			}
			// Once we get here, the PollerThread should have stopped

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
		bool MsDirectSound::AddCapturePort(std::uint32_t idx)
		{
			bool isplaying = threadRunning_;
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
		bool MsDirectSound::RemoveCapturePort(std::uint32_t idx)
		{
			bool restartplayback = false;
			std::vector<PortCapt> newports;
			if ( idx >= _capEnums.size() ) return false;
			for (unsigned int i=0;i<_capPorts.size();++i)
			{
				if (_capPorts[i]._pGuid == _capEnums[idx].guid )
				{
					if (dsBufferPlaying_)
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
			format.nChannels = captureSettings().numChannels();
			format.wBitsPerSample = captureSettings().bitDepth();
			format.nSamplesPerSec = captureSettings().samplesPerSec();
			format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
			format.cbSize = 0;

			///\todo: The buffer size may not be the appropiate. Note that if this changes,
			/// the doBlocksRecording method has to be modified.
			int dsBufferSize = _exclusive ? 0 : playbackSettings().totalBufferBytes();
			ZeroMemory( &dscbd, sizeof(DSCBUFFERDESC) );
			dscbd.dwSize        = sizeof(DSCBUFFERDESC);
			dscbd.dwBufferBytes = dsBufferSize;
			dscbd.lpwfxFormat   = &format;

			if( FAILED( hr = port._pDs->CreateCaptureBuffer( &dscbd, reinterpret_cast<LPDIRECTSOUNDCAPTUREBUFFER*>(&port._pBuffer), NULL ) ) )
			{
				Error(L"Failed to create Capture DirectSound Buffer(s)");
				return false;
			}
			hr = port._pBuffer->Start(DSCBSTART_LOOPING);
			universalis::os::aligned_memory_alloc(16, port.pleft, dsBufferSize);
			universalis::os::aligned_memory_alloc(16, port.pright, dsBufferSize);
			return true;
		}

		void MsDirectSound::GetReadBuffers(std::uint32_t idx,float **pleft, float **pright,int numsamples)
		{
			if (threadRunning_)
			{
				if (idx >=_capPorts.size()) return;
				*pleft=_capPorts[_portMapping[idx]].pleft+_capPorts[_portMapping[idx]]._machinepos;
				*pright=_capPorts[_portMapping[idx]].pright+_capPorts[_portMapping[idx]]._machinepos;
				_capPorts[_portMapping[idx]]._machinepos+=numsamples;
			}
		}

		DWORD WINAPI MsDirectSound::PollerThreadStatic(void * pDirectSound)
		{
			universalis::os::thread_name thread_name("direct sound");
			universalis::processor::exception::install_handler_in_thread();
			MsDirectSound * pThis = reinterpret_cast<MsDirectSound*>( pDirectSound );
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			SetThreadAffinityMask(GetCurrentThread(), 1);
			pThis->PollerThread();
			//::_endthread();
			return 0;
		}
		void MsDirectSound::PollerThread()
		{
			// notify that the thread is now running
			{
				scoped_lock lock(mutex_);
				threadRunning_ = true;
			}
			condition_.notify_one();
			//Prefill buffer:
			for(int i=0; i< _buffersToDo;i++)
			{
				for (unsigned int i =0; i < _capPorts.size(); i++)
				{
					DoBlocksRecording(_capPorts[i]);
				}
				if (playbackSettings().bitDepth() == 16) {
					DoBlocks16();
				} else if (playbackSettings().bitDepth() == 24) {
					DoBlocks24();
				}
			}
			while(true)
			{
				std::uint32_t runs=0;

				while(WantsMoreBlocks())
				{
					for (unsigned int i =0; i < _capPorts.size(); i++)
					{
						DoBlocksRecording(_capPorts[i]);
					}
					if (playbackSettings().bitDepth() == 16) {
						DoBlocks16();
					} else if (playbackSettings().bitDepth() == 24) {
						DoBlocks24();
					}

					// This if prevents the application to become unresponsible.
					// in case of too high cpu usage.
					if (++runs > playbackSettings().blockCount())
						break;
				}
				// check whether the thread has been asked to terminate
				{
					scoped_lock lock(mutex_);
					if(stopRequested_) break;
				}
				::Sleep(10);
			}

			if(dsBufferPlaying_)
			{
				_pBuffer->Stop();
				dsBufferPlaying_ = false;
			}

			// notify that the thread is not running anymore
			{
				scoped_lock lock(mutex_);
				threadRunning_ = false;
			}
			condition_.notify_one();
		}
		bool MsDirectSound::WantsMoreBlocks()
		{
			// [_lowMark,_highMark] is the next buffer to be filled.
			// if pos is still inside, we have to wait.
			DWORD pos=0;
			HRESULT hr = _pBuffer->GetCurrentPosition(&pos, 0);
			if(hr == DSERR_BUFFERLOST)
			{
				dsBufferPlaying_ = false;
				if(FAILED(_pBuffer->Restore())) return false;
				hr = _pBuffer->GetCurrentPosition(&pos, 0);
				if (FAILED(hr)) return false;
				else return true;
			}
			if (FAILED(hr)) return false;
			if(_highMark < _lowMark)
			{
				if((pos >= _lowMark) || (pos < _highMark)) return false;
			}
			else if((pos >= _lowMark) && (pos < _highMark)) return false;
			return true;
		}
		bool MsDirectSound::WantsMoreBlocksRecording(PortCapt& port)
		{
			// [_lowMark,_highMark] is the next buffer to be filled.
			// if pos is still inside, we have to wait.
#if 0
			DWORD pos=0;
			HRESULT hr = port._pBuffer->GetCurrentPosition(0,&pos);
			if(hr == DSERR_BUFFERLOST)
			{
				dsBufferPlaying_ = false;
				if(FAILED(port._pBuffer->Start(DSCBSTART_LOOPING))) return false;
				hr = port._pBuffer->GetCurrentPosition(0,&pos);
				if (FAILED(hr)) return false;
				else return true;
			}
			if (FAILED(hr)) return false;
			int _highMark= _lowMark+_runningBufSize;
			if(_highMark < port._lowMark)
			{
				if((pos >= port._lowMark) || (pos < _highMark)) return false;
			}
			else if((pos >= port._lowMark) && (pos < _highMark)) return false;
#endif
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
				int numSamples = blockSize1 / captureSettings().sampleSize();
				if(captureSettings().bitDepth() == 16) {
					DeQuantize16AndDeinterlace(pBlock1, port.pleft,port.pright, numSamples);
				}
				else if (captureSettings().bitDepth() == 24) {
					DeQuantize24AndDeinterlace(pBlock1, port.pleft,port.pright, numSamples);
				}
				port._lowMark += blockSize1;
				if (blockSize2 > 0)
				{
					numSamples = blockSize2 / captureSettings().sampleSize();
					if(captureSettings().bitDepth() == 16) {
						DeQuantize16AndDeinterlace(pBlock2, port.pleft+numSamples,port.pright+numSamples, numSamples);
					}
					else if (captureSettings().bitDepth() == 24) {
						DeQuantize24AndDeinterlace(pBlock2, port.pleft+numSamples,port.pright+numSamples, numSamples);
					}
					_lowMark += blockSize2;
				}
				// Release the data back to DirectSound. 
				hr = port._pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);

				if ( port._lowMark >= playbackSettings().totalBufferBytes() ) {
					port._lowMark -= playbackSettings().totalBufferBytes();
				}
			}
			port._machinepos=0;
		}
		void MsDirectSound::DoBlocks16()
		{
			// Next, proceeed with the generation of audio
			std::int16_t* pBlock1 , *pBlock2;
			DWORD blockSize1, blockSize2;
			// Obtain write pointer. 
			HRESULT hr = _pBuffer->Lock(_lowMark, _runningBufSize, 
				(void**)&pBlock1, &blockSize1, 
				(void**)&pBlock2, &blockSize2, 0);
			if (DSERR_BUFFERLOST == hr) 
			{ 
				// If DSERR_BUFFERLOST is returned, restore and retry lock. 
				_pBuffer->Restore(); 
				dsBufferPlaying_ = false;
				hr = _pBuffer->Lock(_lowMark, _runningBufSize, 
					(void**)&pBlock1, &blockSize1, 
					(void**)&pBlock2, &blockSize2, 0);
			} 
			if (SUCCEEDED(hr))
			{ 
				// Generate audio and put it into the buffer
				int numSamples = blockSize1 / playbackSettings().sampleSize();
				float *pFloatBlock = _pCallback(_callbackContext, numSamples);
				if(_dither) Quantize16WithDither(pFloatBlock, pBlock1, numSamples);
				else Quantize16(pFloatBlock, pBlock1, numSamples);
				_lowMark += blockSize1;

				if(blockSize2 > 0)
				{
					numSamples = blockSize2 / playbackSettings().sampleSize();
					float *pFloatBlock = _pCallback(_callbackContext, numSamples);
					if(_dither) Quantize16WithDither(pFloatBlock, pBlock2, numSamples);
					else Quantize16(pFloatBlock, pBlock2, numSamples);
					_lowMark += blockSize2;
				}
				// Release the data back to DirectSound. 
				hr = _pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);
				if(SUCCEEDED(hr) && !dsBufferPlaying_)
				{
					dsBufferPlaying_ = true;
					hr = _pBuffer->Play(0, 0, DSBPLAY_LOOPING);
				}
				_highMark = _lowMark + _runningBufSize;
				const std::uint32_t dsBufferSize = playbackSettings().totalBufferBytes();
				if(_highMark > dsBufferSize)
				{
					_highMark -= dsBufferSize;
					if ( _lowMark >= dsBufferSize ) _lowMark -= dsBufferSize;
				}
			}
		}
		void MsDirectSound::DoBlocks24()
		{
			// Next, proceeed with the generation of audio
			std::int32_t* pBlock1 , *pBlock2;
			DWORD blockSize1, blockSize2;
			// Obtain write pointer. 
			HRESULT hr = _pBuffer->Lock(_lowMark, _runningBufSize, 
				(void**)&pBlock1, &blockSize1, 
				(void**)&pBlock2, &blockSize2, 0);
			if (DSERR_BUFFERLOST == hr) 
			{ 
				// If DSERR_BUFFERLOST is returned, restore and retry lock. 
				_pBuffer->Restore(); 
				dsBufferPlaying_ = false;
				hr = _pBuffer->Lock(_lowMark, _runningBufSize, 
					(void**)&pBlock1, &blockSize1, 
					(void**)&pBlock2, &blockSize2, 0);
			} 
			if (SUCCEEDED(hr))
			{ 
				// Generate audio and put it into the buffer
				int numSamples = blockSize1 / playbackSettings().sampleSize();
				float *pFloatBlock = _pCallback(_callbackContext, numSamples);
				if(_dither) Quantize24WithDither(pFloatBlock, pBlock1, numSamples);
				else Quantize24(pFloatBlock, pBlock1, numSamples);
				_lowMark += blockSize1;

				if(blockSize2 > 0)
				{
					numSamples = blockSize2 / playbackSettings().sampleSize();
					float *pFloatBlock = _pCallback(_callbackContext, numSamples);
					if(_dither) Quantize24WithDither(pFloatBlock, pBlock2, numSamples);
					else Quantize24(pFloatBlock, pBlock2, numSamples);
					_lowMark += blockSize2;
				}
				// Release the data back to DirectSound. 
				hr = _pBuffer->Unlock(pBlock1, blockSize1, pBlock2, blockSize2);
				if(SUCCEEDED(hr) && !dsBufferPlaying_)
				{
					dsBufferPlaying_ = true;
					hr = _pBuffer->Play(0, 0, DSBPLAY_LOOPING);
				}
				_highMark = _lowMark + _runningBufSize;
				const std::uint32_t dsBufferSize = playbackSettings().totalBufferBytes();
				if(_highMark > dsBufferSize)
				{
					_highMark -= dsBufferSize;
					if ( _lowMark >= dsBufferSize ) _lowMark -= dsBufferSize;
				}
			}
		}
		void MsDirectSound::ReadConfig()
		{
			// default configuration
			device_guid = GUID(); // DSDEVID_DefaultPlayback <-- unresolved external symbol
			_exclusive = false;
			_dither = false;
			playbackSettings_.setBitDepth(16);
			playbackSettings_.setChannelMode(3);
			playbackSettings_.setSamplesPerSec(44100);
			playbackSettings_.setBlockBytes(8192);
			playbackSettings_.setBlockCount(4);
			captureSettings_.setBitDepth(16);
			captureSettings_.setChannelMode(3);
			captureSettings_.setSamplesPerSec(44100);
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

			if (ui_) {
				ui_->SetValues(device_guid,
							   _exclusive,
							   _dither, 
							   playbackSettings_.samplesPerSec(),
							   playbackSettings_.blockBytes(),
							   playbackSettings_.blockCount());
				if (ui_->DoModal() != IDOK) return;
				_configured = true;

				// save the settings to be able to rollback if it doesn't work
				GUID device_guid = this->device_guid;
				bool exclusive = _exclusive;
				bool dither = _dither;
				int samplesPerSec = playbackSettings_.samplesPerSec();
				int bufferSize = playbackSettings_.blockBytes();
				int numBuffers = playbackSettings_.blockCount();

				if(_initialized) Stop();

				int tmp_samplespersec, tmp_blockbytes, tmp_block_count;
				ui_->GetValues(this->device_guid,
							   _exclusive,
							   _dither,
							   tmp_samplespersec,
							   tmp_blockbytes, 
							   tmp_block_count);
				playbackSettings_.setSamplesPerSec(tmp_samplespersec);
				playbackSettings_.setBlockBytes(tmp_blockbytes);
				playbackSettings_.setBlockCount(tmp_block_count);

				if(_initialized)
				{
					if(Start()) WriteConfig();
					else
					{
						// rollback
						this->device_guid = device_guid;
						_exclusive = exclusive;
						_dither = dither;
						playbackSettings_.setSamplesPerSec(samplesPerSec);
						playbackSettings_.setBlockBytes(bufferSize);
						playbackSettings_.setBlockCount(numBuffers);
						Start();
					}
				}
				else WriteConfig();
			}
		}

		int MsDirectSound::GetPlayPos()
		{
			if(!threadRunning_) return 0;
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
			if(!threadRunning_) return 0;
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
}}
#endif // defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
