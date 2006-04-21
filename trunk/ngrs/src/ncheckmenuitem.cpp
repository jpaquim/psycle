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
#include "ncheckmenuitem.h"
#include "nlabel.h"
#include "nmenu.h"
#include "nbevelborder.h"
#include "napp.h"
#include "nconfig.h"
#include "ncheckbox.h"
#include "nbutton.h"

NCheckMenuItem::NCheckMenuItem()
 : NCustomMenuItem()
{
  init();
}

NCheckMenuItem::NCheckMenuItem( const std::string & text )
{
  init();
  caption_->setText(text);
}


NCheckMenuItem::~NCheckMenuItem()
{

}

void NCheckMenuItem::init( )
{
  checkBox_ = new NCheckBox();
  checkBox_->setCheck(true);
  checkBox_->clicked.connect(this,&NCheckMenuItem::onCheckBoxClicked);
  checkBox_->mouseEnter.connect(this,&NCheckMenuItem::onCheckBoxMouseEnter);
  checkBox_->mouseExit.connect(this,&NCheckMenuItem::onCheckBoxMouseExit);
  add(checkBox_);

  caption_ = new NLabel();
  add(caption_);

  NApp::config()->setSkin(&itemNone,"mitemnone");
  NApp::config()->setSkin(&itemOver,"mitemover");

  setSkin(itemNone);
}


int NCheckMenuItem::maxIconSize( )
{
  return 20;
}

int NCheckMenuItem::maxCaptionSize( )
{
  return caption_->preferredWidth();
}

int NCheckMenuItem::maxShortCutSize( )
{
  return 20;
}

int NCheckMenuItem::maxHintSize( )
{
  return 20;
}

int NCheckMenuItem::preferredWidth( ) const
{
  return checkBox_->preferredWidth() + caption_->preferredWidth() +15+ 20+20+20;
}

int NCheckMenuItem::preferredHeight( ) const
{
  return checkBox_->preferredHeight();
}

void NCheckMenuItem::onCheckBoxClicked( NButtonEvent * ev )
{
  NApp::unmapPopupWindows();
  NButtonEvent ev1(this,0,0,0,caption_->text());
  click.emit(&ev1);
}

void NCheckMenuItem::onCheckBoxMouseEnter( NEvent * ev )
{
  setSkin(itemOver);
  repaint();
}

void NCheckMenuItem::onCheckBoxMouseExit( NEvent * ev )
{
  setSkin(itemNone);
  repaint();
}

void NCheckMenuItem::setIconSize( int size )
{
  checkBox_->setLeft(0);
  checkBox_->setWidth(size);
}

void NCheckMenuItem::setCaptionSize( int size )
{
  caption_->setWidth(size);
  caption_->setLeft(checkBox_->width() + 10);
}

void NCheckMenuItem::setShortCutSize( int size )
{
   caption_->setWidth(caption_->width()+size);
}

void NCheckMenuItem::setHintSize( int size )
{
  caption_->setWidth(caption_->width()+size);
}

void NCheckMenuItem::onMouseEnter( )
{
  NCustomMenuItem::onMouseEnter();
  setSkin(itemOver);
  repaint();
}


void NCheckMenuItem::onMouseExit( )
{
  NCustomMenuItem::onMouseExit();
  setSkin(itemNone);
  repaint();
}

void NCheckMenuItem::setHeight( int height )
{
  caption_->setHeight(height);
  checkBox_->setHeight(height);
}






