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
#include "nautoscrolllayout.h"

NAutoScrollLayout::NAutoScrollLayout()
 : NLayout()
{
  maxY_ = maxY_ = 0;
}


NAutoScrollLayout::~NAutoScrollLayout()
{
}

void NAutoScrollLayout::align( NVisualComponent * parent )
{
  maxX_ = 0;
  maxY_ = 0;
  std::vector<NRuntime*>::iterator itr = parent->components.begin();
  int yp = 0;
  for (;itr < parent->components.end(); itr++) {
    NRuntime* child = *itr;
    //if (child->visit(NVisualComponent::isVisualComponent)) {
       // we know that the Component is a visual Component and can type safe cast due to the visitor pattern
       NVisualComponent* visualChild = static_cast<NVisualComponent*> (child);
       if (visualChild->visible()) {
        if (visualChild->width()+visualChild->left() > maxX_) {
           maxX_ = visualChild->width()+visualChild->left();
        }
        if (visualChild->height()+visualChild->top() > maxY_) {
           maxY_ = visualChild->width()+visualChild->left();
        }
       }
  }
  
}

int NAutoScrollLayout::preferredWidth( const NVisualComponent * target ) const
{
  return maxX_;
}

int NAutoScrollLayout::preferredHeight( const NVisualComponent * target ) const
{
  return maxY_;
}


