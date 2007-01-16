/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "checkbox.h"
#include "label.h"
#include "alignlayout.h"

namespace ngrs {

  NCheckBox::NCheckBox()
    : Panel()
  {
    init();
  }


  NCheckBox::NCheckBox( const std::string & text ) : Panel()
  {
    init();
    label_->setText(text);
  }

  void NCheckBox::init( )
  {
    label_ = new Label();
    add(label_);

    dx=dy=10;
    setWidth(100);
    setHeight(20);
    checked_=false;
  }


  NCheckBox::~NCheckBox()
  {
  }

  void NCheckBox::paint( Graphics& g )
  {
    dx = 5;
    dy = 3;
    g.setForeground(Color(255,255,255));
    g.fillRect(0+dx,0+dy,10,10);
    g.setForeground(Color(0,0,0));
    g.drawRect(0+dx,0+dy,10,10);

    if (checked_) drawCheck(g);
  }

  void NCheckBox::onMousePress( int x, int y, int button )
  {
    checked_ = !checked_;
    repaint();
  }

  void NCheckBox::drawCheck(Graphics& g)
  {
    dx = 5;
    dy = 3; // (spacingHeight()-10)/2;

    g.setForeground(Color(0,0,0));
    g.drawLine(dx+1,dy+1,dx+9,dy+9);
    g.drawLine(dx+1,dy+9,dx+9,dy+1);

  }


  void NCheckBox::setCheck( bool on )
  {
    checked_ = on;
  }

  bool NCheckBox::checked( ) const
  {
    return checked_;
  }

  void NCheckBox::setText( const std::string & text )
  {
    label_->setText(text);
  }

  const std::string &  NCheckBox::text( ) const
  {
    return label_->text();
  }

  int NCheckBox::preferredWidth( ) const
  {
    if (ownerSize()) return VisualComponent::preferredWidth();
    return 20 + label_->preferredWidth() + spacing().left()+spacing().right()+borderLeft()+borderRight();
  }

  int NCheckBox::preferredHeight( ) const
  {
    if (ownerSize()) return VisualComponent::preferredHeight();

    return label_->preferredHeight() + spacing().top()+spacing().bottom() +borderTop()+borderBottom();
  }

  void NCheckBox::resize( )
  {
    label_->setPosition(20,0,spacingWidth()-20,spacingHeight());
  }

  void NCheckBox::onMousePressed( int x, int y, int button )
  {
    ButtonEvent ev(this,x,y,button);
    clicked.emit(&ev);
  }

  void NCheckBox::setWordWrap( bool on )
  {
    label_->setWordWrap(on);
  }

  bool NCheckBox::wordWrap( ) const
  {
    return label_->wordWrap();
  }

}

// class factories
extern "C" ngrs::Object* createCheckBox() {
  return new ngrs::NCheckBox();
}

extern "C" void destroyCheckBox( ngrs::Object* p ) {
  delete p;
}