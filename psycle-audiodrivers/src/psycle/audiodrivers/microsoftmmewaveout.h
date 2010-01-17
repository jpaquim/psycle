// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__MICROSOFT_MME_WAVE_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__MICROSOFT_MME_WAVE_OUT__INCLUDED
#pragma once

#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
#include "audiodriver.h"
#include <universalis/stdlib/cstdint.hpp>
#include <iostream>

#include <diversalis/compiler.hpp>

#include <windows.h>

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

#include <mmsystem.h>
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "winmm")
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

namespace psycle { namespace audiodrivers {

using namespace universalis::stdlib;

///\todo work in progress
/// status working, restarting etc not working
///\todo freeing and configure    
class MsWaveOut : public AudioDriver {
	public:
		MsWaveOut();
		~MsWaveOut() throw();

		/*override*/ AudioDriverInfo info() const;

	protected:
		/*override*/ void do_open() throw(std::exception);
		/*override*/ void do_start() throw(std::exception);
		/*override*/ void do_stop() throw(std::exception);
		/*override*/ void do_close() throw(std::exception);

		/*override*/ bool opened() const throw() { return hWaveOut != 0; }
		/*override*/ bool started() const throw() { return _running; }

	private:
		std::int16_t *buf;
		// mme variables
		HWAVEOUT hWaveOut;   // device handle
		static CRITICAL_SECTION waveCriticalSection;
		static WAVEHDR*         waveBlocks; // array of header structure, 
		// that points to a block buffer
		static volatile /* why volatile ? */ int waveFreeBlockCount;
		static int waveCurrentBlock;
		// mme functions
		// waveOut interface notifies about device is opened, closed, 
		// and what we handle here, when a block finishes.
		static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);      
		WAVEHDR* allocateBlocks();
		static void freeBlocks( WAVEHDR* blockArray );
		// writes a intermediate buffer into a ring buffer to the sound card
		void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
		// thread , the writeAudio loop is in
		// note : waveOutproc is a different, thread, too, but we cant
		// use all winapi calls there we need due to restrictions of the winapi
		HANDLE _hThread;
		static DWORD WINAPI audioOutThread(void *pWaveOut);
		static bool _running; // check, if thread loop should be left
		void fillBuffer();
		bool _dither;
};

}}
#endif
#endif
