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
#include "nmenu.h"
#include "ncustommenuitem.h"
#include "napp.h"
#include "nconfig.h"
#include "npopupmenu.h"

NMenu::NMenu()
 : NCustomButton()
{
  init();
}

NMenu::NMenu( const std::string & text ) : NCustomButton(text)
{
  init();
}

void NMenu::init( )
{
  popupMenu_ = new NPopupMenu();
  NCustomButton::add(popupMenu_);

  popupMenu_->addMessageListener(this);

  NApp::config()->setSkin(&btnOver_,"mbtnover");
  NApp::config()->setSkin(&btnNone_,"mbtnnone");

  setSkin(btnNone_);
}

NMenu::~NMenu()
{
}

NCustomMenuItem* NMenu::add( NCustomMenuItem * item )
{
  popupMenu_->add(item);
  item->addMessageListener(popupMenu_);
  return item;
}

void NMenu::onMouseEnter( )
{
  setSkin(btnOver_);
  repaint();

  NEvent ev(this, "ngrs_menu_enter");
  sendMessage(&ev);
}

void NMenu::onMouseExit( )
{
  NEvent ev(this, "ngrs_menu_exit");
  sendMessage(&ev);
}

void NMenu::onMousePress( int x, int y, int button )
{
  if ( button == 1 ) {
    NEvent ev(this, "ngrs_menu_press");
    sendMessage(&ev);
  }
}

void NMenu::onMessage( NEvent * ev )
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
     if (popupMenu_->mapped()) popupMenu_->setVisible(false);
  } else 
    if (ev->text() == "ngrs_menu_item_click") {
     NEvent ev(this, "ngrs_menu_press");
     sendMessage(&ev);
  } else 
  if (ev->text() == "ngrs_menu_key_left") {
     NEvent ev(this, "ngrs_menu_key_left");
     sendMessage(&ev);
  } else 
  if (ev->text() == "ngrs_menu_key_right") {
    NEvent ev(this, "ngrs_menu_key_right");
    sendMessage(&ev);
  }
}





