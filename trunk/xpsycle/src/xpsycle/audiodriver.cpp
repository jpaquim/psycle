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

namespace psycle { namespace host {


AudioDriver::AudioDriver() : 
    _samplesPerSec(44100)
  , _bitDepth(16)
  , _channelmode(3)
  , _numBlocks(0)
  , _blockSize(0)
{
	properties()->bind("Samples/sec", *this, &AudioDriver::samplesPerSec,  &AudioDriver::setSamplesPerSec);
	properties()->publish("Samples/sec");
  properties()->bind("Bitdepth", *this, &AudioDriver::bitDepth,  &AudioDriver::setBitDepth);
	properties()->publish("Bitdepth");
	properties()->bind("ChannelMode", *this, &AudioDriver::channelMode,  &AudioDriver::setChannelMode);
	properties()->publish("ChannelMode");
}

AudioDriver * AudioDriver::clone( ) const
{
  return new AudioDriver(*this);
}


void AudioDriver::setSamplesPerSec( int samples )
{
  _samplesPerSec = samples;
}

int AudioDriver::samplesPerSec( ) const
{
  return _samplesPerSec;
}

void AudioDriver::setBitDepth( int depth )
{
  _bitDepth = depth;
}

void AudioDriver::setChannelMode( int mode )
{
  mode = _channelmode;
}

int AudioDriver::channelMode( ) const
{
  return _channelmode;
}

int AudioDriver::bitDepth( ) const
{
	return _bitDepth;
}

AudioDriver::~AudioDriver()
{
}

AudioDriverInfo AudioDriver::info( ) const
{
  return AudioDriverInfo("silent");
}

}}









