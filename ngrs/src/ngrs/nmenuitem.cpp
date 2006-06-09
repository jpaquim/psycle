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

#include "nmenuitem.h"
#include "nmenu.h"
#include "nlabel.h"
#include "nalignlayout.h"
#include "nimage.h"
#include "napp.h"
#include "nconfig.h"

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

}

NMenuItem::~NMenuItem()
{
}

void NMenuItem::onMouseEnter( )
{
  iconImg_->setTransparent(true);
  NCustomMenuItem::onMouseEnter();
}

void NMenuItem::onMouseExit( )
{
  iconImg_->setTransparent(false);
  NCustomMenuItem::onMouseExit();
}



