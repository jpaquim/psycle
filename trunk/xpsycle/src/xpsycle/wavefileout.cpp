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


namespace psycle
{
	namespace host
	{

		WaveFileOut::WaveFileOut()
			: AudioDriver()
		{
			kill_thread = 0;
			_initialized = false;
		}


		WaveFileOut::~WaveFileOut()
		{
		}

		void WaveFileOut::Initialize(AUDIODRIVERWORKFN pCallback, void * context )
		{
			_pCallback = pCallback;
			_callbackContext = context;
			_initialized = true;
		}

		void WaveFileOut::setFileName( const std::string & fileName )
		{
			fileName_ = fileName;
		}

		const std::string & WaveFileOut::fileName( ) const
		{
			return fileName_;
		}

		bool WaveFileOut::Enable( bool e )
		{
			bool _recording = false;
			if (e && !_recording) {
					kill_thread = 0;
					if(_outputWaveFile.OpenForWrite(fileName().c_str(), _samplesPerSec, _bitDepth, _channelmode ) == DDC_SUCCESS)
							_recording = true;
					else
							_recording = false;
					if (_recording) {
							pthread_create(&threadid, NULL, (void*(*)(void*))audioOutThread, (void*) this);
					}

			} else 
			if (!e && _recording) { // disable fileout
				 kill_thread = 0;
				_recording = false;
				usleep(500); // give thread time to close
			}
			return _recording;
		}

		int WaveFileOut::audioOutThread( void * ptr )
		{
			WaveFileOut* waveFileOut = (WaveFileOut*) ptr;
			waveFileOut->writeBuffer();
		}

		void WaveFileOut::writeBuffer( )
		{
			bool _recording = true;

			int count = 441;
			int amount = count;

			while(!(kill_thread) && _recording)
			{
				float const * input(_pCallback(_callbackContext, count));
				usleep(100); // give cpu time to breath

				for (int i = 0; i < 441; i++) {
      		_pSamplesL[i] = *input++;
					_pSamplesR[i] = *input++;
  			}

				float* pL = _pSamplesL;
				float* pR = _pSamplesR;

				// Processing plant
				int i;
				switch(_channelmode)
				{
					case 0: // mono mix
						for( i = 0; i<amount; i++)
							{
								//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
								if(_outputWaveFile.WriteMonoSample(((*pL++)+(*pR++))/2) != DDC_SUCCESS)  _recording = false;
							}
							break;
						case 1: // mono L
							for( i = 0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample((*pL++)) != DDC_SUCCESS) _recording = false;
							}
							break;
						case 2: // mono R
							for( i = 0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample((*pR++)) != DDC_SUCCESS) _recording = false;
							}
							break;
						default: // stereo
							for( i = 0; i<amount; i++)
							{
								if(_outputWaveFile.WriteStereoSample((*pL++),(*pR++)) != DDC_SUCCESS) _recording = false;
							}
							break;
					}
				}
			_outputWaveFile.Close();
			pthread_exit(0);
		}


	} // end of host namespace
} // end of psycle namespace








