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
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
#include "microsoftmmewaveout.h"
namespace psy { namespace core {
		int const SHORT_MIN = -32768;
		int const SHORT_MAX =  32767;

		bool MsWaveOut::_running = 0;
		CRITICAL_SECTION MsWaveOut::waveCriticalSection;
		WAVEHDR*         MsWaveOut::waveBlocks;
		volatile int     MsWaveOut::waveFreeBlockCount;
		int              MsWaveOut::waveCurrentBlock;

		AudioDriverInfo MsWaveOut::info( ) const {
			return AudioDriverInfo("mmewaveout","Microsoft MME WaveOut Driver","Microsoft legacy output driver",true);
		}      

		MsWaveOut::MsWaveOut() {
			_pCallback = 0;
			hWaveOut = 0; // use this audio handle to detect if the driver is working
			_running = 0; // running condition for the thread loop
			_initialized = 0;
		}

		MsWaveOut::~MsWaveOut() {
			if ( hWaveOut ) stop( );
		}

		void MsWaveOut::Initialize( AUDIODRIVERWORKFN pCallback, void * context )
		{
			_callbackContext = context;
			_pCallback = pCallback;
			_initialized= true;
		}

		bool MsWaveOut::Initialized( )
		{
			return _initialized;
		}

		bool MsWaveOut::Enable( bool e )
		{
			return e ? start() : stop();
		}

		WAVEHDR* MsWaveOut::allocateBlocks( )
		{

			unsigned char* buffer;
			int i;
			WAVEHDR* blocks;
			DWORD totalBufferSize = (settings().blockSize() + sizeof(WAVEHDR)) * settings().blockCount();

			//
			// allocate memory for the entire set in one go
			//
			if((buffer = static_cast<unsigned char*>( HeapAlloc(
				GetProcessHeap(), 
				HEAP_ZERO_MEMORY, 
				totalBufferSize
				))) == NULL) {
					std::cerr << "Memory allocation error\n" << std::endl;
					return 0;
			}
			//
			// and set up the pointers to each bit
			//
			blocks = (WAVEHDR*)buffer;
			buffer += sizeof(WAVEHDR) * settings().blockCount();
			for(i = 0; i < settings().blockCount(); i++) {
				blocks[i].dwBufferLength = settings().blockSize();
				blocks[i].lpData = (CHAR*)( buffer );
				buffer += settings().blockSize();
			}

			return blocks;
		}

		void MsWaveOut::freeBlocks(WAVEHDR* blockArray)
		{
			//
			// and this is why allocateBlocks works the way it does
			// 
			HeapFree( GetProcessHeap(), 0, blockArray );
		}

		void MsWaveOut::writeAudio( HWAVEOUT hWaveOut, LPSTR data, int size )
		{
			WAVEHDR* current;
			int remain;

			current = &waveBlocks[waveCurrentBlock];

			while(size > 0) {
				// 
				// first make sure the header we're going to use is unprepared
				//
				if(current->dwFlags & WHDR_PREPARED) 
					waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));

				if( size < static_cast<int>( settings().blockSize() - current->dwUser) ) {
					memcpy(current->lpData + current->dwUser, data, size);
					current->dwUser += size;
					break;
				}

				remain = settings().blockSize() - current->dwUser;
				memcpy(current->lpData + current->dwUser, data, remain);
				size -= remain;
				data += remain;
				current->dwBufferLength = settings().blockSize();

				waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
				waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));

				EnterCriticalSection(&waveCriticalSection);
				waveFreeBlockCount--;
				LeaveCriticalSection(&waveCriticalSection);

				//
				// wait for a block to become free
				//
				while(!waveFreeBlockCount) {
					Sleep(10);
				}

				//
				// point to the next block
				//
				waveCurrentBlock++;
				waveCurrentBlock %= settings().blockCount();

				current = &waveBlocks[waveCurrentBlock];
				current->dwUser = 0;
			}
		}


		// called by the waveOut interface as different thread!
		// thus we need CriticalSection for the block counter
		void CALLBACK MsWaveOut::waveOutProc(
			HWAVEOUT hWaveOut, 
			UINT uMsg, 
			DWORD dwInstance,  
			DWORD dwParam1,    
			DWORD dwParam2     
			)
		{
			int* freeBlockCounter = (int*)dwInstance;
			//
			// ignore calls that occur due to openining and closing the
			// device.
			//
			if(uMsg != WOM_DONE)
				return;

			EnterCriticalSection(&waveCriticalSection);
			(*freeBlockCounter)++;
			LeaveCriticalSection(&waveCriticalSection);
		}

		void MsWaveOut::fillBuffer() {
			// this protects freeBlockCounter, that is manipulated from two threads.
			// the waveOut interface callback WM_Done thread in waveOutProc and in writeAudio
			InitializeCriticalSection( &waveCriticalSection );
			int bufSize = 1024 / 2;
			std::int16_t buf[1024 / 2];
			int newCount = bufSize / 2;        
			while ( _running ) {
				float const * input(_pCallback(_callbackContext, newCount));
				Quantize16(input,buf,newCount);
				writeAudio(hWaveOut, (CHAR*) buf, sizeof(buf) );
			}
		}

		DWORD WINAPI MsWaveOut::audioOutThread( void *pWaveOut ) {
			SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
			MsWaveOut * pThis = reinterpret_cast<MsWaveOut*>( pWaveOut ) ;
			pThis->fillBuffer();
			return 0;
		}

		bool MsWaveOut::start()
		{
			if ( hWaveOut ) return true;   // do not start again
			if(!_pCallback) return false;  // no player callback
			// WAVEFORMATEX is defined in mmsystem.h
			// this structure is used, to define the sampling rate, sampling resolution
			// and the number of channles
			// some informations are redundant, but you need to define them
			WAVEFORMATEX format;
			format.wFormatTag = WAVE_FORMAT_PCM;
			format.wBitsPerSample = settings().bitDepth();
			format.nSamplesPerSec = settings().samplesPerSec();
			format.nChannels = 2;
			format.nBlockAlign = ( format.nChannels * format.wBitsPerSample ) >> 3;
			format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
			format.cbSize = 0;

			_dither = 0;

			// the buffer block variables
			if (!(waveBlocks = allocateBlocks())) return 0; // memory error
			waveFreeBlockCount =  settings().blockCount();
			waveCurrentBlock   = 0;

			// this will protect the monitor buffer counter variable 

			if( waveOutOpen( &hWaveOut, WAVE_MAPPER, &format, (DWORD_PTR)waveOutProc, 
				(DWORD_PTR)&waveFreeBlockCount, 
				CALLBACK_FUNCTION ) != MMSYSERR_NOERROR)
			{
				std::cerr << "waveOutOpen() failed" << std::endl;
				return false;
			}

			_running = true;
			DWORD dwThreadId;
			_hThread = CreateThread( NULL, 0, audioOutThread, this, 0, &dwThreadId );
			return true;
		}

		bool MsWaveOut::stop()
		{
			if(!_running) return true;
			_running=false;
			///\todo: some threadlocking mechanism. For now adding these sleeps
		#if defined _WIN32
			Sleep(1000);
		#else
			sleep(1);
		#endif
			TerminateThread( _hThread, 0 ); // just in case
			if(::waveOutReset(hWaveOut) != MMSYSERR_NOERROR)
			{
				std::cerr << "waveOutReset() failed" << std::endl;
				hWaveOut=0;
				return false;
			}

			if(waveOutClose(hWaveOut) != MMSYSERR_NOERROR)
			{
				std::cerr << "waveOutClose() failed" << std::endl;
				hWaveOut=0;
				return false;
			}
			hWaveOut=0;
			return true;
		}
	}
}
#endif
