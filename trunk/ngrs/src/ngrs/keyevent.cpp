/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "keyevent.h"
#include "object.h"

namespace ngrs {

  KeyEvent::KeyEvent(Object* sender, std::string buffer, int scancode, int shift )
  {
    buffer_   = buffer;
    scancode_ = scancode;
    sender_   = sender;
    shift_    = shift; 
  }


  KeyEvent::~KeyEvent()
  {
  }

  std::string KeyEvent::buffer( ) const
  {
    return buffer_;
  }

  int KeyEvent::scancode( ) const
  {
    return scancode_;
  }

  Object * KeyEvent::sender( ) const
  {
    return sender_;
  }

  int KeyEvent::shift() const
  {
    return shift_;
  }

}
