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
#pragma once
#include "audiodriver.h"
#if defined __unix__ || defined __APPLE__
	#include <unistd.h>
#else
	#include <windows.h>
#endif
namespace psycle { namespace core {

class WaveFileOut : public AudioDriver {
	public:
		WaveFileOut();
		~WaveFileOut();
		/*override*/ AudioDriverInfo info() const;

	protected:
		/*override*/ void do_open() {}
		/*override*/ void do_start();
		/*override*/ void do_stop();
		/*override*/ void do_close() {}

	private:
		///\todo use std::thread
		#if defined __unix__ || defined __APPLE__
			pthread_t threadid;
		#else
			DWORD threadid;
		#endif

		///\todo bad
		static volatile int kill_thread;
		///\todo bad
		static volatile int threadOpen;

		static int audioOutThread(void * ptr);

		void writeBuffer();
};

}}
