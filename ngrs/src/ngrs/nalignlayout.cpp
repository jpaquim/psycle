/***************************************************************************
 *   Copyright (C) 2005 by Stefan Nattkemper                               *
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
#include <algorithm>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

NAlignLayout::NAlignLayout()
 : NLayout()
{
  maxX_ = maxY_ = 0;
  hgap_ = vgap_ = 0;
}

NAlignLayout::NAlignLayout( int hgap, int vgap )
{
  maxX_ = maxY_ = 0;
  hgap_ = hgap;
  vgap_ = vgap;
}

NAlignLayout * NAlignLayout::clone( ) const
{
  return new NAlignLayout(*this);
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
  std::vector<NVisualComponent*>::const_iterator itr = components.begin();

  for (;itr < components.end(); itr++) {
     NVisualComponent* visualChild = *itr;
     if (visualChild->visible()) {
      switch (visualChild->align()) {
           case nAlRight: {
                  int topOff     = (lastTop  == 0) ? vgap_ : lastTop->top()   + lastTop->height() +vgap_  ;
                  int bottomOff  = (lastBottom == 0) ? vgap_ : parent->clientHeight() - lastBottom->top() - vgap_;
                  int rightOff = (lastRight  == 0) ? parent->clientWidth() - hgap_ : lastRight->left() - hgap_;

                  visualChild->setLeft(rightOff-visualChild->preferredWidth());
                  visualChild->setTop ( topOff );
                  visualChild->setWidth(visualChild->preferredWidth());
                  visualChild->setHeight(parent->clientHeight() - topOff - bottomOff );
                  lastRight = visualChild;
                }
                break;
           case nAlLeft: {
                  int leftOff    = (lastLeft == 0) ? hgap_ : lastLeft->left()  + lastLeft->width() + hgap_ ;
                  int topOff     = (lastTop  == 0) ? vgap_ : lastTop->top()   + lastTop->height() +vgap_ ;
                  int bottomOff  = (lastBottom == 0) ? vgap_ : parent->clientHeight() - lastBottom->top() - vgap_ ;
                  visualChild->setLeft(leftOff);
                  visualChild->setTop ( topOff );
                  visualChild->setWidth(visualChild->preferredWidth());
                  visualChild->setHeight(parent->clientHeight() - topOff - bottomOff );
                  lastLeft = visualChild;
                }
                break;
           case nAlTop : {
                 int rightOff  = (lastRight == 0) ? hgap_ : parent->clientWidth() - lastRight->left() - vgap_ ;
                 int leftOff = (lastLeft == 0) ? vgap_ : lastLeft->left()   + lastLeft->width() + vgap_;
                 int topOff  = (lastTop  == 0) ? vgap_ : lastTop->top()   + lastTop->height() + vgap_;
                 visualChild->setLeft(leftOff);
                 visualChild->setTop ( topOff );
                 visualChild->setWidth(parent->clientWidth() - leftOff - rightOff);
                 visualChild->setHeight(visualChild->preferredHeight());
                 lastTop = visualChild;
                }
                break;
           case nAlBottom : {
                  int leftOff = (lastLeft == 0) ? hgap_ : lastLeft->left() + lastLeft->width() + hgap_;
                  int bottomOff = (lastBottom  == 0) ? parent->clientHeight() - vgap_ : lastBottom->top() - vgap_;
                  visualChild->setLeft(leftOff);
                  visualChild->setWidth(parent->clientWidth()-leftOff);
                  visualChild->setHeight(visualChild->preferredHeight());
                  visualChild->setTop(bottomOff-visualChild->preferredHeight());
                  lastBottom = visualChild;
                }
                break;
           case nAlClient : {
                 int leftOff = (lastLeft == 0) ? hgap_ : lastLeft->left() + lastLeft->width() + hgap_ ;
                 int topOff  = (lastTop  == 0) ? vgap_ : lastTop->top()   + lastTop->height() + vgap_;
                 int bottomOff  = (lastBottom == 0) ? vgap_ : parent->clientHeight() - lastBottom->top() + vgap_ ;
                 int rightOff  = (lastRight == 0) ? hgap_ : parent->clientWidth() - (lastRight->left() - hgap_);
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
  // this will store the block width of same aligns; eg. the width sum of all nAlignTop components
  int top    = 0;
  int left   = 0;
  int right  = 0;
  int bottom = 0;
  int client = 0;

  // pointers to determinate, if a coponent with that align is there
  NVisualComponent* lastTop    = 0;
  NVisualComponent* lastLeft   = 0;
  NVisualComponent* lastRight  = 0;
  NVisualComponent* lastBottom = 0;

  // flags to determine the 16 possible layout structures
  bool topBeforeLeft     = 1;
  bool topBeforeRight    = 1;
  bool bottomBeforeLeft  = 1;
  bool bottomBeforeRight = 1;

  std::vector<NVisualComponent*>::const_iterator it = components.begin();

  // compute for each block the preferredHeight
  for ( ; it < components.end(); it++ ) {
    NVisualComponent* visualChild = *it;
    if (visualChild->visible()) {
      switch ( visualChild->align() ) {
        case nAlLeft   :
          left += hgap_ + visualChild->preferredWidth();
          lastLeft = visualChild;
        break;
        case nAlTop    :
          if (lastLeft)  topBeforeLeft  = 0;
          if (lastRight) topBeforeRight = 0;
          top = std::max(visualChild->preferredWidth(),top);
          lastTop = visualChild;
        break;
        case nAlRight  :
          right += hgap_ + visualChild->preferredWidth();
          lastRight = visualChild;
        break;
        case nAlBottom :
          if (lastLeft)  bottomBeforeLeft  = 0;
          if (lastRight) bottomBeforeRight = 0;
          bottom = std::max(visualChild->preferredWidth(),bottom);
          lastBottom = visualChild;
        break;
        case nAlClient :
          client = hgap_ + visualChild->preferredWidth();
        break;
      }
    }
  }

  int xmax = 0;

  // determine, which layout structure is there and compute the preferredWidth of the layout

  if (topBeforeLeft && topBeforeRight && bottomBeforeLeft && bottomBeforeRight) { // 0000
    xmax = std::max(std::max(client + left + right, bottom),top);
  } else
  if (topBeforeLeft && topBeforeRight && bottomBeforeLeft && !bottomBeforeRight) { //0001
    xmax = std::max(std::max(left+client,bottom)+right,top);
  } else
  if (topBeforeLeft && topBeforeRight && !bottomBeforeLeft && bottomBeforeRight) { //0010
    xmax = std::max( std::max( right + client, bottom ) + left, top);
  } else
  if (topBeforeLeft && topBeforeRight && !bottomBeforeLeft && !bottomBeforeRight) {//0011
    xmax = std::max(std::max(client,bottom)+left+right,top);
  } else
  if (topBeforeLeft && !topBeforeRight && bottomBeforeLeft && bottomBeforeRight) { //0100
    xmax = std::max(std::max(left+client,top)+right,bottom);
  } else
  if (topBeforeLeft && !topBeforeRight && bottomBeforeLeft && !bottomBeforeRight) {//0101
    xmax = std::max(std::max(left+client,top),bottom)+right;
  } else
  if (topBeforeLeft && !topBeforeRight && !bottomBeforeLeft && bottomBeforeRight) {//0110
    xmax = std::max(std::max(left+client,top)+right,bottom+left);
  } else
  if (topBeforeLeft && !topBeforeRight && !bottomBeforeLeft && !bottomBeforeRight) {//0111
    xmax = std::max(std::max(client,bottom)+left,top)+right;
  } else
  if (!topBeforeLeft && topBeforeRight && bottomBeforeLeft && bottomBeforeRight) {//1000
    xmax = std::max(std::max(client+right,top)+left,bottom);
  } else
  if (!topBeforeLeft && topBeforeRight && bottomBeforeLeft && !bottomBeforeRight) {//1001
    xmax = std::max(left+top,right+bottom);
  } else
  if (!topBeforeLeft && topBeforeRight && !bottomBeforeLeft && bottomBeforeRight) {//1010
    xmax = std::max(std::max(client+right,bottom),top)+left;
  } else
  if (!topBeforeLeft && topBeforeRight && !bottomBeforeLeft && !bottomBeforeRight) {//1011
    xmax = std::max(std::max(client,bottom)+right,top)+left;
  } else
  if (!topBeforeLeft && !topBeforeRight && bottomBeforeLeft && bottomBeforeRight) {//1100
    xmax = std::max(std::max(client,top)+left+right,bottom);
  } else
  if (!topBeforeLeft && !topBeforeRight && bottomBeforeLeft && !bottomBeforeRight) {//1101
    xmax = std::max(std::max(client,top)+left,bottom)+right;
  } else
  if (!topBeforeLeft && !topBeforeRight && !bottomBeforeLeft && bottomBeforeRight) {//1110
    xmax = std::max(std::max(client,top)+right,bottom)+left;
  } else
  if (!topBeforeLeft && !topBeforeRight && !bottomBeforeLeft && !bottomBeforeRight) {//1111
    xmax = std::max(std::max(client,top),bottom)+left+right;
  }

  return hgap_ + xmax;
}

int NAlignLayout::preferredHeight( const NVisualComponent * target ) const
{
  // this will store the block height of same aligns; eg. the height sum of all nAlignTop components
  int top    = 0;
  int left   = 0;
  int right  = 0;
  int bottom = 0;
  int client = 0;

  // pointers to determinate, if a coponent with that align is there
  NVisualComponent* lastTop    = 0;
  NVisualComponent* lastLeft   = 0;
  NVisualComponent* lastRight  = 0;
  NVisualComponent* lastBottom = 0;

  // flags to determine the 16 possible layout structures
  bool topBeforeLeft     = 1;
  bool topBeforeRight    = 1;
  bool bottomBeforeLeft  = 1;
  bool bottomBeforeRight = 1;

  std::vector<NVisualComponent*>::const_iterator it = components.begin();

  // compute for each block the preferredHeight
  for ( ; it < components.end(); it++ ) {
    NVisualComponent* visualChild = *it;
    if (visualChild->visible()) {
      switch ( visualChild->align() ) {
        case nAlLeft   :
          left = std::max( left, visualChild->preferredHeight() );
          lastLeft = visualChild;
        break;
        case nAlTop    :
          if (lastLeft)  topBeforeLeft  = 0;
          if (lastRight) topBeforeRight = 0;
          top += vgap_ + visualChild->preferredHeight();
          lastTop = visualChild;
        break;
        case nAlRight  :
          right = std::max( right, vgap_ + visualChild->preferredHeight() );
          lastRight = visualChild;
        break;
        case nAlBottom :
          if (lastLeft)  bottomBeforeLeft  = 0;
          if (lastRight) bottomBeforeRight = 0;
          bottom += vgap_ + visualChild->preferredHeight();
          lastBottom = visualChild;
        break;
        case nAlClient :
          client = vgap_+visualChild->preferredHeight();
        break;
      }
    }
  }

	int ymax = 0;
	
  // determine, which layout structure is there and compute the preferredHeight of the layout

  if (topBeforeLeft && topBeforeRight && bottomBeforeLeft && bottomBeforeRight) { // 0000
    ymax = std::max( std::max(left,right), client ) + top + bottom;
  } else
  if (topBeforeLeft && topBeforeRight && bottomBeforeLeft && !bottomBeforeRight) { //0001
    ymax = std::max( std::max(left,client) + bottom, right ) + top;
  } else
  if (topBeforeLeft && topBeforeRight && !bottomBeforeLeft && bottomBeforeRight) { //0010
    ymax = std::max( std::max(right,client) + bottom, left ) + top;
  } else
  if (topBeforeLeft && topBeforeRight && !bottomBeforeLeft && !bottomBeforeRight) {//0011
    ymax = std::max( std::max(client + bottom, left), right ) + top;
  } else
  if (topBeforeLeft && !topBeforeRight && bottomBeforeLeft && bottomBeforeRight) { //0100
    ymax = std::max(right, std::max ( client, left ) + top ) + bottom;
  } else
  if (topBeforeLeft && !topBeforeRight && bottomBeforeLeft && !bottomBeforeRight) {//0101
    ymax = std::max(right, std::max ( client, left ) + top + bottom);
  } else
  if (topBeforeLeft && !topBeforeRight && !bottomBeforeLeft && bottomBeforeRight) {//0110
    ymax = std::max(right, std::max ( top + client, right ) + bottom); // not sure
  } else
  if (topBeforeLeft && !topBeforeRight && !bottomBeforeLeft && !bottomBeforeRight) {//0111
    ymax = std::max(right, std::max ( client + bottom, left ) + top);
  } else
  if (!topBeforeLeft && topBeforeRight && bottomBeforeLeft && bottomBeforeRight) {//1000
    ymax = std::max( std::max( client, right) + top, left) + bottom;
  } else
  if (!topBeforeLeft && topBeforeRight && bottomBeforeLeft && !bottomBeforeRight) {//1001
    ymax = std::max( std::max( client + top, left) + bottom, right);
  } else
  if (!topBeforeLeft && topBeforeRight && !bottomBeforeLeft && bottomBeforeRight) {//1010
    ymax = std::max( std::max( client, right) + top + bottom, left);
  } else
  if (!topBeforeLeft && topBeforeRight && !bottomBeforeLeft && !bottomBeforeRight) {//1011
    ymax = std::max( std::max( client + bottom, right) + top, left);
  } else
  if (!topBeforeLeft && !topBeforeRight && bottomBeforeLeft && bottomBeforeRight) {//1100
    ymax = std::max( std::max( client + top, right ), left) + bottom;
  } else
  if (!topBeforeLeft && !topBeforeRight && bottomBeforeLeft && !bottomBeforeRight) {//1101
    ymax = std::max( std::max( client + top, left) + bottom, right);
  } else
  if (!topBeforeLeft && !topBeforeRight && !bottomBeforeLeft && bottomBeforeRight) {//1110
    ymax = std::max( std::max( client + top, right) + bottom, left);
  } else
  if (!topBeforeLeft && !topBeforeRight && !bottomBeforeLeft && !bottomBeforeRight) {//1111
    ymax = std::max( top + bottom + client, std::max( left, right));
  }

  return 2 * vgap_ + ymax;
}

void NAlignLayout::setHgap( int hgap )
{
  hgap_ = hgap;
}

void NAlignLayout::setVgap( int vgap )
{
  vgap_ = vgap;
}

// this adds and removes components to the layout

void NAlignLayout::add( NVisualComponent * comp )
{
  components.push_back(comp);
}

void NAlignLayout::remove( NVisualComponent * comp )
{
  std::vector<NVisualComponent*>::iterator it = components.begin();
  it = find(components.begin(), components.end(), comp);
  if ( it != components.end() ) components.erase(it);
}

void NAlignLayout::removeAll( )
{
  components.clear();
}
