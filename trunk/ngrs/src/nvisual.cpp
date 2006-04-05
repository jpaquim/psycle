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
#include "nvisual.h"

NVisual::NVisual()
 : NRuntime(), geometry_(0),visible_(true),minWidth_(10),minHeight_(10)
{
}


NVisual::~NVisual()
{
}

void NVisual::setVisible( bool on )
{
  visible_ = on;
}

int NVisual::visible( )
{
  return visible_;
}

void NVisual::onMousePress( int x, int y, int button )
{
  NButtonEvent ev(this,x,y,button);
  mousePress.emit(&ev);
}

void NVisual::onMousePressed( int x, int y, int button )
{
  NButtonEvent ev(this,x,y,button);
  mousePressed.emit(&ev);
}

void NVisual::onMouseDoublePress( int x, int y, int button )
{
}

void NVisual::setPosition( int x, int y, int width, int height )
{
  if (geometry_!=0) {
     geometry_->setPosition(x,y,width,height);
     resize();
  }
}

void NVisual::setPosition( const NRect & pos )
{
  setPosition(pos.left(),pos.top(),pos.width(),pos.height());
}

NShape * NVisual::geometry( )
{
  return geometry_;
}

void NVisual::setGeometry( NShape * geometry )
{
  geometry_ = geometry;
}

long NVisual::top( ) const
{
  return geometry_->rectArea().top();
}

long NVisual::left( ) const
{
  return geometry_->rectArea().left();
}

void NVisual::resize( )
{
}

int NVisual::preferredWidth( ) const
{
  return width();
}

int NVisual::preferredHeight( ) const
{
  return height();
}

int NVisual::minimumHeight( ) const
{
  return minHeight_;
}

int NVisual::minimumWidth( ) const
{
  return minWidth_;
}

void NVisual::onMouseOver( int x, int y )
{
}

int NVisual::absoluteLeft( ) const
{
  return 0;
}

int NVisual::absoluteTop( ) const
{
  return 0;
}

int NVisual::width( ) const
{
  return geometry_->width();
}

int NVisual::height( ) const
{
  return geometry_->height();
}

void NVisual::setTop( int top )
{
   if (geometry_!=0) {
     geometry_->setTop(top);
     resize();
  }
}

void NVisual::setWidth( int width )
{
   if (geometry_!=0) {
     geometry_->setWidth( (width < minWidth_) ? minWidth_ : width);
     resize();
  }
}

void NVisual::setHeight( int height )
{
  if (geometry_!=0) {
     geometry_->setHeight(height);
     resize();
  }
}

void NVisual::setLeft( int left )
{
  if (geometry_!=0) {
     geometry_->setLeft(left);
     resize();
  }
}

void NVisual::onMouseExit( )
{
  NEvent ev(this);
  mouseExit.emit(&ev);
}

void NVisual::onMouseEnter( )
{
  NEvent ev(this);
  mouseEnter.emit(&ev);
}

void NVisual::onKeyPress( const NKeyEvent & event )
{
}

bool NVisual::mausin( int x, int y, const NRect & a ) const
{
  if ((a.left() <= x) && (a.width()+a.left() >= x) && (a.top()  <= y) 
                       && (a.height()+a.top() >= y)) return true; else return false;
}

void NVisual::setMinimumWidth( int minWidth )
{
  minWidth_ = minWidth;
}

void NVisual::setMinimumHeight( int minHeight )
{
  minHeight_ = minHeight;
}

NRect NVisual::clipBox( ) const
{
  NRect clip(left(),top(),width(),height());
  return clip;
}

NShape * NVisual::geometry( ) const
{
  return geometry_;
}

void NVisual::setPreferredSize( int width, int height )
{

}

void NVisual::setFocus( )
{
}

bool NVisual::focus( ) const
{
  return true;
}

void NVisual::onFocus( )
{
}









