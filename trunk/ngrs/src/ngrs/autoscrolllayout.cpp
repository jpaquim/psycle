/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#include "autoscrolllayout.h"

namespace ngrs {

  AutoScrollLayout::AutoScrollLayout()
    : Layout()
  {
    maxY_ = maxY_ = 0;
  }

  Layout * AutoScrollLayout::clone( ) const
  {
    return new AutoScrollLayout(*this);
  }

  AutoScrollLayout::~AutoScrollLayout()
  {
  }

  void AutoScrollLayout::align( VisualComponent * parent )
  {
    maxX_ = 0;
    maxY_ = 0;
    std::vector<VisualComponent*>::const_iterator itr = parent->visualComponents().begin();

    for (;itr < parent->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      if (visualChild->visible()) {
        if (visualChild->width()+visualChild->left() > maxX_) {
          maxX_ = visualChild->width()+visualChild->left();
        }
        if (visualChild->height()+visualChild->top() > maxY_) {
          maxY_ = visualChild->height()+visualChild->top();
        }
      }
    }

  }

  int AutoScrollLayout::preferredWidth( const VisualComponent * target ) const
  {
    return maxX_;
  }

  int AutoScrollLayout::preferredHeight( const VisualComponent * target ) const
  {
    return maxY_;
  }

}