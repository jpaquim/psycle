/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "nkeyevent.h"
#include "nobject.h"

NKeyEvent::NKeyEvent(NObject* sender, std::string buffer, int scancode )
{
  buffer_   = buffer;
  scancode_ = scancode;
  sender_ = sender;
}


NKeyEvent::~NKeyEvent()
{
}

std::string NKeyEvent::buffer( ) const
{
  return buffer_;
}

int NKeyEvent::scancode( ) const
{
  return scancode_;
}

NObject * NKeyEvent::sender( ) const
{
  return sender_;
}


