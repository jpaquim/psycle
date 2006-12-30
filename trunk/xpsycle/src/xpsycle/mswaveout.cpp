/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper and others  *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#if defined _WIN64 || defined _WIN32 || defined __CYGWIN__ || defined __MSYS__ || defined _UWIN

#include "mswaveout.h"
#include "cstdint.h"
#include "helpers.h"
#include <iostream>


namespace psycle
{
	namespace host
	{
        int const SHORT_MIN = -32768;
		int const SHORT_MAX =  32767;


        AudioDriverInfo MsWaveOut::info( ) const
        {
            return AudioDriverInfo("mswaveout","Microsoft WaveOut Driver","Microsoft standard output driver",true);
        }      
              
//		AudioDriverEvent MsWaveOut::_event;
//		CCriticalSection MsWaveOut::_lock;

		void MsWaveOut::Error(char const msg[])
		{
			MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
		}

		MsWaveOut::MsWaveOut()
			: _initialized(false)
			, _configured(false)
			, _running(false)
			, _pCallback(0)
		{
		}
		
		MsWaveOut * MsWaveOut::clone( ) const
        {
          return new MsWaveOut(*this);
        }

		void MsWaveOut::Initialize( AUDIODRIVERWORKFN pCallback, void * context )
		{
			_callbackContext = context;
			_pCallback = pCallback;
			_running = false;
			ReadConfig();
			_initialized = true;
		}
		
		
        bool MsWaveOut::Initialized( )
        {
           return _initialized;
        }
        
        bool MsWaveOut::Configured() {
           return _configured;
        }
        
        int MsWaveOut::GetMaxLatencyInSamples() { 
           return settings().sampleSize() * settings().blockSize() * _numBlocks; 
        }

		void MsWaveOut::Reset()
		{
			if (_running) Stop();
		}

		MsWaveOut::~MsWaveOut()
		{
			if(_initialized) Reset();
		}

		bool MsWaveOut::Start()
		{
//			CSingleLock lock(&_lock, true);
			if(_running) return true;
			if(!_pCallback) return false;

			WAVEFORMATEX format;
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.wBitsPerSample = settings().bitDepth();
			format.nSamplesPerSec = settings().samplesPerSec();
			format.cbSize = 0;
			format.nChannels = 2;
 
			_deviceID=0;
			_numBlocks = 7;
			_pollSleep = 20;
			_dither = 0;


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
				pBlock->Handle = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, settings().blockSize() );
				pBlock->pData = reinterpret_cast<unsigned char *>(::GlobalLock(pBlock->Handle));
			}

			// allocate block headers
			for(CBlock *pBlock = _blocks; pBlock < _blocks + _numBlocks; pBlock++)
			{
				pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
				pBlock->pHeader = (WAVEHDR *)GlobalLock(pBlock->HeaderHandle);

				WAVEHDR *ph = pBlock->pHeader;
				ph->lpData = (char *)pBlock->pData;
				ph->dwBufferLength = settings().blockSize();
				ph->dwFlags = WHDR_DONE;
				ph->dwLoops = 0;

				pBlock->Prepared = false;
			}

			_stopPolling = false;

            DWORD dwThreadId;
            ::CreateThread( NULL, 0, PollerThread, this, 0, &dwThreadId );
			_running = true;

			return true;
		}

		DWORD MsWaveOut::PollerThread(void * pWaveOut)
		{
//			universalis::processor::exception::new_thread("mme wave out");
			MsWaveOut * pThis = (MsWaveOut*) pWaveOut;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			while(!pThis->_stopPolling)
			{
				pThis->DoBlocks();
				::Sleep(pThis->_pollSleep);
			}
//			_event.SetEvent();
//			TerminateThread ::_endthread();
			return 0;
		}

		bool MsWaveOut::Stop()
		{
//			CSingleLock lock(&_lock, true);
			if(!_running) return true;
			_stopPolling = true;
	//		CSingleLock event(&_event, true);
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
			TerminateThread( hThread_, 0 );
			return true;
		}

		void MsWaveOut::DoBlocks()
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
				int bs = settings().blockSize() / settings().sampleSize();
				do
				{
					int n = bs;
					float * pBuf = _pCallback(_callbackContext, n);
				//	if(_dither)
                //      quantizeWithDither(pBuf, pOut, n); 
                //    else 
                      quantize(pBuf, pOut, n);
					pOut += n;
					bs -= n;
				}
				while(bs > 0);

				_writePos += settings().blockSize() / settings().sampleSize();

				pb->pHeader->dwFlags = 0;
				pb->pHeader->lpData = (char *)pb->pData;
				pb->pHeader->dwBufferLength = settings().sampleSize();
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

		void MsWaveOut::ReadConfig()
		{
			// Default configuration
			bool saveatend(false);
			
			setSettings( AudioDriverSettings() );				
   		 
   			_deviceID=0;
   			_numBlocks = 7;
  
   			_pollSleep = 20;
   			_dither = 0;
			_configured = true;

		}

		void MsWaveOut::WriteConfig()
		{
		}

		void MsWaveOut::Configure()
		{
			ReadConfig();

			if( _initialized ) Stop();

			_configured = true;

			if( _initialized ) Start();
		}

		int MsWaveOut::GetPlayPos()
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

		int MsWaveOut::GetWritePos()
		{
			if(!_running) return 0;
			return _writePos & ((1 << 23) - 1);
		}

		bool MsWaveOut::Enable(bool e)
		{
			return e ? Start() : Stop();
		}


		void MsWaveOut::quantizeWithDither(float *pin, int *piout, int c)
		{
			///\todo sizeof *piout has to be 32-bit

	/*		double const d2i = (1.5 * (1 << 26) * (1 << 26));
			
			do
			{
				double res = ((double)pin[1] + frand()) + d2i;

				int r = *(int *)&res;

				if (r < SHORT_MIN)
				{
					r = SHORT_MIN;
				}
				else if (r > SHORT_MAX)
				{
					r = SHORT_MAX;
				}

				res = ((double)pin[0] + frand()) + d2i;

				int l = *(int *)&res;

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
			while(--c);*/
		}

		void MsWaveOut::quantize(float *pin, int *piout, int c)
		{
			do
			{
				int r = f2i( (pin[1]) );
				
				if (r < SHORT_MIN)
				{
					r = SHORT_MIN;
				}
				else if (r > SHORT_MAX)
				{
					r = SHORT_MAX;
				}

				int l = f2i( (pin[0]) );

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

#endif // windows platform
