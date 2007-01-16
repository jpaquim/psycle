/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "custombutton.h"
#include "label.h"
#include "bevelborder.h"
#include "gradient.h"
//#include "property.h"

namespace ngrs {

  CustomButton::CustomButton()
    : Panel()
  {
    init();
  }

  CustomButton::CustomButton( const std::string & text ) : Panel()
  {
    init();
    label_->setText(text);
  }

  void CustomButton::init( )
  {
    toggle_ = down_ = false;

    setTransparent(false);

    label_ = new Label();
    label_->setSpacing(4,2,4,2);
    label_->setHAlign(nAlCenter);
    label_->setVAlign(nAlCenter);
    label_->setEvents(false);
    add(label_);

    resize();

    // runtime
    //  if (properties()) properties()->registrate<std::string>("text", *this, &CustomButton::text, &CustomButton::setText);
  }

  CustomButton::~CustomButton()
  {

  }


  void CustomButton::setText( const std::string & text )
  {
    label_->setText(text);
  }


  void CustomButton::resize( )
  {
    label_->setPosition(0,0,spacingWidth(),spacingHeight());
  }

  int CustomButton::preferredWidth( ) const
  {
    return label_->preferredWidth() + spacing().left()+spacing().right()+borderLeft()+borderRight();
  }

  int CustomButton::preferredHeight( ) const
  {
    return label_->preferredHeight() + spacing().top()+spacing().bottom()+borderTop()+borderBottom();
  }

  void CustomButton::onMousePress( int x, int y, int button )
  {
    setDown(!down_);
    ButtonEvent ev(this,x,y,button,"btnpress");
    click.emit(&ev);
    sendMessage(&ev);
  }

  void CustomButton::onMousePressed( int x, int y, int button )
  {
    Panel::onMousePressed(x,y,button);
    if (!toggle_) {
      setDown(false);
    }
    if (Rect(0,0,width(),height()).intersects(x,y)) {
      ButtonEvent ev(this,x,y,button,"btnpressed");
      clicked.emit(&ev);
      sendMessage(&ev);
    }
  }


  const std::string & CustomButton::text( ) const
  {
    return label_->text();
  }

  char CustomButton::mnemonic( )
  {
    return label_->mnemonic();
  }

  void CustomButton::setMnemonic(char c )
  {
    label_->setMnemonic(c);
  }


  bool CustomButton::toggle( ) const
  {
    return toggle_;
  }

  bool CustomButton::down( ) const
  {
    return down_;
  }

  void CustomButton::setToggle( bool on )
  {
    toggle_ = on;
  }

  void CustomButton::setDown( bool on )
  {
    down_ = on;
  }

  void CustomButton::onMessage( Event * ev )
  {
    if (toggle() && ev->text() == "toggle:'up'") {
      setDown(false);
    }
  }

  void CustomButton::setTextHAlign( int align )
  {
    label_->setHAlign(align);
  }

  void CustomButton::setTextVAlign( int align )
  {
    label_->setVAlign(align);
  }

  Label * CustomButton::label( )
  {
    return label_;
  }

  Label * CustomButton::label( ) const
  {
    return label_;
  }

}
