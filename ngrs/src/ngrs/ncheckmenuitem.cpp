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
  captionLbl_->setText(text);
}

void NCheckMenuItem::init( )
{
  setLayout(NAlignLayout());

  checkBox_ = new NCheckBox();
     checkBox_->setPreferredSize(25,25);
     checkBox_->setCheck(true);
     checkBox_->setSkin(NApp::config()->skin("mitemiconbg"));
     checkBox_->setEvents(false);
  add(checkBox_, nAlLeft);

  captionLbl_ = new NLabel();
     captionLbl_->setSpacing(4,0,0,0);
    captionLbl_->setVAlign(nAlCenter);
  add(captionLbl_, nAlClient);
}



NCheckMenuItem::~NCheckMenuItem()
{
}

void NCheckMenuItem::onMousePress( int x, int y, int button )
{
  NCustomMenuItem::onMousePress(x,y,button);
  checkBox_->setCheck(!checkBox_->checked());
}

void NCheckMenuItem::onMouseEnter( )
{
  checkBox_->setTransparent(true);
  NCustomMenuItem::onMouseEnter();
}

void NCheckMenuItem::onMouseExit( )
{
  checkBox_->setTransparent(false);
  NCustomMenuItem::onMouseExit();
}




