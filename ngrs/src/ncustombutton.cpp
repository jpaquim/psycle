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
#include "ncustombutton.h"
#include "nlabel.h"
#include "nbevelborder.h"
#include <ngradient.h>


NCustomButton::NCustomButton()
 : NPanel()
{
  init();
}

NCustomButton::NCustomButton( const std::string & text ) : NPanel()
{
  init();
  label_->setText(text);
}

void NCustomButton::init( )
{
  toggle_ = down_ = false;

  setTransparent(false);

  gradient_ = new NGradient();
     gradient_->setColor_1(NColor(200,200,205));
     gradient_->setColor_2(NColor(230,230,255));
     gradient_->setColor_3(NColor(200,200,205));
     gradient_->setPercent(30);
     gradient_->setActive(false);
     gradient_->setHorizontal(false);
     gradient_->setEvents(false);
  add(gradient_);

  label_ = new NLabel();
    label_->setSpacing(4,2,4,2);
    label_->setHAlign(nAlCenter);
    label_->setVAlign(nAlCenter);
    label_->setEvents(false);
  add(label_);

  resize();
}

NCustomButton::~NCustomButton()
{

}


void NCustomButton::setText( std::string text )
{
  label_->setText(text);
}


void NCustomButton::resize( )
{
  gradient_->setPosition(0,0,spacingWidth(),spacingHeight());
  label_->setPosition(0,0,spacingWidth(),spacingHeight());
}

int NCustomButton::preferredWidth( ) const
{
  return label_->preferredWidth() + spacing().left()+spacing().right()+borderLeft()+borderRight();
}

int NCustomButton::preferredHeight( ) const
{
  return label_->preferredHeight() + spacing().top()+spacing().bottom()+borderTop()+borderBottom();
}

void NCustomButton::onMousePress( int x, int y, int button )
{
  setDown(!down_);
  NButtonEvent ev(this,x,y,button);
  click.emit(&ev);
}

void NCustomButton::onMousePressed( int x, int y, int button )
{
  NPanel::onMousePressed(x,y,button);
  if (!toggle_) {
    setDown(false);
  }
  if (mausin(x,y,NRect(0,0,width(),height()))) {
    NButtonEvent ev(this,x,y,button);
    clicked.emit(&ev);
  }
}


std::string NCustomButton::text( ) const
{
  return label_->text();
}

char NCustomButton::mnemonic( )
{
  return label_->mnemonic();
}

void NCustomButton::setMnemonic(char c )
{
  label_->setMnemonic(c);
}


bool NCustomButton::toggle( ) const
{
  return toggle_;
}

bool NCustomButton::down( ) const
{
  return down_;
}

void NCustomButton::setToggle( bool on )
{
  toggle_ = on;
}

void NCustomButton::setActiveGradient( bool on )
{
  gradient_->setActive(on);
}

void NCustomButton::setGradientStyle( const NColor & startCl, NColor & midCl, NColor & endCl , int percent )
{
  gradient_->setColor_1(startCl);
  gradient_->setColor_2(midCl);
  gradient_->setColor_3(endCl);
  gradient_->setPercent(percent);
}


void NCustomButton::setDown( bool on )
{
  down_ = on;
}

void NCustomButton::onCustomMessage( NEvent * ev )
{
  if (ev->text() == "toggle:'up'") {
    setDown(false);
  }
}

void NCustomButton::setTextHAlign( int align )
{
  label_->setHAlign(align);
}

void NCustomButton::setTextVAlign( int align )
{
  label_->setVAlign(align);
}



