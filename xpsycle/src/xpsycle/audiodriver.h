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

		// class that provides some textinfo about the dirver
		class AudioDriverInfo {
		public:
				AudioDriverInfo( const std::string & name, const std::string & header, const std::string & description, bool show );
		
				// the driver name, used e.g in the configuration for the driver map
				// as convention, please use only lower case names here
				const std::string & name();
				// a short textual description
				const std::string & header();
				// a more deeper explanation
				const std::string & description();
				// gives the audio dialog a hint, to show this driver, or not
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

				// some drivers require a device name like alsa
				void setDeviceName( const std::string & name );
				const std::string & deviceName() const;

				void setBufferSize( int size );
				int bufferSize() const;

				// not all values possible maybe we should add a enum type here ..
				void setSamplesPerSec( int samples );
				int samplesPerSec() const;

				// channel mode 3 == stereo, 1 == mono left, 2 == mono right, 0 = mono both channels
				void setChannelMode( int mode );
				int channelMode() const;

				// bit depth values 8 16 24 
				void setBitDepth( int depth );
				int bitDepth() const;

				int sampleSize() const;

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

				// here you can set the settings of the driver, like samplerate depth etc
				virtual void setSettings( const AudioDriverSettings & settings );
				// here you get the special audio driver settings
				// in case of some drivers like  e.g jack you must prepare, that a driver can set itself
				const AudioDriverSettings & settings() const;


				virtual void Reset(void) {};
				// enable will start the driver and the calling for the work player function
				virtual bool Enable(bool e) { return false; };
				// initialize has nth with the driver todo, it sets only the ptr for a later player work call
				virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context) {};
				virtual void Configure(void) {};
				virtual bool Initialized(void) { return true; };
				virtual bool Configured(void) { return true; };
		
		private:

				AudioDriverSettings settings_; // holds the sampleRate/ bitRate etc

		};

	} // end of host namespace
} // end of psycle namespace

#endif
