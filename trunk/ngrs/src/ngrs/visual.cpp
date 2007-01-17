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
#include "visual.h"
#include "property.h"
#include "app.h"
#include "system.h"

namespace ngrs {

  Visual::Visual()
    : Runtime(), geometry_(0),visible_(true),minWidth_(10),minHeight_(10), cursorId_( nCrDefault )
  {
    // if (properties()) properties()->bind("left", *this, &Visual::left, &Visual::setLeft);

  }


  Visual::~Visual()
  {
  }

  void Visual::setVisible( bool on )
  {
    visible_ = on;
  }

  bool Visual::visible( ) const
  {
    return visible_;
  }

  void Visual::setCursor( int crIdentifier ) {
    cursorId_ = crIdentifier;
  }

  int Visual::cursor() const {
    return cursorId_;
  }

  void Visual::onMousePress( int x, int y, int button )
  {
    ButtonEvent ev(this,x,y,button);
    mousePress.emit(&ev);
  }

  void Visual::onMousePressed( int x, int y, int button )
  {
    ButtonEvent ev(this,x,y,button);
    mousePressed.emit(&ev);
  }

  void Visual::onMouseDoublePress( int x, int y, int button )
  {
    ButtonEvent ev(this,x,y,button);
    mouseDoublePress.emit(&ev);
  }

  void Visual::setPosition( int x, int y, int width, int height )
  {
    if (geometry_!=0) {
      geometry_->setPosition(x,y,width,height);
      resize();
    }
  }

  void Visual::setPosition( const Rect & pos )
  {
    setPosition(pos.left(),pos.top(),pos.width(),pos.height());
  }

  Shape * Visual::geometry( )
  {
    return geometry_;
  }

  void Visual::setGeometry( Shape * geometry )
  {
    geometry_ = geometry;
  }

  int Visual::top( ) const
  {
    return geometry_->rectArea().top();
  }

  int Visual::left( ) const
  {
    return geometry_->rectArea().left();
  }

  void Visual::resize( )
  {
  }

  int Visual::preferredWidth( ) const
  {
    return width();
  }

  int Visual::preferredHeight( ) const
  {
    return height();
  }

  int Visual::minimumHeight( ) const
  {
    return minHeight_;
  }

  int Visual::minimumWidth( ) const
  {
    return minWidth_;
  }

  void Visual::onMouseOver( int x, int y )
  {
  }

  int Visual::absoluteLeft( ) const
  {
    return 0;
  }

  int Visual::absoluteTop( ) const
  {
    return 0;
  }

  int Visual::width( ) const
  {
    return geometry_->width();
  }

  int Visual::height( ) const
  {
    return geometry_->height();
  }

  void Visual::setTop( int top )
  {
    if (geometry_!=0) {
      geometry_->setTop(top);
      resize();
    }
  }

  void Visual::setWidth( int width )
  {
    if (geometry_!=0) {
      geometry_->setWidth( (width < minWidth_) ? minWidth_ : width);
      resize();
    }
  }

  void Visual::setHeight( int height )
  {
    if (geometry_!=0) {
      geometry_->setHeight(height);
      resize();
    }
  }

  void Visual::setLeft( int left )
  {
    if (geometry_!=0) {
      geometry_->setLeft(left);
      resize();
    }
  }

  void Visual::onMouseExit( )
  {
    Event ev(this);
    mouseExit.emit(&ev);
  }

  void Visual::onMouseEnter( )
  {
    Event ev(this);
    mouseEnter.emit(&ev);
  }

  void Visual::onKeyPress( const KeyEvent & event )
  {
  }

  void Visual::setMinimumWidth( int minWidth )
  {
    minWidth_ = minWidth;
  }

  void Visual::setMinimumHeight( int minHeight )
  {
    minHeight_ = minHeight;
  }

  Rect Visual::clipBox( ) const
  {
    Rect clip(left(),top(),width(),height());
    return clip;
  }

  Shape * Visual::geometry( ) const
  {
    return geometry_;
  }

  void Visual::setPreferredSize( int width, int height )
  {

  }

  void Visual::setFocus( )
  {
  }

  bool Visual::focus( ) const
  {
    return true;
  }

  void Visual::onEnter( )
  {
  }

  void Visual::onExit( )
  {
  }

  void Visual::onKeyRelease( const KeyEvent & event )
  {
  }

  void Visual::setSize( int width, int height )
  {
  }

}
