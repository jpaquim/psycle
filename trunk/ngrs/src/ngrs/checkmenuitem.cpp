/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "checkmenuitem.h"
#include "label.h"
#include "menu.h"
#include "bevelborder.h"
#include "app.h"
#include "config.h"
#include "checkbox.h"
#include "button.h"

namespace ngrs {

  CheckMenuItem::CheckMenuItem()
    : CustomMenuItem()
  {
    init();
  }

  CheckMenuItem::CheckMenuItem( const std::string & text )
  {
    init();
    captionLbl_->setText(text);
  }

  void CheckMenuItem::init( )
  {
    setLayout(AlignLayout());

    checkBox_ = new NCheckBox();
    checkBox_->setPreferredSize(25,25);
    checkBox_->setCheck(true);
    checkBox_->setSkin(App::config()->skin("mitemiconbg"));
    checkBox_->setEvents(false);
    add(checkBox_, nAlLeft);

    captionLbl_ = new Label();
    captionLbl_->setSpacing(4,0,0,0);
    captionLbl_->setVAlign(nAlCenter);
    add(captionLbl_, nAlClient);
  }



  CheckMenuItem::~CheckMenuItem()
  {
  }

  void CheckMenuItem::onMousePress( int x, int y, int button )
  {
    CustomMenuItem::onMousePress(x,y,button);
    checkBox_->setCheck(!checkBox_->checked());
  }

  void CheckMenuItem::onMouseEnter( )
  {
    checkBox_->setTransparent(true);
    CustomMenuItem::onMouseEnter();
  }

  void CheckMenuItem::onMouseExit( )
  {
    checkBox_->setTransparent(false);
    CustomMenuItem::onMouseExit();
  }

  void CheckMenuItem::setCheck( bool on )
  {
    checkBox_->setCheck(on);
  }

  bool CheckMenuItem::checked( ) const
  {
    return checkBox_->checked();
  }

}
