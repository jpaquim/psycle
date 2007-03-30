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
#ifndef CHANNELDATA_H
#define CHANNELDATA_H

#include <vector>
#include <list>

namespace psy {
  namespace core {

    class Channel {
    public:
      Channel();
      ~Channel();

      void setBuffer( float* buffer, int size );
      float* buffer();
      void createBuffer( int size = 0 );

      int size() const;

    private:
      
      bool owner_;
      float* data_;
      int size_;

    };

    class ChannelData
    {
    public:

      ChannelData();

      ~ChannelData();

      std::list< Channel >::iterator begin();
      std::list< Channel >::iterator end();

      void addNewChannel( int count = 1 );

      void removeAllChannels();

    private:

      // 1. channel list, 2. sample data of each channel
      std::list< Channel > data_;

    };

  }
}

#endif
