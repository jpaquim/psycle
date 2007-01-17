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
#include "borderlayout.h"
#include "visualcomponent.h"

namespace ngrs {

  BorderLayout::BorderLayout()
    : Layout()
  {
  }


  BorderLayout::~BorderLayout()
  {
  }

  BorderLayout * BorderLayout::clone( ) const
  {
    return new BorderLayout(*this);
  }

  void BorderLayout::align( VisualComponent * parent )
  {
    VisualComponent* lastTop    = 0;
    VisualComponent* lastLeft   = 0;
    VisualComponent* lastRight  = 0;
    VisualComponent* lastBottom = 0;
    VisualComponent* lastClient = 0;

    std::vector<VisualComponent*>::const_iterator itr = parent->visualComponents().begin();

    for (;itr < parent->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      switch (visualChild->align()) {
       case nAlTop    : lastTop    = visualChild;  break;
       case nAlLeft   : lastLeft   = visualChild;  break;
       case nAlRight  : lastRight  = visualChild;  break;
       case nAlBottom : lastBottom = visualChild;  break;
       case nAlClient : lastClient = visualChild;  break;
      }
    }


    if (lastTop != 0) {
      lastTop->setWidth(parent->clientWidth());
      lastTop->setTop(0);
      lastTop->setLeft(0);
      lastTop->setHeight(lastTop->preferredHeight());
    }
    if (lastBottom != 0) {
      lastBottom->setWidth(parent->width());
      int pH = lastBottom->preferredHeight();
      lastBottom->setTop(parent->clientHeight()-pH);
      lastBottom->setLeft(0);
      lastBottom->setHeight(pH);
    }
    if (lastLeft != 0) {
      lastLeft->setTop( (lastTop!=0) ? lastTop->height() : 0);
      lastLeft->setLeft(0);
      int tOff = ((lastTop    !=0) ? lastTop->preferredHeight() : 0);
      int bOff = ((lastBottom !=0) ? lastBottom->preferredHeight() : 0);
      lastLeft->setHeight(parent->clientHeight() - tOff -bOff);
      lastLeft->setWidth(lastLeft->preferredWidth());
    }

    if (lastRight != 0) {
      lastRight->setTop( (lastTop!=0) ? lastTop->height() : 0);
      int tOff = ((lastTop    !=0) ? lastTop->preferredHeight() : 0);
      int bOff = ((lastBottom !=0) ? lastBottom->preferredHeight() : 0);
      lastRight->setHeight(parent->clientHeight() - tOff -bOff);
      int pW = lastRight->preferredWidth();
      lastRight->setLeft(parent->clientWidth() - pW);
      lastRight->setWidth(pW);
    }

    if (lastClient != 0) {
      int tOff = ((lastTop    !=0) ? lastTop->preferredHeight() : 0);
      int bOff = ((lastBottom !=0) ? lastBottom->preferredHeight() : 0);
      int rOff = ((lastRight  !=0) ? lastRight->preferredWidth() : 0);
      int lOff = ((lastLeft   !=0) ? lastLeft->preferredWidth()  : 0);

      lastClient->setLeft( lOff );
      lastClient->setTop(  tOff );
      lastClient->setWidth( parent->clientWidth()  - rOff - lOff);
      lastClient->setHeight(parent->clientHeight() - tOff - bOff );
    }

  }

  int BorderLayout::preferredWidth( const VisualComponent * target ) const
  {
    VisualComponent* lastLeft   = 0;
    VisualComponent* lastRight  = 0;
    VisualComponent* lastClient = 0;

    std::vector<VisualComponent*>::const_iterator itr = parent()->visualComponents().begin();

    for (;itr < parent()->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      switch (visualChild->align()) {
       case nAlLeft   : lastLeft   = visualChild;  break;
       case nAlRight  : lastRight  = visualChild;  break;
       case nAlClient : lastClient = visualChild;  break;
      }
    }


    int rOff = ((lastRight  !=0) ? lastRight->preferredWidth()   : 0);
    int lOff = ((lastLeft   !=0) ? lastLeft  ->preferredWidth()  : 0);
    int cOff = ((lastClient !=0) ? lastClient->preferredWidth()  : 0);

    std::cout << rOff + lOff + cOff << "," << lOff << "," << rOff << "," << cOff << std::endl;

    return rOff + lOff + cOff;
  }

  int BorderLayout::preferredHeight( const VisualComponent * target ) const
  {
    VisualComponent* lastTop    = 0;
    VisualComponent* lastBottom = 0;
    VisualComponent* lastClient = 0;

    std::vector<VisualComponent*>::const_iterator itr = parent()->visualComponents().begin();

    for (;itr < parent()->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      switch (visualChild->align()) {
       case nAlTop    : lastTop    = visualChild;  break;
       case nAlBottom : lastBottom = visualChild;  break;
       case nAlClient : lastClient = visualChild;  break;
      }
    }

    int tOff = ((lastTop    !=0) ? lastTop->preferredHeight()    : 0);
    int bOff = ((lastBottom !=0) ? lastBottom->preferredHeight() : 0);
    int cOff = ((lastClient !=0) ? lastClient->preferredHeight() : 0);

    return tOff + bOff + cOff;
  }

}