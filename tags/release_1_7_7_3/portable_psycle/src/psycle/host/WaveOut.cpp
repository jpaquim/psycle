#include "stdafx.h"
#include "WaveOut.h"
#pragma comment(lib, "winmm")
#include "resource.h"
#include "WaveOutDialog.h"
#include "Registry.h"
#include "Configuration.h"
#include "MidiInput.h"
#include <process.h>
#include <operating_system/exception.h>
///\file
///\brief implementation file for psycle::host::WaveOut.
namespace psycle
{
	namespace host
	{
		#define BYTES_PER_SAMPLE 4	// 2 * 16bits

		AudioDriverInfo WaveOut::_info = { "Windows WaveOut MME" };
		AudioDriverEvent WaveOut::_event;
		CCriticalSection WaveOut::_lock;

		void WaveOut::Error(char const msg[])
		{
			::MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
		}

		WaveOut::WaveOut()
			: _initialized(false)
			, _configured(false)
			, _running(false)
			, _pCallback(0)
		{
		}

		void WaveOut::Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void * context)
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
			CSingleLock lock(&_lock, TRUE);
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

			_stopPolling = false;
			_event.ResetEvent();
			::_beginthread(PollerThread, 0, this);
			_running = true;
			CMidiInput::Instance()->ReSync();	// MIDI IMPLEMENTATION
			return true;
		}

		void WaveOut::PollerThread(void * pWaveOut)
		{
			operating_system::exceptions::translated::new_thread("mme wave out");
			WaveOut * pThis = (WaveOut*) pWaveOut;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
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
			CSingleLock lock(&_lock, TRUE);
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
				int bs = _blockSize / BYTES_PER_SAMPLE;
				do
				{
					int n = bs;
					float * pBuf = _pCallback(_callbackContext, n);
					if(_dither) QuantizeWithDither(pBuf, pOut, n); else Quantize(pBuf, pOut, n);
					pOut += n;
					bs -= n;
				}
				while(bs > 0);

				_writePos += _blockSize / BYTES_PER_SAMPLE;

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
			bool configured;
			DWORD type;
			DWORD numData;
			Registry reg;

			// Default configuration
			_samplesPerSec=44100;
			_deviceID=0;
			_numBlocks = 7;
			_blockSize = 4096;
			_pollSleep = 20;
			_dither = 0;
			_channelmode = 3;
			_bitDepth = 16;

			if(reg.OpenRootKey(HKEY_CURRENT_USER, CONFIG_ROOT_KEY) != ERROR_SUCCESS)
			{
				return;
			}
			if(reg.OpenKey("WaveOut") != ERROR_SUCCESS)
			{
				return;
			}
			configured = true;
			numData = sizeof _numBlocks;
			configured &= (reg.QueryValue("NumBlocks", &type, (BYTE*)&_numBlocks, &numData) == ERROR_SUCCESS);
			numData = sizeof _blockSize;
			configured &= (reg.QueryValue("BlockSize", &type, (BYTE*)&_blockSize, &numData) == ERROR_SUCCESS);
			numData = sizeof _deviceID;
			configured &= (reg.QueryValue("DeviceID", &type, (BYTE*)&_deviceID, &numData) == ERROR_SUCCESS);
			numData = sizeof _pollSleep;
			configured &= (reg.QueryValue("PollSleep", &type, (BYTE*)&_pollSleep, &numData) == ERROR_SUCCESS);
			numData = sizeof _dither;
			configured &= (reg.QueryValue("Dither", &type, (BYTE*)&_dither, &numData) == ERROR_SUCCESS);
			numData = sizeof _samplesPerSec;
			configured &= (reg.QueryValue("SamplesPerSec", &type, (BYTE*)&_samplesPerSec, &numData) == ERROR_SUCCESS);
			//numData = sizeof _bitDepth;
			//(reg.QueryValue("BitDepth", &type, (BYTE*)&_bitDepth, &numData) == ERROR_SUCCESS);
			reg.CloseKey();
			reg.CloseRootKey();
			_configured = configured;
		}

		void WaveOut::WriteConfig()
		{
			Registry reg;
			if(reg.OpenRootKey(HKEY_CURRENT_USER, CONFIG_ROOT_KEY) != ERROR_SUCCESS)
			{
				Error("Unable to write configuration to the registry");
				return;
			}
			if(reg.OpenKey("WaveOut") != ERROR_SUCCESS)
			{
				if (reg.CreateKey("WaveOut") != ERROR_SUCCESS)
				{
					Error("Unable to write configuration to the registry");
					return;
				}
			}
			reg.SetValue("NumBlocks", REG_DWORD, (BYTE*)&_numBlocks, sizeof _numBlocks);
			reg.SetValue("BlockSize", REG_DWORD, (BYTE*)&_blockSize, sizeof _blockSize);
			reg.SetValue("DeviceID", REG_DWORD, (BYTE*)&_deviceID, sizeof _deviceID);
			reg.SetValue("PollSleep", REG_DWORD, (BYTE*)&_pollSleep, sizeof _pollSleep);
			reg.SetValue("Dither", REG_DWORD, (BYTE*)&_dither, sizeof _dither);
			reg.SetValue("SamplesPerSec", REG_DWORD, (BYTE*)&_samplesPerSec, sizeof _samplesPerSec);
			//reg.SetValue("BitDepth", REG_DWORD, (BYTE*)&_bitDepth, sizeof _bitDepth);
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
