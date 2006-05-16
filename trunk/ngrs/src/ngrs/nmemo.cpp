/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "nmemo.h"
#include "nalignlayout.h"
#include "nlistlayout.h"
#include "nscrollbox.h"
#include "nedit.h"

NMemo::NMemo()
 : NTextBase()
{
  setLayout(NAlignLayout());
  NScrollBox* box = new NScrollBox();
    scrollPane_ = new NPanel();
       scrollPane_->setClientSizePolicy(nVertical | nHorizontal);
       scrollPane_->setLayout(NListLayout());
    box->setScrollPane(scrollPane_);
  add(box, nAlClient);

  clear();
}


NMemo::~NMemo()
{
}

std::string NMemo::text( ) const
{
  return "";
}

void NMemo::clear( )
{
  NEdit* edt = new NEdit();
    edt->setAutoSize(true);
    edt->keyPress.connect(this, &NMemo::onKeyPress);
  scrollPane_->add(edt);
}

void NMemo::onKeyPress( const NKeyEvent & event )
{
  if (event.scancode() == XK_Return) {
   NEdit* edt = new NEdit();
    edt->setAutoSize(true);
    edt->keyPress.connect(this, &NMemo::onKeyPress);
   scrollPane_->add(edt);
   scrollPane_->repaint();
  }
}


