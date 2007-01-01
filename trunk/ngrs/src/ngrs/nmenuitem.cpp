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

#include "nmenuitem.h"
#include "nmenu.h"
#include "nlabel.h"
#include "nalignlayout.h"
#include "nimage.h"
#include "napp.h"
#include "nconfig.h"
#include "npopupmenu.h"

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
"            "};


NMenuItem::NMenuItem()
 : NCustomMenuItem()
{
  init();
}

NMenuItem::NMenuItem( const std::string & text )
{
  init();
  captionLbl_->setText(text);
}

NMenuItem::NMenuItem( const std::string & text, const NBitmap & icon )
{
  init();
  captionLbl_->setText(text);
  iconImg_->setBitmap(icon);
}

void NMenuItem::init( )
{
  setLayout(NAlignLayout());

  iconImg_ = new NImage();
    iconImg_->setPreferredSize(25,25);
    iconImg_->setVAlign(nAlCenter);
    iconImg_->setHAlign(nAlCenter);
    iconImg_->setSkin(NApp::config()->skin("mitemiconbg"));
  NCustomMenuItem::add(iconImg_, nAlLeft);

  subMenuImg_ = new NImage();
    subMenuImg_->setPreferredSize(25,25);
  NCustomMenuItem::add(subMenuImg_, nAlRight);

  captionLbl_ = new NLabel();
    captionLbl_->setSpacing(4,0,0,0);
    captionLbl_->setVAlign(nAlCenter);
  NCustomMenuItem::add(captionLbl_, nAlClient);

  menu_ = 0;
}

NMenuItem::~NMenuItem()
{
}

void NMenuItem::onMouseEnter( )
{
  iconImg_->setTransparent(true);
  NCustomMenuItem::onMouseEnter();

  if (menu_ && !menu_->mapped()) {
     menu_->popupMenu()->setPosition(window()->left()+window()->width(), window()->top()+absoluteTop(),100,100);
     menu_->popupMenu()->setVisible(true);
  }
}

void NMenuItem::onMouseExit( )
{
  if (menu_!=0 && (NApp::mouseOverWindow() == menu_->popupMenu())) {

  } else {

    iconImg_->setTransparent(false);
    NCustomMenuItem::onMouseExit();

     if (menu_!=0 && menu_->popupMenu()->mapped()) menu_->popupMenu()->setVisible(false);

  }
}

void NMenuItem::add( NMenu * menu )
{
  menu_ = menu;
  if (menu_) {
    subMenuImg_->setBitmap(NBitmap(submenu_arrow_right_xpm));
    subMenuImg_->setVAlign(nAlCenter);
    subMenuImg_->setHAlign(nAlCenter);
  } else {
    subMenuImg_->setBitmap(NBitmap());
  }
}

