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
#include "nalignlayout.h"
#include "nvisualcomponent.h"

NAlignLayout::NAlignLayout()
 : NLayout()
{
  maxX_ = maxY_ = 0;
}


NAlignLayout::~NAlignLayout()
{
}

void NAlignLayout::align( NVisualComponent * parent )
{
  NVisualComponent* lastTop    = 0;
  NVisualComponent* lastLeft   = 0;
  NVisualComponent* lastRight  = 0;
  NVisualComponent* lastBottom = 0;
  std::vector<NVisualComponent*>::const_iterator itr = parent->visualComponents().begin();

  for (;itr < parent->visualComponents().end(); itr++) {
     NVisualComponent* visualChild = *itr;
     if (visualChild->visible()) {
      switch (visualChild->align()) {
           case nAlRight: {
                  int topOff     = (lastTop  == 0) ? 0 : lastTop->top()   + lastTop->height() ;
                  int bottomOff  = (lastBottom == 0) ? 0 : parent->clientHeight() - lastBottom->top() ;
                  visualChild->setLeft(parent->clientWidth()-visualChild->preferredWidth());
                  visualChild->setTop ( topOff );
                  visualChild->setWidth(visualChild->preferredWidth());
                  visualChild->setHeight(parent->clientHeight() - topOff - bottomOff );
                  lastRight = visualChild;
                }
                break;
           case nAlLeft: {
                  int leftOff    = (lastLeft == 0) ? 0 : lastLeft->left()  + lastLeft->width() ;
                  int topOff     = (lastTop  == 0) ? 0 : lastTop->top()   + lastTop->height() ;
                  int bottomOff  = (lastBottom == 0) ? 0 : parent->clientHeight() - lastBottom->top() ;
                  visualChild->setLeft(leftOff);
                  visualChild->setTop ( topOff );
                  visualChild->setWidth(visualChild->preferredWidth());
                  visualChild->setHeight(parent->clientHeight() - topOff - bottomOff );
                  lastLeft = visualChild;
                }
                break;
           case nAlTop : {
                 int rightOff  = (lastRight == 0) ? 0 : parent->clientWidth() - lastRight->left() ;
                 int leftOff = (lastLeft == 0) ? 0 : lastLeft->left()   + lastLeft->width() ;
                 int topOff  = (lastTop  == 0) ? 0 : lastTop->top()   + lastTop->height() ;
                 visualChild->setLeft(leftOff);
                 visualChild->setTop ( topOff );
                 visualChild->setWidth(parent->clientWidth() - leftOff - rightOff);
                 visualChild->setHeight(visualChild->preferredHeight());
                 lastTop = visualChild;
                }
                break;
           case nAlBottom : {
                  int leftOff    = (lastLeft == 0) ? 0 : lastLeft->left()  + lastLeft->width() ;
                  int bottomOff  = (lastBottom  == 0) ? parent->clientHeight() : lastBottom->top();
                  visualChild->setLeft(leftOff);
                  visualChild->setWidth(parent->clientWidth()-leftOff);
                  visualChild->setHeight(visualChild->preferredHeight());
                  visualChild->setTop(bottomOff-visualChild->preferredHeight());
                  lastBottom = visualChild;
                }
                break;
           case nAlClient : {
                 int leftOff = (lastLeft == 0) ? 0 : lastLeft->left() + lastLeft->width() ;
                 int topOff  = (lastTop  == 0) ? 0 : lastTop->top()   + lastTop->height() ;
                 int bottomOff  = (lastBottom == 0) ? 0 : parent->clientHeight() - lastBottom->top() ;
                 int rightOff  = (lastRight == 0) ? 0 : parent->clientWidth() - lastRight->left() ;
                 maxX_ = visualChild->preferredWidth()  + leftOff;
                 maxY_ = visualChild->preferredHeight() + topOff + bottomOff;
                 visualChild->setPosition(leftOff,topOff,parent->clientWidth() - leftOff-rightOff,parent->clientHeight() - topOff - bottomOff);
                }
                break;
           default : ;
       }
     }
    }
}

int NAlignLayout::preferredWidth( const NVisualComponent * target ) const
{
  int xp = 0;

  std::vector<NVisualComponent*>::const_iterator itr = parent()->visualComponents().begin();

  for (;itr < parent()->visualComponents().end(); itr++) {
     NVisualComponent* visualChild = *itr;
     switch (visualChild->align()) {
       case nAlTop    : xp = xp + visualChild->preferredWidth();  break;
       case nAlLeft   : xp = xp + visualChild->preferredWidth();  break;
       case nAlRight  : xp = xp + visualChild->preferredWidth();  break;
       case nAlClient : xp = xp + visualChild->preferredWidth();  break;
      }
  }

 return xp;
}

int NAlignLayout::preferredHeight( const NVisualComponent * target ) const
{
  int yp = 0;
  std::vector<NVisualComponent*>::const_iterator itr = parent()->visualComponents().begin();

  for (;itr < parent()->visualComponents().end(); itr++) {
     NVisualComponent* visualChild = *itr;
     switch (visualChild->align()) {
       case nAlTop    : yp = yp + visualChild->preferredHeight();  break;
       case nAlBottom : yp = yp + visualChild->preferredHeight();  break;
       case nAlClient : yp = yp + visualChild->preferredHeight();  break;
      }
  }

 return yp;
}


