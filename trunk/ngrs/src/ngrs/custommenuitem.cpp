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
#include "custommenuitem.h"
#include "app.h"
#include "config.h"
#include "menu.h"

namespace ngrs {

  CustomMenuItem::CustomMenuItem()
    : CustomItem()
  {
    itemNone = App::config()->skin("mitemnone");
    itemOver = App::config()->skin("mitemover");

    setSkin(itemNone);
  }


  CustomMenuItem::~CustomMenuItem()
  {
  }

  void CustomMenuItem::add( Menu * menu )
  {

  }

  void CustomMenuItem::add( Runtime * comp )
  {
    CustomItem::add(comp);
  }

  void CustomMenuItem::add( VisualComponent * comp, int align )
  {
    CustomItem::add(comp, align);
  }

  void CustomMenuItem::onMouseEnter( )
  {
    setSkin(itemOver);
    repaint();

    Event ev(this,"ngrs_menu_item_enter");
    sendMessage(&ev);
  }

  void CustomMenuItem::onMouseExit( )
  {
    setSkin(itemNone);
    repaint();

    Event ev(this,"ngrs_menu_item_exit");
    sendMessage(&ev);
  }

  void CustomMenuItem::onMousePress( int x, int y, int button )
  {
    ButtonEvent ev(this,x,y,button,"ngrs_menu_item_click");
    sendMessage(&ev);

    click.emit(&ev);
  }

  void CustomMenuItem::onMessage( Event * ev )
  {
    if (ev->text() == "ngrs_menu_item_do_enter") {
      onMouseEnter();
    } else
      if (ev->text() == "ngrs_menu_item_do_exit") {
        onMouseExit();
      }
  }

  void CustomMenuItem::setFont( const NFont & font )
  {
    itemNone.setFont(font);
    itemOver.setFont(font);
  }

}
