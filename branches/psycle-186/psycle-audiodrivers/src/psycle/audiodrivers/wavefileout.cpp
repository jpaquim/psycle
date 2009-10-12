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
#include "wavefileout.h"
#include <iostream>
#if defined _WIN64 || defined _WIN32
	#include <windows.h>
#endif
namespace psy { namespace core {

volatile int WaveFileOut::kill_thread = 0;
volatile int WaveFileOut::threadOpen = 0;

WaveFileOut::WaveFileOut()
: AudioDriver()
{
	kill_thread = 0;
	threadOpen = 0;
	_initialized = false;
}

WaveFileOut::~WaveFileOut() {
	///\todo use proper synchronisation mecanisms
	while ( threadOpen ) {
		kill_thread = 1;
		#if defined __unix__ || defined __APPLE__
			usleep(200);
		#else
			Sleep(1);
		#endif
	}
}

AudioDriverInfo WaveFileOut::info( ) const {
	return AudioDriverInfo("wavefileout","Wave to File Driver","Recording a wav to a file",false);
}

void WaveFileOut::Initialize(AUDIODRIVERWORKFN pCallback, void * context ) {
	_pCallback = pCallback;
	_callbackContext = context;
	_initialized = true;
}

bool WaveFileOut::Initialized(void) {
	return _initialized;
}

bool WaveFileOut::Enable( bool e ) {
	bool threadStarted = false;
	if (e && !threadOpen) {
		kill_thread = 0;
		///\todo use std::thread
		#if defined __unix__ || defined __APPLE__
			pthread_create(&threadid, NULL, (void*(*)(void*))audioOutThread, (void*) this);
		#else
			CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)audioOutThread, this, 0, &threadid);
		#endif
		threadStarted = true;
	} else
		if (!e && threadOpen) {
			kill_thread = 1;
			threadStarted = false;
			while ( threadOpen ) {
				///\todo bad
				#if defined __unix__ || defined __APPLE__
					usleep(10); // give thread time to close
				#else
					Sleep(1);
				#endif
			}
		}
		return threadStarted;
}

int WaveFileOut::audioOutThread(void * ptr) {
	WaveFileOut* waveFileOut = (WaveFileOut*) ptr;
	waveFileOut->writeBuffer();
	return 0;
}

void WaveFileOut::writeBuffer() {
	threadOpen = 1;
	int count = 441;

	while(!(kill_thread)) {
		///\todo bad
		#if defined __unix__ || defined __APPLE__
			usleep(50); // give cpu time to breath, and not too much :)
		#else
			Sleep(1);
		#endif
		float const * input(_pCallback(_callbackContext, count));
		///\todo well, the real job, i.e. output to a file
	}

	threadOpen = 0;
	std::cout << "closing thread" << std::endl;
	///\todo use std::thread.join
	#if defined __unix__ || defined __APPLE__
		pthread_exit(0);
	#else
		ExitThread(0);
	#endif
}
}}
