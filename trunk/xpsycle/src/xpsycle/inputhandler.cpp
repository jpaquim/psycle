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
#include "inputhandler.h"

namespace psycle {
namespace host {

InputHandler::InputHandler()
{
}


InputHandler::~InputHandler()
{
}

int InputHandler::getEnumCodeByKey( const Key & key )
{
  std::map<Key,int>::iterator itr;
  if ( (itr = keyMap.find(key)) != keyMap.end() )
      return itr->second;
    else
      return -1;
}

void InputHandler::changeKeyCode( int keyEnumCode, const Key & key )
{
  keyMap[key] = keyEnumCode;
}

Key::Key( int c, int k )
{
  scancode = k;
///\todo: this needs to be reworked, first fix xml parser to user ControlMask instead of 0/1. Later on, think about ShiftMask or others..
  if (c ) ctrl = ControlMask; else ctrl = 0;
}

}
}
