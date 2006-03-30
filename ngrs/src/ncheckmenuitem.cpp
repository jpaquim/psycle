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
#include "ncheckbox.h"

NCheckMenuItem::NCheckMenuItem()
 : NCustomMenuItem()
{
  init();
}

void NCheckMenuItem::init( )
{
  setTransparent(true);
  iconField_ = new NPanel();
  iconField_->setEvents(false);
  iconField_->setWidth(20);
  iconField_->setBorder(0);
  iconField_->setTransparent(true);
  add(iconField_);
  caption_ = new NCheckBox();
  //caption_->setEvents(false);
  add(caption_);
  caption_->setWidth(caption_->preferredWidth());
  menu_ = 0;

  border_ = new NBevelBorder(nRaised,nLowered);
  setBorder(0);

  shortCut_ = new NLabel();
  shortCut_->setEvents(false);
  add(shortCut_);
  subMenuHint_ = new NLabel();
  add(subMenuHint_);
  subMenuHint_->setEvents(false);

  gap_ = 10;
}

NCheckMenuItem::~NCheckMenuItem()
{
  delete border_;
}

void NCheckMenuItem::setText( std::string text )
{
  caption_->setText(text);
  caption_->setWidth(caption_->preferredWidth());
  caption_->setHeight(caption_->preferredHeight());

}


int NCheckMenuItem::preferredHeight( ) const
{
  return caption_->height() + spacing().top() + spacing().bottom() + borderTop() + borderBottom();
}

void NCheckMenuItem::onMouseEnter( )
{
  NCustomMenuItem::onMouseEnter();
  setBorder(border_);
  repaint();
  if (menu_!=0 && !menu_->popupMenu()->mapped()) {
     menu_->popupMenu()->setPosition(window()->left()+window()->width(), window()->top()+absoluteTop(),100,100);
     menu_->popupMenu()->setVisible(true);
     subMenuMapped.emit(this);
  }
}


void NCheckMenuItem::onMouseExit( )
{
  if (menu_!=0 && (NApp::mouseOverWindow() == menu_->popupMenu())) {

  } else {
   NCustomMenuItem::onMouseExit();
   setBorder(0);
   repaint();
   if (menu_!=0 && menu_->popupMenu()->mapped()) menu_->popupMenu()->setVisible(false);
  }
}

void NCheckMenuItem::add( NMenu * menu )
{
  menu->setVisible(false);
  caption_->setText(menu->text());
  NVisualComponent::add(menu);
  menu_ = menu;
  subMenuHint_->setText(">");
}

void NCheckMenuItem::add( NRuntime* comp )
{
  NVisualComponent::add(comp);
}

NCheckMenuItem::NCheckMenuItem( std::string text )
{
  init();
  caption_->setText(text);
  caption_->setWidth(caption_->preferredWidth());
  caption_->setHeight(caption_->preferredHeight());
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
  return shortCut_->preferredWidth();;
}

int NCheckMenuItem::maxHintSize( )
{
  return 20;
}

void NCheckMenuItem::setIconSize( int size )
{
  iconField_->setWidth(size);
  iconField_->setLeft(0);
}

void NCheckMenuItem::setCaptionSize( int size )
{
  caption_->setWidth(size);
  caption_->setLeft(gap_ + iconField_->width());
}

void NCheckMenuItem::setShortCutSize( int size )
{
  shortCut_->setWidth(size);
  shortCut_->setLeft(2* gap_ + caption_->width()+iconField_->width());
}

void NCheckMenuItem::setHintSize( int size )
{
  subMenuHint_->setWidth(size);
  subMenuHint_->setLeft(3*gap_ + caption_->width()+iconField_->width()+shortCut_->width());
}

int NCheckMenuItem::preferredWidth( ) const
{
   return (caption_->width() + iconField_->width() + shortCut_->width() + subMenuHint_->width()) + (width() - clientWidth()) + 3*gap_;
}

void NCheckMenuItem::setHeight( int height )
{
   caption_->setHeight(height);
   shortCut_->setHeight(height);
   iconField_->setHeight(height);
   subMenuHint_->setHeight(height);
   NCustomMenuItem::setHeight(height);
}

void NCheckMenuItem::setShortCut( int keyMask, char c , std::string description  )
{
  shortCut_->setText(description);
  NApp::addKeyAccelerator(NKeyAccelerator(keyMask,c),this);
}

void NCheckMenuItem::setGap( int gap )
{
  gap_ = gap;
}

void NCheckMenuItem::onKeyAcceleratorNotify(NKeyAccelerator accelerator )
{
  NButtonEvent ev(this,0,0,0);
  click.emit(&ev);
}

void NCheckMenuItem::onMousePress( int x, int y, int button )
{
  NApp::unmapPopupWindows();
  NButtonEvent ev(this,x,y,button,text());
  click.emit(&ev);
}

void NCheckMenuItem::setMnemonic( char c )
{
  NCustomMenuItem::setMnemonic(c);
//  caption_->setMnemonic(c);
  caption_->setHeight(caption_->preferredHeight());
}

std::string NCheckMenuItem::text( )
{
  return caption_->text();
}






