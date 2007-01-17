/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "layout.h"
#include "visualcomponent.h"

namespace ngrs {

  Layout::Layout()
  {
    parent_ = 0;
  }


  Layout::~Layout()
  {
  }

  void Layout::align( VisualComponent * parent )
  {
  }

  void Layout::setParent( VisualComponent * parent )
  {
    parent_ = parent;
  }

  class VisualComponent * Layout::parent( ) const
  {
    return parent_;
  }

  void Layout::drawComponents( VisualComponent * target, Graphics& g, const ngrs::Region & repaintArea , VisualComponent* sender)
  {
    std::vector<VisualComponent*>::const_iterator itr = target->visualComponents().begin();
    for (;itr < target->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      visualChild->draw(g,repaintArea,sender);
    }
  }

  void Layout::insert( class VisualComponent * comp, int index )
  {
  }

  void Layout::add( class VisualComponent * comp )
  {
  }

  void Layout::remove( class VisualComponent * comp )
  {
  }

  void Layout::removeAll( )
  {
  }

}
