///\file
///\brief implementation file for psycle::host::WaveOut.


#include "WaveOut.hpp"
#include "WaveOutDialog.hpp"
#include "Registry.hpp"
#include "Configuration.hpp"
#include "MidiInput.hpp"
#include <universalis.hpp>
#include <universalis/os/thread_name.hpp>
#include <universalis/os/aligned_alloc.hpp>
#include <psycle/helpers/dsp.hpp>
#include <process.h>
#include "cpu_time_clock.hpp"

namespace psycle
{
	namespace host
	{
		AudioDriverInfo WaveOut::_info = { "Windows WaveOut MME" };
		AudioDriverEvent WaveOut::_event;
//		CCriticalSection WaveOut::_lock;

		void WaveOut::Error(char const msg[])
		{
			MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
		}

		WaveOut::WaveOut()
			: _initialized(false)
			, _configured(false)
			, _running(false)
			, _pCallback(0)
		{
			_capEnums.resize(0);
			_capPorts.resize(0);
		}

		void WaveOut::Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void * context)
		{
			_callbackContext = context;
			_pCallback = pCallback;
			_running = false;
			ReadConfig();
			EnumerateCapturePorts();
			_initialized = true;
		}
		void WaveOut::RefreshAvailablePorts()
		{
			EnumeratePlaybackPorts();
			EnumerateCapturePorts();
		}
		void WaveOut::EnumeratePlaybackPorts()
		{
			_playEnums.resize(0);
			for (int i = 0; i < waveOutGetNumDevs(); i++)
			{
				WAVEOUTCAPS caps;
				waveOutGetDevCaps(i, &caps, sizeof(WAVEOUTCAPS));
				PortEnums port(caps.szPname, i);
				_playEnums.push_back(port);
			}
		}
		void WaveOut::EnumerateCapturePorts()
		{
			_capEnums.resize(0);
			for (unsigned int i=0; i < waveInGetNumDevs(); ++i)
			{
				WAVEINCAPS caps;
				waveInGetDevCaps(i,&caps,sizeof(WAVEINCAPS));
				PortEnums port(caps.szPname, i);
				_capEnums.push_back(port);
			}
		}

		void WaveOut::Reset()
		{
			if (_running) Stop();
		}

		WaveOut::~WaveOut() throw()
		{
			if(_initialized) Reset();
		}

		bool WaveOut::Start()
		{
//			CSingleLock lock(&_lock, TRUE);
			if(_running) return true;
			if(!_pCallback) return false;

			WAVEFORMATPCMEX format;
			PrepareWaveFormat(format,2,_samplesPerSec,_sampleBits, _sampleValidBits);
			if(::waveOutOpen(&_handle, _deviceID, reinterpret_cast<LPWAVEFORMATEX>(&format), 0, 0, 0) != MMSYSERR_NOERROR)
			{
				Error("waveOutOpen() failed");
				return false;
			}

			_currentBlock = 0;
			_writePos = 0;
			m_readPosWraps = 0;
			m_lastPlayPos = 0;

			// allocate blocks
			for(CBlock *pBlock = _blocks; pBlock < _blocks + _numBlocks; pBlock++)
			{
				pBlock->Handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, _blockSizeBytes);
				pBlock->pData = (byte *)GlobalLock(pBlock->Handle);
			}

			// allocate block headers
			for(CBlock *pBlock = _blocks; pBlock < _blocks + _numBlocks; pBlock++)
			{
				pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
				pBlock->pHeader = (WAVEHDR *)GlobalLock(pBlock->HeaderHandle);

				WAVEHDR *ph = pBlock->pHeader;
				ph->lpData = (char *)pBlock->pData;
				ph->dwBufferLength = _blockSizeBytes;
				ph->dwFlags = WHDR_DONE;
				ph->dwLoops = 0;

				pBlock->Prepared = false;
			}

			for (unsigned int i=0; i<_capPorts.size();i++)
				CreateCapturePort(_capPorts[i]);

			_stopPolling = false;
			_event.ResetEvent();
			::_beginthread(PollerThread, 0, this);
			_running = true;
			CMidiInput::Instance()->ReSync();	// MIDI IMPLEMENTATION
			return true;
		}

		bool WaveOut::Stop()
		{
//			CSingleLock lock(&_lock, TRUE);
			if(!_running) return true;
			_stopPolling = true;
			CSingleLock event(&_event, TRUE);
			// Once we get here, the PollerThread should have stopped
			if(::waveOutReset(_handle) != MMSYSERR_NOERROR)
			{
				Error("waveOutReset() failed");
				return false;
			}
			for(;;)
			{
				bool alldone = true;
				for(CBlock *pBlock = _blocks; pBlock < _blocks + _numBlocks; pBlock++)
				{
					if((pBlock->pHeader->dwFlags & WHDR_DONE) == 0) alldone = false;
				}
				if(alldone) break;
				::Sleep(10);
			}
			for(CBlock *pBlock = _blocks; pBlock < _blocks + _numBlocks; pBlock++)
			{
				if(pBlock->Prepared)
				{
					if(::waveOutUnprepareHeader(_handle, pBlock->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
					{
						Error("waveOutUnprepareHeader() failed");
					}
				}
			}
			if(::waveOutClose(_handle) != MMSYSERR_NOERROR)
			{
				Error("waveOutClose() failed");
				return false;
			}
			for(CBlock *pBlock = _blocks; pBlock < _blocks + _numBlocks; pBlock++)
			{
				::GlobalUnlock(pBlock->Handle);
				::GlobalFree(pBlock->Handle);
				::GlobalUnlock(pBlock->HeaderHandle);
				::GlobalFree(pBlock->HeaderHandle);
			}
			for(unsigned int i=0; i<_capPorts.size(); i++)
			{
				if(_capPorts[i]._handle == NULL)
					continue;
				if(::waveInReset(_capPorts[i]._handle) != MMSYSERR_NOERROR)
				{
					Error("waveInReset() failed");
					return false;
				}
				///\todo: wait until WHDR_DONE like with waveout?
				for(CBlock *pBlock = _capPorts[i]._blocks; pBlock < _capPorts[i]._blocks + _numBlocks; pBlock++)
				{
					if(pBlock->Prepared)
					{
						if(::waveInUnprepareHeader(_capPorts[i]._handle, pBlock->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
						{
							Error("waveInUnprepareHeader() failed");
						}
					}
				}
				waveInClose(_capPorts[i]._handle);
				_capPorts[i]._handle = NULL;
				universalis::os::aligned_memory_dealloc(_capPorts[i].pleft);
				universalis::os::aligned_memory_dealloc(_capPorts[i].pright);
				for(CBlock *pBlock = _capPorts[i]._blocks; pBlock < _capPorts[i]._blocks + _numBlocks; pBlock++)
				{
					::GlobalUnlock(pBlock->Handle);
					::GlobalFree(pBlock->Handle);
					::GlobalUnlock(pBlock->HeaderHandle);
					::GlobalFree(pBlock->HeaderHandle);
				}
			}

			_running = false;
			return true;
		}
		void WaveOut::GetPlaybackPorts(std::vector<std::string> &ports)
		{
			ports.resize(0);
			for (unsigned int i=0;i<_playEnums.size();i++) ports.push_back(_playEnums[i].portname);
		}
		void WaveOut::GetCapturePorts(std::vector<std::string> &ports)
		{
			ports.resize(0);
			for (unsigned int i=0;i<_capEnums.size();i++) ports.push_back(_capEnums[i].portname);
		}
		bool WaveOut::AddCapturePort(int idx)
		{
			bool isplaying = _running;
			if ( idx >= _capEnums.size()) return false;
			if ( idx < _portMapping.size() && _portMapping[idx] != -1) return true;
			
			if (isplaying) Stop();
			PortCapt port;
			port._idx = idx;
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
		bool WaveOut::RemoveCapturePort(int idx)
		{
			bool isplaying = _running;
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
		bool WaveOut::CreateCapturePort(PortCapt &port)
		{
			HRESULT hr;
			//not try to open a port twice
			if (port._handle) return true;

			WAVEFORMATPCMEX format;
			PrepareWaveFormat(format,2,_samplesPerSec,_sampleBits, _sampleValidBits);

			if ((hr = waveInOpen(&port._handle,port._idx,reinterpret_cast<LPWAVEFORMATEX>(&format),NULL,NULL,CALLBACK_NULL)) != MMSYSERR_NOERROR )
			{
				Error(_T("waveInOpen() failed"));
				return false;
			}
			// allocate blocks
			for(CBlock *pBlock = port._blocks; pBlock < port._blocks + _numBlocks; pBlock++)
			{
				pBlock->Handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, _blockSizeBytes);
				pBlock->pData = (byte *)GlobalLock(pBlock->Handle);
			}

			// allocate block headers
			for(CBlock *pBlock = port._blocks; pBlock < port._blocks + _numBlocks; pBlock++)
			{
				pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
				pBlock->pHeader = (WAVEHDR *)GlobalLock(pBlock->HeaderHandle);

				WAVEHDR *ph = pBlock->pHeader;
				ph->lpData = (char *)pBlock->pData;
				ph->dwBufferLength = _blockSizeBytes;
				ph->dwFlags = WHDR_DONE;
				ph->dwLoops = 0;

				pBlock->Prepared = false;
			}
			universalis::os::aligned_memory_alloc(16, port.pleft, _blockSizeBytes);
			universalis::os::aligned_memory_alloc(16, port.pright, _blockSizeBytes);
			ZeroMemory(port.pleft, 2*_blockSizeBytes);
			ZeroMemory(port.pright, 2*_blockSizeBytes);
			port._machinepos=0;

			waveInStart(port._handle);

			return true;
		}

		void WaveOut::GetReadBuffers(int idx,float **pleft, float **pright,int numsamples)
		{
			if (!_running || idx >= _portMapping.size() || _portMapping[idx] == -1 
				|| _capPorts[_portMapping[idx]]._handle == NULL)
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

		void WaveOut::PollerThread(void * pWaveOut)
		{
			universalis::os::thread_name thread_name("mme wave out");
			universalis::cpu::exceptions::install_handler_in_thread();
			WaveOut * pThis = (WaveOut*) pWaveOut;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			while(!pThis->_stopPolling)
			{
				CBlock *pb = pThis->_blocks + pThis->_currentBlock;
				int underruns=0;
				while(pb->pHeader->dwFlags & WHDR_DONE)
				{
					for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
					{
						if(pThis->_capPorts[i]._handle == NULL)
							continue;
						pThis->DoBlocksRecording(pThis->_capPorts[i]);
					}
					pThis->DoBlocks();
					++pb;
					if(pb == pThis->_blocks + pThis->_numBlocks) pb = pThis->_blocks;
					if ( pb->pHeader->dwFlags & WHDR_DONE)
					{
						underruns++;
						if ( underruns > pThis->_numBlocks )
						{
							// Audio dropout most likely happened
							// (There's a possibility a dropout didn't happen, but the cpu usage
							// is almost at 100%, so we force an exit of the loop for a "Sleep()" call,
							// preventing psycle from being frozen.
							break;
						}
					}
					pThis->_currentBlock = pb - pThis->_blocks;
				}
				::Sleep(pThis->_pollSleep);
			}
			_event.SetEvent();
			::_endthread();
		}
		void WaveOut::DoBlocksRecording(PortCapt& port)
		{
			CBlock *pb = port._blocks + _currentBlock;

			if(pb->Prepared)
			{
				if(::waveInUnprepareHeader(port._handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
				{
					Error("waveInUnprepareHeader() failed");
				}
				pb->Prepared = false;
			}

			WAVEHDR *ph = pb->pHeader;
			ph->lpData = (char *)pb->pData;
			ph->dwBufferLength = _blockSizeBytes;
			ph->dwFlags = WHDR_DONE;
			ph->dwLoops = 0;

			if(::waveInPrepareHeader(port._handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				Error("waveInPrepareHeader() failed");
			}
			pb->Prepared = true;
			waveInAddBuffer(port._handle, pb->pHeader, sizeof(WAVEHDR));
			// Put the audio in our float buffers.
			int numSamples = _blockSizeBytes / GetSampleSizeBytes();
			if (_sampleValidBits == 32) {
				DeinterlaceFloat(reinterpret_cast<float*>(pb->pData), port.pleft,port.pright, numSamples);
			}
			else if (_sampleValidBits == 24) {
				DeQuantize32AndDeinterlace(reinterpret_cast<int*>(pb->pData), port.pleft,port.pright, numSamples);
			}
			else {
				DeQuantize16AndDeinterlace(reinterpret_cast<short int*>(pb->pData), port.pleft,port.pright, numSamples);
			}
			port._machinepos=0;
		}

		void WaveOut::DoBlocks()
		{
			CBlock *pb = _blocks + _currentBlock;
			if(pb->Prepared)
			{
				while (::waveOutUnprepareHeader(_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
				{
				}
				pb->Prepared = false;
			}
			int *pBlock = (int *)pb->pData;
			int numSamples = GetBufferSamples();
			float * pFloatBlock = _pCallback(_callbackContext,numSamples);
			if(_sampleValidBits == 32) {
				dsp::MovMul(pFloatBlock, reinterpret_cast<float*>(pBlock), numSamples*2, 1.f/32768.f);
			}
			else if(_sampleValidBits == 24) {
				Quantize24in32Bit(pFloatBlock, pBlock, numSamples);
			}
			else if (_sampleValidBits == 16) {
				if(_dither) Quantize16WithDither(pFloatBlock, pBlock, numSamples);
				else Quantize16(pFloatBlock, pBlock, numSamples);
			}

			_writePos += numSamples;

			pb->pHeader->lpData = (char *)pb->pData;
			pb->pHeader->dwBufferLength = _blockSizeBytes;
			pb->pHeader->dwFlags = 0;
			pb->pHeader->dwLoops = 0;

			if(::waveOutPrepareHeader(_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				Error("waveOutPrepareHeader() failed");
			}
			pb->Prepared = true;

			if(::waveOutWrite(_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				Error("waveOutWrite() failed");
			}
		}

		void WaveOut::ReadConfig()
		{
			// Default configuration
			bool saveatend(false);
			_samplesPerSec=44100;
			_deviceID=0;
			_numBlocks = 6;
			_blockSizeBytes = 4096;
			_pollSleep = 10;
			_dither = 0;
			_channelMode = stereo;
			_sampleBits = 16;
			_sampleValidBits = 16;

			Registry reg;
			reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT);
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\mme") != ERROR_SUCCESS) // settings in version 1.8
			{
				reg.CloseRootKey();
				reg.OpenRootKey(HKEY_CURRENT_USER,PSYCLE__PATH__REGISTRY__ROOT "--1.7"); // settings in version 1.7 alpha
				if(reg.OpenKey("configuration\\devices\\mme") != ERROR_SUCCESS)
				{
					reg.CloseRootKey();
					reg.OpenRootKey(HKEY_CURRENT_USER,"Software\\AAS\\Psycle\\CurrentVersion");
					if(reg.OpenKey("WaveOut") != ERROR_SUCCESS)
					{
						reg.CloseRootKey();
						return;
					}
				}
				saveatend=true;
			}
			bool configured(true);
			configured &= ERROR_SUCCESS == reg.QueryValue("NumBlocks", _numBlocks);
			configured &= ERROR_SUCCESS == reg.QueryValue("BlockSize", _blockSizeBytes);
			configured &= ERROR_SUCCESS == reg.QueryValue("DeviceID", _deviceID);
			configured &= ERROR_SUCCESS == reg.QueryValue("PollSleep", _pollSleep);
			configured &= ERROR_SUCCESS == reg.QueryValue("Dither", _dither);
			configured &= ERROR_SUCCESS == reg.QueryValue("bitDepth", _sampleValidBits);
			if (configured) { if(_sampleValidBits == 24) {_sampleBits = 32; }
				else {_sampleBits = _sampleValidBits; }
			}
			configured &= ERROR_SUCCESS == reg.QueryValue("SamplesPerSec", _samplesPerSec);
			reg.CloseKey();
			reg.CloseRootKey();
			_configured = configured;
			if(saveatend) WriteConfig();
		}

		void WaveOut::WriteConfig()
		{
			Registry reg;
			if(reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT) != ERROR_SUCCESS)
			{
				Error("Unable to write configuration to the registry");
				return;
			}
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\mme") != ERROR_SUCCESS)
			{
				if (reg.CreateKey(PSYCLE__PATH__REGISTRY__CONFIGKEY "\\devices\\mme") != ERROR_SUCCESS)
				{
					Error("Unable to write configuration to the registry");
					return;
				}
			}
			reg.SetValue("NumBlocks", _numBlocks);
			reg.SetValue("BlockSize", _blockSizeBytes);
			reg.SetValue("DeviceID", _deviceID);
			reg.SetValue("PollSleep", _pollSleep);
			reg.SetValue("Dither", _dither);
			reg.SetValue("bitDepth", _sampleValidBits);
			reg.SetValue("SamplesPerSec", _samplesPerSec);
			reg.CloseKey();
			reg.CloseRootKey();
		}

		void WaveOut::Configure()
		{
			if(!_configured) ReadConfig();

			CWaveOutDialog dlg;
			dlg.m_device = _deviceID;
			dlg.m_sampleRate = _samplesPerSec;
			dlg.m_bitDepth = _sampleValidBits;
			dlg.m_dither = _dither;
			dlg.m_bufNum = _numBlocks;
			dlg.m_bufSamples = GetBufferSamples();
			dlg.waveout = this;

			if(dlg.DoModal() != IDOK) return;

			WAVEFORMATPCMEX wf;
			PrepareWaveFormat(wf, 2, dlg.m_sampleRate, (dlg.m_bitDepth==24)?32:dlg.m_bitDepth, dlg.m_bitDepth);
			bool supported = _playEnums[dlg.m_device].IsFormatSupported(wf,false);
			if(!supported) {
				Error("The Format selected is not supported. Keeping the previous configuration");
				return;
			}

			_deviceID = dlg.m_device;
			_samplesPerSec = dlg.m_sampleRate;
			_sampleValidBits = dlg.m_bitDepth;
			if(_sampleValidBits == 24) _sampleBits = 32;
			else _sampleBits = _sampleValidBits;
			_dither = dlg.m_dither;
			_numBlocks = dlg.m_bufNum;
			_blockSizeBytes = dlg.m_bufSamples * GetSampleSizeBytes();

			_configured = true;
			WriteConfig();
		}

		std::uint32_t WaveOut::GetPlayPosInSamples()
		{
			// WARNING! waveOutGetPosition in TIME_SAMPLES has max of 0x7FFFFF for 16bit stereo signals.
			if(!_stopPolling) return 0;
			MMTIME time;
			time.wType = TIME_SAMPLES;
			if(::waveOutGetPosition(_handle, &time, sizeof(MMTIME)) != MMSYSERR_NOERROR)
			{
				Error("waveOutGetPosition() failed");
			}
			if(time.wType != TIME_SAMPLES)
			{
				Error("waveOutGetPosition() doesn't support TIME_SAMPLES");
			}
			
			std::uint32_t retval = time.u.sample;
			// sample counter wrap around?
			if( m_lastPlayPos > retval)
			{
				m_readPosWraps++;
				if(m_lastPlayPos > _writePos) {
					m_readPosWraps = 0;
					CMidiInput::Instance()->ReSync();	// MIDI IMPLEMENTATION
				}
			}
			m_lastPlayPos = retval;
			return retval + (m_readPosWraps*0x800000);
		}

		std::uint32_t WaveOut::GetWritePosInSamples()
		{
			if(!_stopPolling) return 0;
			return _writePos;
		}

		bool WaveOut::Enable(bool e)
		{
			return e ? Start() : Stop();
		}

		bool WaveOut::PortEnums::IsFormatSupported(WAVEFORMATEXTENSIBLE& pwfx, bool isInput) 
		{ 
			if(isInput) {
				return MMSYSERR_NOERROR == waveInOpen(
					NULL,                 // ptr can be NULL for query 
					idx,            // the device identifier 
					reinterpret_cast<WAVEFORMATEX*>(&pwfx),                 // defines requested format 
					NULL,                 // no callback 
					NULL,                 // no instance data 
					WAVE_FORMAT_QUERY);  // query only, do not open device 
			}
			else {
				return MMSYSERR_NOERROR == waveOutOpen( 
					NULL,                 // ptr can be NULL for query 
					idx,            // the device identifier 
					reinterpret_cast<WAVEFORMATEX*>(&pwfx),                 // defines requested format 
					NULL,                 // no callback 
					NULL,                 // no instance data 
					WAVE_FORMAT_QUERY);  // query only, do not open device 
			}
		} 
	}
}
