///\file
///\brief implementation file for psycle::host::WaveOut.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/audiodrivers/WaveOut.hpp>
#include <psycle/host/resources/resources.hpp>
#include <psycle/host/audiodrivers/WaveOutDialog.hpp>
#include <psycle/engine/registry.hpp>
//\todo:
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/engine/MidiInput.hpp>
#include <universalis/processor/exception.hpp>
#include <process.h>
namespace psycle
{
	namespace host
	{
		AudioDriverInfo WaveOut::_info = { "Windows WaveOut MME" };
		AudioDriverEvent WaveOut::_event;
		CCriticalSection WaveOut::_lock;

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
		}

		void WaveOut::Initialize(HWND hwnd, WorkFunction pCallback, void * context)
		{
			_callbackContext = context;
			_pCallback = pCallback;
			_running = false;
			ReadConfig();
			_initialized = true;
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
			CSingleLock lock(&_lock, true);
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
				pBlock->Handle = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, _blockSize);
				pBlock->pData = reinterpret_cast<unsigned char *>(::GlobalLock(pBlock->Handle));
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

			_stopPolling = false;
			_event.ResetEvent();
			::_beginthread(PollerThread, 0, this);
			_running = true;
			CMidiInput::Instance()->ReSync();	// MIDI IMPLEMENTATION
			return true;
		}

		void WaveOut::PollerThread(void * pWaveOut)
		{
			universalis::processor::exception::new_thread("mme wave out");
			WaveOut * pThis = (WaveOut*) pWaveOut;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			while(!pThis->_stopPolling)
			{
				pThis->DoBlocks();
				::Sleep(pThis->_pollSleep);
			}
			_event.SetEvent();
			::_endthread();
		}

		bool WaveOut::Stop()
		{
			CSingleLock lock(&_lock, true);
			if(!_running) return true;
			_stopPolling = true;
			CSingleLock event(&_event, true);
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
				::Sleep(20);
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
			_running = false;
			return true;
		}

		void WaveOut::DoBlocks()
		{
			CBlock *pb = _blocks + _currentBlock;
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
				do
				{
					int n = bs;
					float * pBuf = _pCallback(_callbackContext, n);
					if(_dither) QuantizeWithDither(pBuf, pOut, n); else Quantize(pBuf, pOut, n);
					pOut += n;
					bs -= n;
				}
				while(bs > 0);

				_writePos += _blockSize / GetSampleSize();

				pb->pHeader->dwFlags = 0;
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
			_pollSleep = 20;
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
	}
}
