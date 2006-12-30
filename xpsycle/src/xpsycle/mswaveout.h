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
#ifndef MSWAVEOUT_H
#define MSWAVEOUT_H

#include "audiodriver.h"

#if defined __unix__ && !defined __CYGWIN__ && !defined __MSYS__ && !defined _UWIN
#else

#include "windows.h"


namespace psycle
{
	namespace host
	{                          
            
        ///\todo  this driver is only making strange noise
              
		/// output device interface implemented by mme.
		class MsWaveOut : public AudioDriver
		{
		public:
                  
			MsWaveOut();
			
			virtual ~MsWaveOut();
			
			virtual MsWaveOut* clone()  const;   // Uses the copy constructor
			
			virtual AudioDriverInfo info() const;
			
			virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
			virtual void Reset();
			virtual bool Enable( bool e );
	  	    virtual bool Initialized();
			virtual bool Configured();
			
			virtual int GetWritePos();
			virtual int GetPlayPos();
			int virtual GetMaxLatencyInSamples();
            // { return GetSampleSize() * _blockSize * _numBlocks; }
			virtual void Configure();

		private:
			static AudioDriverInfo _info;

			int _deviceID;
			HWAVEOUT _handle;

			bool _initialized;

			void ReadConfig();
			bool _configured;
			void WriteConfig();

			bool Start();
			bool _running;
			bool Stop();

			AUDIODRIVERWORKFN _pCallback;
			void* _callbackContext;
			static DWORD WINAPI PollerThread(void *pWaveOut);
			int _pollSleep;
			bool _stopPolling;

			int const static MAX_WAVEOUT_BLOCKS = 8;
			int _numBlocks;
			int _currentBlock;
			class CBlock
			{
				public:
					HANDLE Handle;
					unsigned char *pData;
					WAVEHDR *pHeader;
					HANDLE HeaderHandle;
					bool Prepared;
			};
			CBlock _blocks[MAX_WAVEOUT_BLOCKS];

			void DoBlocks();
			int _writePos;

			int _dither;

			void Error(const char msg[]);
			
			HANDLE hThread_;

			void quantizeWithDither(float *pin, int *piout, int c);
			void quantize(float *pin, int *piout, int c);
			
		};
	}
}

#endif // end of windows detetection

#endif
