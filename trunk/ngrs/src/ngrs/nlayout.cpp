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
#include "ngrs/nlayout.h"
#include "ngrs/nvisualcomponent.h"

NLayout::NLayout()
{
  parent_ = 0;
}


NLayout::~NLayout()
{
}

void NLayout::align( NVisualComponent * parent )
{
}



void NLayout::setParent( NVisualComponent * parent )
{
  parent_ = parent;
}

class NVisualComponent * NLayout::parent( ) const
{
  return parent_;
}



void NLayout::drawComponents( NVisualComponent * target, NGraphics* g, const NRegion & repaintArea , NVisualComponent* sender)
{
  std::vector<NVisualComponent*>::const_iterator itr = target->visualComponents().begin();
  for (;itr < target->visualComponents().end(); itr++) {
     NVisualComponent* visualChild = *itr;
     visualChild->draw(g,repaintArea,sender);
  }
}

void NLayout::add( class NVisualComponent * comp )
{
}

void NLayout::remove( class NVisualComponent * comp )
{
}

void NLayout::removeAll( )
{
}





