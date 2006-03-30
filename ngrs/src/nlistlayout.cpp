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
#include "nlistlayout.h"

NListLayout::NListLayout()
 : NLayout()
{
  align_ = nAlNone;
  maxX_ = maxY_ = 0;
}


NListLayout::~NListLayout()
{
}

void NListLayout::align( NVisualComponent * parent )
{
  maxX_ = maxY_ = 0;
  std::vector<NRuntime*>::iterator itr = parent->components.begin();
  int yp = 0;
  for (;itr < parent->components.end(); itr++) {
    NRuntime* child = *itr;
    //if (child->visit(NVisualComponent::isVisualComponent)) {
       // we know that the Component is a visual Component and can type safe cast due to the visitor pattern
       NVisualComponent* visualChild = static_cast<NVisualComponent*> (child);
       if (visualChild->visible()) {
        int prefWidth = visualChild->preferredWidth();
        int minWidth = visualChild->minimumWidth();
        int width = (prefWidth < minWidth) ? minWidth : prefWidth; 
        visualChild->setPosition(0,yp,width,visualChild->preferredHeight());
        if (visualChild->align() == nAlClient) visualChild->setWidth(parent->clientWidth());
        if (width > maxX_) {
           maxX_ = width;
        }
        yp+= visualChild->preferredHeight();
       }
    //}
  }
  maxY_ = yp;
}


int NListLayout::preferredHeight( const NVisualComponent * target ) const
{
  /*if (parent()->componentSize() > 0) {
      NVisualComponent* child = (NVisualComponent*) parent()->components.back();
      return child->top()+child->preferredHeight();
  }*/
  return maxY_;
}

int NListLayout::preferredWidth( const NVisualComponent * target ) const
{
  return maxX_;
}

void NListLayout::drawComponents( NVisualComponent * target, NGraphics * g, const NRect & repaintArea )
{
  int start = findVerticalStart(target->scrollDy(), target);
  std::vector<NRuntime*>::iterator itr = target->components.begin() + start;
  for (;itr < target->components.end(); itr++) {
      NRuntime* child = *itr;
      if (child->visit(NVisualComponent::isVisualComponent)) {
       // we know that the Component is a visual Component and can type safe cast due to the visitor pattern
        NVisualComponent* visualChild = static_cast<NVisualComponent*> (child);
        visualChild->draw(g,repaintArea);
        if (visualChild->top()+visualChild->height()-target->scrollDy()>target->height()) break;
      }
    }
  //printf("%d\n",c); fflush(stdout);
}

int NListLayout::findVerticalStart( long comparator , NVisualComponent* owner)
{
  int Low = 0; int High = owner->componentSize()-1;  int Mid=High; int w=0;
  int z=0;
  NVisualComponent* cont = owner;
  while( Low <= High ) {
    z++;
    Mid = ( Low + High ) / 2;
    NVisualComponent* b = (NVisualComponent*) owner->components.at(Mid);
    int absTop = cont->absoluteTop();
    w=absTop+b->top()-comparator;
    if(  w < 0 ) {
                   if (absTop+b->top()+b->height() > comparator ) {
        //              printf("%d\n",z); fflush(stdout);
                   return Mid;
                   }
                   Low = Mid + 1;
                 }  else 
                 {
                   if (absTop+b->top()+b->height() < comparator ) {
      //               printf("%d\n",z); fflush(stdout);
                     return Mid;
                   }
                   High = Mid - 1;
                 }
  }
    //printf("%d\n",z); fflush(stdout);
  if (Mid<0) return 0;
  return Mid;
}

void NListLayout::setAlign( int align )
{
  align_ = align;
}

