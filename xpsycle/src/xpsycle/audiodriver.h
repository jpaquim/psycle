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

#include <ngrs/nobject.h>

/**
@author Stefan
*/

namespace psycle { namespace host {

class AudioDriverInfo {
public:

		AudioDriverInfo( const std::string & name ) {
			name_ = name;
		}

    const std::string & name() {
			return name_;
		}

private:

		std::string name_;
		
};

typedef float* (*AUDIODRIVERWORKFN)(void* context, int& numSamples);

class AudioDriver : NObject {
public:
    AudioDriver();

    ~AudioDriver();

		virtual AudioDriverInfo info() const;

    virtual void Reset(void) {};
    virtual bool Enable(bool e) { return false; };
    virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context) {};
    virtual void Configure(void) {};
    virtual bool Initialized(void) { return true; };
    virtual bool Configured(void) { return true; };

    int _numBlocks;
    int _blockSize;
    int _samplesPerSec;
    int _channelmode;
    int _bitDepth;

    signal0<> tick;


};

}}

#endif
