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
#include "audiodriver.h"

#include <ngrs/nproperty.h>

namespace psycle
{
	namespace host
	{

		AudioDriverInfo::AudioDriverInfo( const std::string & name, 
										 const std::string & header,
										 const std::string & description,
										 bool show ) : 
					name_( name ), header_( header ), description_( description ), show_( show )
		{
		}

		const std::string & AudioDriverInfo::name() {
			return name_;
		}

		const std::string & AudioDriverInfo::header() {
			return header_;
		}

		const std::string & AudioDriverInfo::description() {
			return description_;
		}

		bool AudioDriverInfo::show() const {
			return show_;
		}


		///
		/// audio driver setting class
		///
		AudioDriverSettings::AudioDriverSettings() :
			samplesPerSec_(44100),
			bitDepth_(16),
			channelMode_(3),
			bufferSize_(2048)
		{
		}

		AudioDriverSettings::~AudioDriverSettings() {
		}

		void AudioDriverSettings::setDeviceName( const std::string & name ) {
			deviceName_ = name;
		}

		const std::string & AudioDriverSettings::deviceName() const {
			return deviceName_;
		}

		void AudioDriverSettings::setBufferSize( int size ) {
			bufferSize_ = size;
		}

		int AudioDriverSettings::bufferSize() const {
			return bufferSize_;
		}

		void AudioDriverSettings::setSamplesPerSec( int samples ) {
			samplesPerSec_ = samples;
		}

		int AudioDriverSettings::samplesPerSec() const {
			return samplesPerSec_;
		}

		void AudioDriverSettings::setChannelMode( int mode ) {
			channelMode_ = mode;
		}

		int AudioDriverSettings::channelMode() const {
			return channelMode_;
		}

		void AudioDriverSettings::setBitDepth( int depth ) {
			bitDepth_ = depth;
		}

		int AudioDriverSettings::bitDepth() const {
			return bitDepth_;
		}


		///
		/// the AudioDriver main class
		///
		AudioDriver::AudioDriver()  
		{
		}

		AudioDriver::~AudioDriver()
		{
		}

		AudioDriver * AudioDriver::clone( ) const
		{
			return new AudioDriver(*this);
		}

		void AudioDriver::setSettings( const AudioDriverSettings & settings ) {
			settings_ = settings;
		}

		const AudioDriverSettings & AudioDriver::settings() {
			return settings_;
		}

		AudioDriverInfo AudioDriver::info( ) const
		{
			return AudioDriverInfo("silent","NullDriver","no sound output",true);
		}

	} // end of host namespace
} // end of psycle namespace
