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

void NMenuItem::init( )
{
  setLayout(NAlignLayout());

  iconImg_ = new NImage();
  iconImg_->setPreferredSize(20,20);
  NCustomMenuItem::add(iconImg_, nAlLeft);

  subMenuImg_ = new NImage();
  subMenuImg_->setPreferredSize(20,20);
  NCustomMenuItem::add(subMenuImg_, nAlRight);

  captionLbl_ = new NLabel();
  NCustomMenuItem::add(captionLbl_, nAlClient);

}

NMenuItem::~NMenuItem()
{
}




