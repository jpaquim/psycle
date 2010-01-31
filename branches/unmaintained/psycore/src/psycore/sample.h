/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper   *
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
#ifndef SAMPLE_H
#define SAMPLE_H

#include "channeldata.h"

namespace psy {
  namespace core {

    // multi channel sample datastructure

    class Sample
    {
    public:

      Sample();
      ~Sample();

      void loadFromFile( const std::string& fileName );

      ChannelData& channels();

    private:

      unsigned int frames_;
      unsigned int samplesPerSec_;

      ChannelData channels_;

    };

  }
}

#endif