/***************************************************************************
*   Copyright (C) 2007 Psycledelics    *
*   psycle.sf.net   *
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
namespace psy
{
	namespace core
	{

		AudioDriverInfo::AudioDriverInfo( const std::string& name, const std::string& header, const std::string& description, bool show ) 
		: name_( name ), header_( header ), description_( description ), show_( show )
		{
		}

		///
		/// audio driver setting class
		///
		AudioDriverSettings::AudioDriverSettings()
		:
			deviceName_("unnamed"),
			samplesPerSec_(44100),
			bitDepth_(16),
			channelMode_(3),
			bufferSize_(2048),
			blockSize_(4096),
			blockCount_(7)
		{}

		void AudioDriverSettings::setDeviceName( const std::string& name ) {
			deviceName_ = name;
		}

		void AudioDriverSettings::setBufferSize( int size ) {
			bufferSize_ = size;
		}

		void AudioDriverSettings::setSamplesPerSec( int samples ) {
			samplesPerSec_ = samples;
		}

		void AudioDriverSettings::setChannelMode( int mode ) {
			channelMode_ = mode;
		}

		void AudioDriverSettings::setBitDepth( int depth ) {
			bitDepth_ = depth;
		}

		int AudioDriverSettings::sampleSize() const {
			return ( channelMode_ == 3 ) ? bitDepth_ / 4 : bitDepth_ / 8;
		}

		void AudioDriverSettings::setBlockSize( int size ) {
			blockSize_ = size;   
		}     

		void AudioDriverSettings::setBlockCount( int count ) {
			blockCount_ = count;
		}
		
		///////////////////////////////////////////////////////////////////////////
		// the AudioDriver base class
		
		AudioDriver * AudioDriver::clone( ) const {
			return new AudioDriver(*this);
		}

		void AudioDriver::setSettings( const AudioDriverSettings& settings ) {
			settings_ = settings;
		}

		AudioDriverInfo AudioDriver::info( ) const
		{
			return AudioDriverInfo("silent", "null output driver", "no sound output", true);
		}

	}
}
