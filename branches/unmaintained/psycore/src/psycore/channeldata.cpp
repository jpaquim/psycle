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

#include "channeldata.h"

namespace psy {
  namespace core {


    Channel::Channel() 
      : owner_(0), data_(0), size_(0)
    {
    }

    Channel::~Channel() {
      if ( owner_) 
        delete[] data_;
    }

    void Channel::setBuffer( float* buffer, int size ) {
      data_ = buffer;
      size_ = size;
      owner_ = false;
    }

    float* Channel::buffer() {
      return data_;
    }

    void Channel::createBuffer( int size ) {
      data_ = new float[size];
      owner_ = true;
      size_ = size;
    }

    int Channel::size() const {
      return size_;
    }



    ChannelData::ChannelData()
    {
    }

    ChannelData::~ChannelData()
    {
    }

    std::list< Channel >::iterator ChannelData::begin() {
      return data_.begin();
    }

    std::list< Channel >::iterator ChannelData::end() {
      return data_.end();
    }
    

    void ChannelData::addNewChannel( int count ) {
      for ( int i = 0; i < count; i++ )
        data_.push_back( Channel() );
    }

    void ChannelData::removeAllChannels() {
      data_.clear();
    }

  }
}
