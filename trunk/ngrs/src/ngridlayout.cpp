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
#include "ngridlayout.h"
#include "nvisualcomponent.h"

NGridLayout::NGridLayout( ) : cols_(1),rows_(1), hgap_(0), vgap_(0)
{
}

NGridLayout::NGridLayout(int cols, int rows)
 : NLayout(), cols_(cols),rows_(rows), hgap_(0), vgap_(0)
{
}

NGridLayout * NGridLayout::clone( ) const
{
  return new NGridLayout(*this);
}

NGridLayout::~NGridLayout()
{
}

void NGridLayout::align( NVisualComponent * parent )
{
  int maxX = findMaxColWidth();
  int maxY = findMaxRowHeight();

  std::vector<NVisualComponent*>::const_iterator itr = parent->visualComponents().begin();
  int x = 0;
  int y = 0;
  for (;itr < parent->visualComponents().end(); itr++) {
     NVisualComponent* visualChild = *itr;
     if (visualChild->visible()) {
        visualChild->setLeft(hgap_ + x*hgap_ + x*maxX);
        visualChild->setTop (vgap_ + y*vgap_ + y*maxY);
        visualChild->setWidth(maxX);
        visualChild->setHeight(maxY);
        x++;
        if (x % cols_ == 0) {
           x = 0;
           y++;
        }
    }
  }
}

int NGridLayout::preferredWidth( const NVisualComponent * target ) const
{
  return hgap_ + (findMaxColWidth() + hgap_) * cols_;
}

int NGridLayout::preferredHeight( const NVisualComponent * target ) const
{
  return vgap_ + ((findMaxRowHeight() + vgap_) * (parent()->visualComponents().size()) / (cols_));
}


// private stuff

int NGridLayout::findMaxColWidth( ) const
{
  int maxX = 0;
  std::vector<NVisualComponent*>::const_iterator itr = parent()->visualComponents().begin();
  for (;itr < parent()->visualComponents().end(); itr++) {
     NVisualComponent* visualChild = *itr;
     if (visualChild->visible()) {
        if (visualChild->preferredWidth() > maxX) maxX = visualChild->preferredWidth();
    }
  }
  return maxX;
}

int NGridLayout::findMaxRowHeight( ) const
{
  int maxY = 0;
  std::vector<NVisualComponent*>::const_iterator itr = parent()->visualComponents().begin();

  for (;itr < parent()->visualComponents().end(); itr++) {
     NVisualComponent* visualChild = *itr;
     if (visualChild->visible()) {
      if (visualChild->preferredHeight() > maxY) maxY = visualChild->preferredHeight();
    }
  }
  return maxY;
}

void NGridLayout::setRows( int number )
{
  rows_ = number;
}

void NGridLayout::setColumns( int number )
{
  cols_ = number;
}

void NGridLayout::setHgap( int hgap )
{
  hgap_ = hgap;
}

void NGridLayout::setVgap( int vgap )
{
  vgap_ = vgap;
}





