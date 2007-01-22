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
#include "menu.h"
#include "custommenuitem.h"
#include "app.h"
#include "config.h"
#include "popupmenu.h"

namespace ngrs {

Menu::Menu()
 : CustomButton()
{
  init();
}

Menu::Menu( const std::string & text ) : CustomButton(text)
{
  init();
}

void Menu::init( )
{
  popupMenu_ = new PopupMenu();
	popupMenu_->setName("popup_menu");
  CustomButton::add(popupMenu_);

  popupMenu_->addMessageListener(this);

  btnOver_ = App::config()->skin("mbtnover");
  btnNone_ = App::config()->skin("mbtnnone");

  setSkin(btnNone_);

  hide_ = false;
}

Menu::~Menu()
{
}

CustomMenuItem* Menu::add( CustomMenuItem * item )
{
  popupMenu_->add(item);
  return item;
}

void Menu::onMouseEnter( )
{
  if ( !popupMenu_->mapped() ) {
    setSkin(btnOver_);
    repaint();

    Event ev(this, "ngrs_menu_enter");
    sendMessage(&ev);
  }
}

void Menu::onMouseExit( )
{
  Event ev(this, "ngrs_menu_exit");
  sendMessage(&ev);
}

void Menu::onMousePress( int x, int y, int button )
{
  if ( button == 1 && !(App::popupUnmapped_ && hide_) ) {
    Event ev(this, "ngrs_menu_press");
    sendMessage(&ev);
  } else {
     hide_ = false;
     setSkin(btnOver_);
     repaint();
  }
}

void Menu::onMessage( Event * ev )
{
  if (ev->text() == "ngrs_menu_expose" && ev->sender() == this) {
    setSkin(btnOver_);
    repaint();
    int winLeft = window()->left();
    int winTop  = window()->top();
    popupMenu_->setPosition(winLeft + absoluteLeft(), winTop + absoluteTop() + height() ,100,100);
    popupMenu_->setVisible(true);
  } else
  if (ev->text() == "ngrs_menu_expose" && !(ev->sender() == this)) {
    if (popupMenu_->mapped()) popupMenu_->setVisible(false);
  } else
  if (ev->text() == "ngrs_menu_hide") {
    popupMenu_->setVisible(false);
    setSkin(btnNone_);
    repaint();
    hide_ = true;
  } else
    if (ev->text() == "ngrs_menu_item_click") {
     Event ev1(this, "ngrs_menu_press");
     sendMessage(&ev1);
  } else 
  if (ev->text() == "ngrs_menu_key_left") {
     Event ev1(this, "ngrs_menu_key_left");
     sendMessage(&ev1);
  } else 
  if (ev->text() == "ngrs_menu_key_right") {
    Event ev1(this, "ngrs_menu_key_right");
    sendMessage(&ev1);
  }
}

bool Menu::mapped( ) const
{
  return popupMenu_->mapped();
}

PopupMenu * Menu::popupMenu( )
{
  return popupMenu_;
}

void Menu::removeChilds( )
{
  popupMenu_->removeChilds();
}

}
