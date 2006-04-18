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
#include "nflipbox.h"
#include "nflipbar.h"
#include "nalignlayout.h"

NFlipBox::NFlipBox()
 : NPanel()
{
  setLayout(new NAlignLayout(), true);

  pane_ = 0;

  add (flipBar_ = new NFlipBar() , nAlLeft);
  flipBar_->change.connect(this,&NFlipBox::onFlipChange);

}


NFlipBox::~NFlipBox()
{
}

int NFlipBox::preferredHeight( ) const
{
   if (flipBar_->expanded())
      return (pane_) ? pane_->preferredHeight() : minimumHeight();
   else
      return (pane_) ? pane_->minimumHeight() : minimumHeight();
}

int NFlipBox::preferredWidth( ) const
{
  return flipBar_->preferredWidth() + (!pane_ ? 0 : pane_->preferredWidth());
}

void NFlipBox::onFlipChange( NFlipBar * sender )
{
  setHeight(preferredHeight());
  ((NVisualComponent*)parent())->resize();
  ((NVisualComponent*)parent())->repaint();
}

void NFlipBox::setFlipPane( NPanel * pane )
{
  pane_ = pane;
  add (pane , nAlTop);
}


