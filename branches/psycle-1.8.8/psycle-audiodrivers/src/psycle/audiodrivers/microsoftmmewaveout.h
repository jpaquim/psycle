// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__MICROSOFT_MME_WAVE_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__MICROSOFT_MME_WAVE_OUT__INCLUDED
#pragma once

#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
#include "audiodriver.h"

#include <universalis/os/include_windows_without_crap.hpp>

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

class MMEUiInterface {
	public:
		MMEUiInterface::MMEUiInterface() {}
		virtual ~MMEUiInterface() {}

		virtual int DoModal() = 0;

		virtual void SetValues(
			int device_idx, bool dither,
			int sample_rate, int buffer_size, int buffer_count) = 0;

		virtual void GetValues(
			int & device_idx, bool & dither,
			int & sample_rate, int & buffer_size, int & buffer_count) = 0;
			
		virtual void WriteConfig(
			int device_idx, bool dither,
			int sample_rate, int buffer_size, int buffer_count) = 0;

		virtual void ReadConfig(
			int & device_idx, bool & dither,
			int & sample_rate, int & buffer_size, int & buffer_count) = 0;

		virtual void Error(std::string const & msg) = 0;
};
///\todo work in progress
/// status working, restarting etc not working
///\todo freeing and configure    
class MsWaveOut : public AudioDriver {
	public:
		MsWaveOut(MMEUiInterface* = 0);
		~MsWaveOut() throw();

		/*override*/ AudioDriverInfo info() const;
		/*override*/ void Configure();

	protected:
		/*override*/ void do_open() throw(std::exception);
		/*override*/ void do_start() throw(std::exception);
		/*override*/ void do_stop() throw(std::exception);
		/*override*/ void do_close() throw(std::exception);

		/*override*/ void ReadConfig();
		/*override*/ void WriteConfig();

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
		static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);      
		WAVEHDR* allocateBlocks();
		static void freeBlocks( WAVEHDR* blockArray );
		// writes a intermediate buffer into a ring buffer to the sound card
		void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
		// thread , the writeAudio loop is in
		// note : waveOutproc is a different, thread, too, but we cant
		// use all winapi calls there we need due to restrictions of the winapi
		HANDLE _hThread;
		static DWORD WINAPI audioOutThread(void *pWaveOut);
		static bool running_; // check, if thread loop should be left
		void fillBuffer();
		int device_idx_;
		bool dither_;

		MMEUiInterface* ui_;
};

}}
#endif
#endif
