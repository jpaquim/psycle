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
#include "nscrollbox.h"
#include "nscrollbar.h"

NScrollBox::NScrollBox()
 : NPanel()
{
  init();
}


NScrollBox::~NScrollBox()
{
}

void NScrollBox::init( )
{
  horBar_ = new NScrollBar();
    horBar_->setOrientation(nHorizontal);
    horBar_->change.connect(this,&NScrollBox::onHPosChange);
  add( horBar_ );
  verBar_ = new NScrollBar();
    verBar_->setOrientation(nVertical);
    verBar_->change.connect(this,&NScrollBox::onVPosChange);
  add( verBar_ );
  scrollPane_ = 0;
}

void NScrollBox::resize( )
{
  horBar_->setPosition(0,clientHeight()-15,clientWidth(),15);
  int horOff = 0;
  if (horBar_->visible()) horOff = 15;
  verBar_->setPosition(clientWidth()-15,0,15,clientHeight()-horOff);
  if (scrollPane_!=0) scrollPane_->setPosition(0,0,clientWidth()-15, clientHeight()-horOff);
}

void NScrollBox::setScrollPane( NVisualComponent * scrollPane )
{
  add(scrollPane);
  scrollPane_ = scrollPane;
    verBar_->setControl( scrollPane_, nDy );
    horBar_->setControl( scrollPane_, nDx );
  resize();
}

NVisualComponent * NScrollBox::scrollPane( )
{
  return scrollPane_;
}

void NScrollBox::setHScrollBarPolicy( int policy )
{
  if ( policy & nNoneVisible ) {
    horBar_->setVisible(false);
  }
}

void NScrollBox::onVPosChange( NScrollBar * sender )
{

}

void NScrollBox::onHPosChange( NScrollBar * sender )
{

}

NScrollBar * NScrollBox::horBar( )
{
  return horBar_;
}

NScrollBar * NScrollBox::verBar( )
{
  return verBar_;
}


