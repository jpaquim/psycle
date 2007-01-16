/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#ifndef NMOVEEVENT_H
#define NMOVEEVENT_H

/**
@author Stefan
*/

namespace ngrs {

  class Object;

  class MoveEvent{
  public:
    MoveEvent( Object* sender, int x, int y, int picker );

    ~MoveEvent();

    int x() const; // the x coord of the mouse relative to the container top
    int y() const; // the y coord of the mouse relative to the container top
    int picker() const; // the picker index, no picker = -1
    Object* sender() const; // pointer to the sender of this event

  private:

    Object* sender_;
    int x_;
    int y_;
    int picker_;

  };

}

#endif
