/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
*   psycle.sf.net   *
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
#pragma once
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
#include "audiodriver.h"
#include <windows.h>
#include <mmsystem.h>
#undef min
#undef max
#include <psycle/core/cstdint.h>
#include <psycle/core/helpers.h>
#include <iostream>
namespace psy
{
	namespace core
	{                                      
		///\ todo work in progress
		///\ status working, restarting etc not working
		///\ todo : freeing and configure    

		class MsWaveOut : public AudioDriver
		{
		public:
			MsWaveOut();

			~MsWaveOut();

			virtual MsWaveOut* clone()  const;   // Uses the copy constructor

			virtual AudioDriverInfo info() const;												
			virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
			bool Initialized( );
			virtual bool Enable( bool e );				

		private:

			char buffer[1024];   // intermediate buffer for reading

			// pointers for work callback
			AUDIODRIVERWORKFN _pCallback;
			void* _callbackContext;
			bool _initialized;

			// mme variables
			HWAVEOUT hWaveOut;   // device handle
			static CRITICAL_SECTION waveCriticalSection;
			static WAVEHDR*         waveBlocks; // array of header structure, 
			// that points to a block buffer
			static volatile int     waveFreeBlockCount;
			static int              waveCurrentBlock;

			// mme functions

			// waveOut interface notifies about device is opened, closed, 
			// and what we handle here, when a block finishes.
			static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);      
			WAVEHDR* allocateBlocks();
			static void freeBlocks( WAVEHDR* blockArray );

			// writes a intermediate buffer into a ring buffer to the sound card
			void writeAudio( HWAVEOUT hWaveOut, LPSTR data, int size );

			// thread , the writeAudio loop is in
			// note : waveOutproc is a different, thread, too, but we cant
			// use all winapi calls there we need due to restrictions of the winapi
			HANDLE _hThread;
			static DWORD WINAPI audioOutThread( void *pWaveOut );
			static bool _running; // check, if thread loop should be left
			void fillBuffer();

			bool _dither;

			bool start();
			bool stop();

			void quantizeWithDither(float *pin, int *piout, int c);
			void quantize(float *pin, int *piout, int c);

		};
	}
}
#endif
