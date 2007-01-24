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
#include "listlayout.h"

namespace ngrs {

  ListLayout::ListLayout()
    : Layout()
  {
    align_ = nAlNone;
    maxX_ = maxY_ = 0;
  }


  ListLayout * ListLayout::clone( ) const
  {
    return new ListLayout(*this);
  }


  ListLayout::~ListLayout()
  {
  }

  void ListLayout::align( VisualComponent * parent )
  {
    maxX_ = 0;
    maxY_ = 0;

    std::vector<VisualComponent*>::const_iterator itr = parent->visualComponents().begin();
    int yp = ident().y();
    for (;itr < parent->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      if (visualChild->visible()) {
        int prefWidth = visualChild->preferredWidth();
        int minWidth = visualChild->minimumWidth();
        int width = (prefWidth < minWidth) ? minWidth : prefWidth; 
        visualChild->setPosition( ident().x(),yp,width,visualChild->preferredHeight());
        if (visualChild->align() == nAlCenter) {
          visualChild->setWidth(parent->spacingWidth());
        }
        if (width > maxX_) {
          maxX_ = width;
        }
        yp+= visualChild->preferredHeight();
      }
    }
    maxY_ = yp;
    maxX_ = maxX_ + ident().x();
  }


  int ListLayout::preferredHeight( const VisualComponent * target ) const
  {
    /*if (parent()->componentSize() > 0) {
    VisualComponent* child = (VisualComponent*) parent()->components.back();
    return child->top()+child->preferredHeight();
    }*/
    return maxY_;
  }

  int ListLayout::preferredWidth( const VisualComponent * target ) const
  {
    return maxX_;
  }

  void ListLayout::drawComponents( VisualComponent * target, Graphics& g, const ngrs::Region & repaintArea,VisualComponent* sender )
  {
    int start = findVerticalStart(target->scrollDy(), target);
    std::vector<Runtime*>::iterator itr = target->components.begin() + start;
    for (;itr < target->components.end(); itr++) {
      Runtime* child = *itr;
      if (child->visit(VisualComponent::isVisualComponent)) {
        // we know that the Component is a visual Component and can type safe cast due to the visitor pattern
        VisualComponent* visualChild = static_cast<VisualComponent*> (child);
        visualChild->draw(g,repaintArea,sender);
        if (visualChild->top()+visualChild->height()-target->scrollDy()>target->height()) break;
      }
    }
    //printf("%d\n",c); fflush(stdout);
  }

  int ListLayout::findVerticalStart( long comparator , VisualComponent* owner)
  {
    int Low = 0; int High = owner->componentSize()-1;  int Mid=High; int w=0;
    int z=0;
    VisualComponent* cont = owner;
    while( Low <= High ) {
      z++;
      Mid = ( Low + High ) / 2;
      VisualComponent* b = (VisualComponent*) owner->components.at(Mid);
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

  void ListLayout::setAlign( int align )
  {
    align_ = align;
  }

  void ListLayout::setIdent( const Point& ident ) {
    ident_ = ident;
  }

  const Point& ListLayout::ident() const {
    return ident_;
  }

}
