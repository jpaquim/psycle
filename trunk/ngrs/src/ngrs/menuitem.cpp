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

#include "menuitem.h"
#include "menu.h"
#include "label.h"
#include "alignlayout.h"
#include "image.h"
#include "app.h"
#include "config.h"
#include "popupmenu.h"


namespace ngrs {

  /* XPM */
  const char * submenu_arrow_right_xpm[] = {
    "12 12 2 1",
    " 	c None",
    ".	c black",
    "            ",
    "            ",
    "    .       ",
    "    ..      ",
    "    ...     ",
    "    ....    ",
    "    ...     ",
    "    ..      ",
    "    .       ",
    "            ",
    "            ",
    "            "
  };


  MenuItem::MenuItem()
    : CustomMenuItem()
  {
    init();
  }

  MenuItem::MenuItem( const std::string & text )
  {
    init();
    captionLbl_->setText(text);
  }

  MenuItem::MenuItem( const std::string & text, const Bitmap & icon )
  {
    init();
    captionLbl_->setText(text);
    iconImg_->setBitmap(icon);
  }

  void MenuItem::init( )
  {
    setLayout(AlignLayout());

    iconImg_ = new Image();
    iconImg_->setPreferredSize(25,25);
    iconImg_->setVAlign(nAlCenter);
    iconImg_->setHAlign(nAlCenter);
    iconImg_->setSkin(App::config()->skin("mitemiconbg"));
    CustomMenuItem::add(iconImg_, nAlLeft);

    subMenuImg_ = new Image();
    subMenuImg_->setPreferredSize(25,25);
    CustomMenuItem::add(subMenuImg_, nAlRight);

    captionLbl_ = new Label();
    captionLbl_->setSpacing(4,0,0,0);
    captionLbl_->setVAlign(nAlCenter);
    CustomMenuItem::add(captionLbl_, nAlClient);

    menu_ = 0;
  }

  MenuItem::~MenuItem()
  {
  }

  void MenuItem::onMouseEnter( )
  {
    iconImg_->setTransparent(true);
    CustomMenuItem::onMouseEnter();

    if (menu_ && !menu_->mapped()) {
      menu_->popupMenu()->setPosition(window()->left()+window()->width(), window()->top()+absoluteTop(),100,100);
      menu_->popupMenu()->setVisible(true);
    }
  }

  void MenuItem::onMouseExit( )
  {
    if (menu_!=0 && (App::mouseOverWindow() == menu_->popupMenu())) {

    } else {

      iconImg_->setTransparent(false);
      CustomMenuItem::onMouseExit();

      if (menu_!=0 && menu_->popupMenu()->mapped()) menu_->popupMenu()->setVisible(false);

    }
  }

  void MenuItem::add( Menu * menu )
  {
    menu_ = menu;
    if (menu_) {
      subMenuImg_->setBitmap(Bitmap(submenu_arrow_right_xpm));
      subMenuImg_->setVAlign(nAlCenter);
      subMenuImg_->setHAlign(nAlCenter);
    } else {
      subMenuImg_->setBitmap(Bitmap());
    }
  }

}
