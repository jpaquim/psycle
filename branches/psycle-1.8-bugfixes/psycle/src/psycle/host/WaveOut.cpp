///\file
///\brief implementation file for psycle::host::WaveOut.
#include <psycle/project.private.hpp>
#include "WaveOut.hpp"
#include "resources/resources.hpp"
#include "WaveOutDialog.hpp"
#include "Registry.hpp"
#include "Configuration.hpp"
#include "MidiInput.hpp"
#include <universalis/processor/exception.hpp>
#include <process.h>
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
		void WaveOut::EnumerateCapturePorts()
		{
			for (unsigned int i=0; i < waveInGetNumDevs(); ++i)
			{
				WAVEINCAPS caps;
				waveInGetDevCaps(i,&caps,sizeof(caps));
				PortEnums port(caps.szPname);
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

			WAVEFORMATEX format;
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.wBitsPerSample = 16;//_bitDepth;
			format.nSamplesPerSec = _samplesPerSec;
			format.cbSize = 0;
			format.nChannels = 2;

			format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

			if(::waveOutOpen(&_handle, _deviceID, &format, 0, 0, 0) != MMSYSERR_NOERROR)
			{
				Error("waveOutOpen() failed");
				return false;
			}

			_currentBlock = 0;
			_writePos = 0;

			// allocate blocks
			for(CBlock *pBlock = _blocks; pBlock < _blocks + _numBlocks; pBlock++)
			{
				pBlock->Handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, _blockSize);
				pBlock->pData = (byte *)GlobalLock(pBlock->Handle);
			}

			// allocate block headers
			for(CBlock *pBlock = _blocks; pBlock < _blocks + _numBlocks; pBlock++)
			{
				pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
				pBlock->pHeader = (WAVEHDR *)GlobalLock(pBlock->HeaderHandle);

				WAVEHDR *ph = pBlock->pHeader;
				ph->lpData = (char *)pBlock->pData;
				ph->dwBufferLength = _blockSize;
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
				if(::waveInReset(_capPorts[i]._handle) != MMSYSERR_NOERROR)
				{
					Error("waveInReset() failed");
					return false;
				}
				///\todo: wait until WHDR_DONE like with waveout?
				waveInClose(_capPorts[i]._handle);
				delete[] _capPorts[i].pleft;
				delete[] _capPorts[i].pright;
			}
			_capPorts.resize(0);

			_running = false;
			return true;
		}
		void WaveOut::GetCapturePorts(std::vector<std::string>&ports)
		{
			for (unsigned int i=0;i<_capEnums.size();i++) ports.push_back(_capEnums[i].portname);
		}
		bool WaveOut::AddCapturePort(int idx)
		{
			bool isplaying = _running;
			for (unsigned int i=0;i<_capPorts.size();++i)
			{
				if (_capPorts[i]._idx == idx ) return false;
			}
			PortCapt port;
			port._idx = idx;
			if (isplaying)
			{
				Stop();
			}
			_capPorts.push_back(port);
			_portMapping[idx]=_capPorts.size()-1;
			if (isplaying)
			{
				return Start();
			}
			return true;
		}
		bool WaveOut::RemoveCapturePort(int idx)
		{
			bool restartplayback = false;
			std::vector<PortCapt> newports;
			for (unsigned int i=0;i<_capPorts.size();++i)
			{
				if (_capPorts[i]._idx == idx )
				{
					if (_running)
					{
						Stop();
						restartplayback=true;
					}
				}
				else newports.push_back(_capPorts[i]);
			}
			_capPorts = newports;
			if (restartplayback) Start();
			return true;
		}
		bool WaveOut::CreateCapturePort(PortCapt &port)
		{
			HRESULT hr;
			//not try to open a port twice
			if (port._handle) return true;

			WAVEFORMATEX format;
			// Set up wave format structure. 
			ZeroMemory(&format, sizeof(WAVEFORMATEX));
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.nChannels = 2;
			format.wBitsPerSample = 16;//_bitDepth;
			format.nSamplesPerSec = _samplesPerSec;
			format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
			format.cbSize = 0;

			if ((hr = waveInOpen(&port._handle,port._idx,&format,NULL,NULL,CALLBACK_NULL)) != MMSYSERR_NOERROR )
			{
				Error(_T("Failed to create Capture DirectSound Device"));
				return false;
			}

/*			waveInAddBuffer(port._handle
				LPWAVEHDR pwh,  
				UINT cbwh       
				);
*/
			waveInStart(port._handle);

			port.pleft = new float[_blockSize];
			port.pright = new float[_blockSize];
			return true;
		}

		void WaveOut::GetReadBuffers(int idx,float **pleft, float **pright,int numsamples)
		{
			if  (_running)
			{
				*pleft=_capPorts[_portMapping[idx]].pleft+_capPorts[_portMapping[idx]]._machinepos;
				*pright=_capPorts[_portMapping[idx]].pright+_capPorts[_portMapping[idx]]._machinepos;
				_capPorts[_portMapping[idx]]._machinepos+=numsamples;
			}
		}

		void WaveOut::PollerThread(void * pWaveOut)
		{
			universalis::processor::exception::new_thread("mme wave out");
			WaveOut * pThis = (WaveOut*) pWaveOut;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			SetThreadAffinityMask(GetCurrentThread(), 1);
			while(!pThis->_stopPolling)
			{
				for (unsigned int i =0; i < pThis->_capPorts.size(); i++)
				{
					pThis->DoBlocksRecording(pThis->_capPorts[i]);
				}
				pThis->DoBlocks();
				::Sleep(pThis->_pollSleep);
			}
			_event.SetEvent();
			::_endthread();
		}
		void WaveOut::DoBlocksRecording(PortCapt& port)
		{
/*
			PortCapt *pb = _capPorts[_currentBlock];
			int underruns=0;
			while(pb->pHeader->dwFlags & WHDR_DONE)
			{
				if(pb->Prepared)
				{
					if(::waveInUnprepareHeader(_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
					{
						Error("waveOutUnprepareHeader() failed");
					}
					pb->Prepared = false;
				}
				int *pOut = (int *)pb->pData;
				int bs = _blockSize / GetSampleSize();
				float * pBuf = _pCallback(_callbackContext,bs);
				if(_dither) QuantizeWithDither(pBuf, pOut,bs);
				else Quantize(pBuf, pOut,bs);

				_writePos += bs;

				pb->pHeader->lpData = (char *)pb->pData;
				pb->pHeader->dwBufferLength = _blockSize;
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
				++pb;
				if(pb == _blocks + _numBlocks) pb = _blocks;
				if ( pb->pHeader->dwFlags & WHDR_DONE)
				{
					underruns++;
					if ( underruns > _numBlocks )
					{
						// Audio dropout most likely happened
						// (There's a possibility a dropout didn't happen, but the cpu usage
						// is almost at 100%, so we force an exit of the loop for a "Sleep()" call,
						// preventing psycle from being frozen.
						break;
					}
				}
			}
			_currentBlock = pb - _blocks;

			DeQuantizeAndDeinterlace(pBlock2, port.pleft+numSamples,port.pright+numSamples, blockSize2 / GetSampleSize());
			port._machinepos=0;
*/
		}

		void WaveOut::DoBlocks()
		{
			CBlock *pb = _blocks + _currentBlock;
			int underruns=0;
			while(pb->pHeader->dwFlags & WHDR_DONE)
			{
				if(pb->Prepared)
				{
					if(::waveOutUnprepareHeader(_handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
					{
						Error("waveOutUnprepareHeader() failed");
					}
					pb->Prepared = false;
				}
				int *pOut = (int *)pb->pData;
				int bs = _blockSize / GetSampleSize();
				float * pBuf = _pCallback(_callbackContext,bs);
				if(_dither) QuantizeWithDither(pBuf, pOut,bs);
				else Quantize(pBuf, pOut,bs);

				_writePos += bs;

				pb->pHeader->lpData = (char *)pb->pData;
				pb->pHeader->dwBufferLength = _blockSize;
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
				++pb;
				if(pb == _blocks + _numBlocks) pb = _blocks;
				if ( pb->pHeader->dwFlags & WHDR_DONE)
				{
					underruns++;
					if ( underruns > _numBlocks )
					{
						// Audio dropout most likely happened
						// (There's a possibility a dropout didn't happen, but the cpu usage
						// is almost at 100%, so we force an exit of the loop for a "Sleep()" call,
						// preventing psycle from being frozen.
						break;
					}
				}
			}
			_currentBlock = pb - _blocks;
		}

		void WaveOut::ReadConfig()
		{
			// Default configuration
			bool saveatend(false);
			_samplesPerSec=44100;
			_deviceID=0;
			_numBlocks = 7;
			_blockSize = 4096;
			_pollSleep = 10;
			_dither = 0;
			_channelmode = 3;
			_bitDepth = 16;

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
			configured &= ERROR_SUCCESS == reg.QueryValue("BlockSize", _blockSize);
			configured &= ERROR_SUCCESS == reg.QueryValue("DeviceID", _deviceID);
			configured &= ERROR_SUCCESS == reg.QueryValue("PollSleep", _pollSleep);
			configured &= ERROR_SUCCESS == reg.QueryValue("Dither", _dither);
			configured &= ERROR_SUCCESS == reg.QueryValue("SamplesPerSec", _samplesPerSec);
			//configured &= ERROR_SUCCESS == reg.QueryValue("BitDepth", _bitDepth);
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
			reg.SetValue("BlockSize", _blockSize);
			reg.SetValue("DeviceID", _deviceID);
			reg.SetValue("PollSleep", _pollSleep);
			reg.SetValue("Dither", _dither);
			reg.SetValue("SamplesPerSec", _samplesPerSec);
			//reg.SetValue("BitDepth", _bitDepth);
			reg.CloseKey();
			reg.CloseRootKey();
		}

		void WaveOut::Configure()
		{
			ReadConfig();

			CWaveOutDialog dlg;
			dlg.m_BufNum = _numBlocks;
			dlg.m_BufSize = _blockSize;
			dlg.m_Device = _deviceID;
			dlg.m_Dither = _dither;
			dlg.m_SampleRate = _samplesPerSec;

			if(dlg.DoModal() != IDOK) return;
			
			int oldnb = _numBlocks;
			int oldbs = _blockSize;
			int olddid = _deviceID;
			int olddither = _dither;
			int oldsps = _samplesPerSec;

			if(_initialized) Stop();

			_numBlocks = dlg.m_BufNum;
			_blockSize = dlg.m_BufSize;
			_deviceID = dlg.m_Device;
			_dither = dlg.m_Dither;
			_samplesPerSec = dlg.m_SampleRate;

			_configured = true;

			if(_initialized)
			{
				if(Start()) WriteConfig();
				else
				{
					_numBlocks = oldnb;
					_blockSize = oldbs;
					_deviceID = olddid;
					_dither = olddither;
					_samplesPerSec = oldsps;
					Start();
				}
			}
			else WriteConfig();
		}

		int WaveOut::GetPlayPos()
		{
			if(!_running) return 0;
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
			return time.u.sample & ((1 << 23) - 1);
		}

		int WaveOut::GetWritePos()
		{
			if(!_running) return 0;
			return _writePos & ((1 << 23) - 1);
		}

		bool WaveOut::Enable(bool e)
		{
			return e ? Start() : Stop();
		}
		MMRESULT WaveOut::IsFormatSupported(LPWAVEFORMATEX pwfx, UINT uDeviceID) 
		{ 
			return (waveOutOpen( 
				NULL,                 // ptr can be NULL for query 
				uDeviceID,            // the device identifier 
				pwfx,                 // defines requested format 
				NULL,                 // no callback 
				NULL,                 // no instance data 
				WAVE_FORMAT_QUERY));  // query only, do not open device 
		} 
	}
}
