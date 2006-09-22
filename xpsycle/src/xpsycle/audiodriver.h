/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
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
#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include <string>

/**
@author Stefan
*/

namespace psycle 
{ 
	namespace host 
	{

		// typedef for work call back
		typedef float* (*AUDIODRIVERWORKFN)(void* context, int& numSamples);

		// class provides some textinfo about the dirver
		class AudioDriverInfo {
		public:
				AudioDriverInfo( const std::string & name, const std::string & header, const std::string & description, bool show );
		
				const std::string & name();
				const std::string & header();
				const std::string & description();
				bool show() const;

		private:

				std::string name_;
				std::string header_;
				std::string description_;
				bool show_;

		};


		// class that holds the info about samplerate depth etc
		class AudioDriverSettings {
		public :

				AudioDriverSettings();

				~AudioDriverSettings();

				void setDeviceName( const std::string & name );
				const std::string & deviceName() const;

				void setBufferSize( int size );
				int bufferSize() const;

				void setSamplesPerSec( int samples );
				int samplesPerSec() const;

				void setChannelMode( int mode );
				int channelMode() const;

				void setBitDepth( int depth );
				int bitDepth() const;

		private:

				int bufferSize_;
	 			int samplesPerSec_;
				int channelMode_;
				int bitDepth_;

				std::string deviceName_;

		};


		class AudioDriver {
		public:
				AudioDriver();

				~AudioDriver();

				// this clones the driver using the copy ctor
				virtual AudioDriver* clone()  const;

				// this gives you driver information
				virtual AudioDriverInfo info() const;

				virtual void Reset(void) {};
				virtual bool Enable(bool e) { return false; };
				virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context) {};
				virtual void Configure(void) {};
				virtual bool Initialized(void) { return true; };
				virtual bool Configured(void) { return true; };

				virtual void setSettings( const AudioDriverSettings & settings );
				const AudioDriverSettings & settings();
		
		private:

				AudioDriverSettings settings_; // holds the sampleRate/ bitRate etc

		};

	} // end of host namespace
} // end of psycle namespace

#endif
