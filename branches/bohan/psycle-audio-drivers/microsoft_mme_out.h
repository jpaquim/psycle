// copyright 2006-2007 psycledelics http://psycle.sourceforge.net
/*************************************************************************
*  This program is covered by the GNU General Public License:            *
*  http://gnu.org/licenses/gpl.html                                      *
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
*  This program is distributed in the hope that it will be useful,       *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
*  See the GNU General Public License for more details.                  *
*                                                                        *
*  You should have received a copy of the GNU General Public License     *
*  along with this program; if not, write to the                         *
*  Free Software Foundation, Inc.,                                       *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
**************************************************************************/
#pragma once
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
#if !defined _WIN64 && !defined _WIN32
	#error unsupported operating system
#endif
#include <windows.h>
#include <mmsystem.h>
#if !defined __GNUG__
	#pragma comment(lib, "winmm")
#endif
#undef min ///\todo
#undef max ///\todo
#include "cstdint.h" ///\todo
#include "helpers.h"
#include <iostream>
#include "audio_driver.h"

namespace psycle
{
	namespace audio_drivers
	{                                      
		///\ todo work in progress
		///\ status working, restarting etc not working
		///\ todo : freeing and configure    
		class MsWaveOut : public AudioDriver
		{
			public:
				MsWaveOut();
				~MsWaveOut();

				/// uses the copy constructor
				virtual MsWaveOut* clone()  const;

				virtual AudioDriverInfo info() const;			
				virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
				bool Initialized( );
				virtual bool Enable( bool e );	

			private:
				/// intermediate buffer for reading
				char buffer[1024];

				/// pointer for work callback
				AUDIODRIVERWORKFN _pCallback;
				/// pointer for work callback
				void* _callbackContext;
				bool _initialized;

				///\name mme variables
				///\{
					/// device handle
					HWAVEOUT hWaveOut;
					static CRITICAL_SECTION waveCriticalSection; ///\todo why static?
					/// array of header structure, 
					static WAVEHDR* waveBlocks; ///\todo why static? 
					/// that points to a block buffer
					static volatile int waveFreeBlockCount; ///\todo why static? why volatile?
					static int waveCurrentBlock; ///\todo why static? 
				///\}
				
				///\name mme functions
				///\{
					/// waveOut interface notifies about device is opened, closed, and what we handle here, when a block finishes.
					static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);      
					WAVEHDR* allocateBlocks();
					static void freeBlocks( WAVEHDR* blockArray );
					/// writes a intermediate buffer into a ring buffer to the sound card
					void writeAudio( HWAVEOUT hWaveOut, LPSTR data, int size );
				///\}

				/// thread , the writeAudio loop is in
				/// note : waveOutproc is a different, thread, too, but we cannot
				/// use all winapi calls there we need due to restrictions of the winapi
				HANDLE _hThread;
				static DWORD WINAPI audioOutThread( void *pWaveOut );
				/// check if thread loop should be left
				static bool _running;
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
