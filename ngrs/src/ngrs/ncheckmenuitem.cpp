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
#include "ngrs/ncheckmenuitem.h"
#include "ngrs/nlabel.h"
#include "ngrs/nmenu.h"
#include "ngrs/nbevelborder.h"
#include "ngrs/napp.h"
#include "ngrs/nconfig.h"
#include "ngrs/ncheckbox.h"
#include "ngrs/nbutton.h"

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
     checkBox_->setPreferredSize(20,20);
  add(checkBox_, nAlLeft);

  captionLbl_ = new NLabel();
  add(captionLbl_, nAlClient);
}


NCheckMenuItem::~NCheckMenuItem()
{
}




