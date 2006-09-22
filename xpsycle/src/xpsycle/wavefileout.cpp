/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "wavefileout.h"
#include "player.h"


namespace psycle
{
	namespace host
	{

		WaveFileOut::WaveFileOut()
			: AudioDriver()
		{
			kill_thread = 0;
			threadOpen = 0;
			_initialized = false;
		}


		WaveFileOut::~WaveFileOut()
		{
		}

		WaveFileOut * WaveFileOut::clone( ) const
		{
			return new WaveFileOut(*this);
		}


		AudioDriverInfo WaveFileOut::info( ) const
		{
			return AudioDriverInfo("wavefileout","Wave to File Driver","Recording a wav to a file",false);
		}

		void WaveFileOut::Initialize(AUDIODRIVERWORKFN pCallback, void * context )
		{
			_pCallback = pCallback;
			_callbackContext = context;
			_initialized = true;
		}

		bool WaveFileOut::Enable( bool e )
		{
			bool threadStarted = false;
			if (e && !threadOpen) {
					kill_thread = 0;
					pthread_create(&threadid, NULL, (void*(*)(void*))audioOutThread, (void*) this);
					threadStarted = true;
			} else
			if (!e && threadOpen) {
				kill_thread = 1;
				threadStarted = false;
				usleep(500); // give thread time to close
			}
			return threadStarted;
		}

		int WaveFileOut::audioOutThread( void * ptr )
		{
			WaveFileOut* waveFileOut = (WaveFileOut*) ptr;
			waveFileOut->writeBuffer();
		}

		void WaveFileOut::writeBuffer( )
		{
			threadOpen = 1;

			int count = 441;


			while(!(kill_thread))
			{
				usleep(100); // give cpu time to breath

				float const * input(_pCallback(_callbackContext, count));
			}

			threadOpen = 0;
			pthread_exit(0);
		}


	} // end of host namespace
} // end of psycle namespace








