/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
 *   Germany   *
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
#ifndef NKEYEVENT_H
#define NKEYEVENT_H

#include <string>

/**
@author  Stefan
*/

namespace ngrs {

  // special keys that can be used in combination with a 'normal' key.
  enum NShiftState {
    nsNone   = 0,   
    nsShift  = 1,  // Shift key pressed
    nsAlt    = 2,  // Alt key pressed
    nsCtrl   = 4,  // Ctrl key pressed 
    nsLeft   = 8,  // Left mouse button pressed
    nsRight  = 16, // Right mouse button pressed
    nsMiddle = 32  // Middle mouse button pressed
  };	

  class KeyEvent{
  public:
    KeyEvent(class Object* sender, std::string buffer, int scancode, int shift = nsNone );

    ~KeyEvent();

    std::string buffer() const;
    int scancode() const;
    Object* sender() const;
    int shift() const;

  private:

    std::string buffer_;
    int scancode_;
    Object* sender_;
    int shift_;

  };

}

#endif
