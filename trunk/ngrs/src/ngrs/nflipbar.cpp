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
#include "nflipbar.h"
#include "nalignlayout.h"
#include "nimage.h"
#include "nbitmap.h"

// flipper

/* XPM */
const char * expanded_xpm[] = {
"12 6 2 1",
" 	c None",
".	c black",
"            ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            "};

/* XPM */
const char * expand_xpm[] = {
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



NFlipBar::NFlipper::NFlipper( NFlipBar * flipBar ) : expanded_(false), flipBar_(flipBar)
{
  expandBmp.createFromXpmData(expand_xpm);
  expandedBmp.createFromXpmData(expanded_xpm);

  expandImg_ = new NImage();
    expandImg_->setVAlign(nAlCenter);
    expandImg_->setHAlign(nAlCenter);
    expandImg_->setSharedBitmap(&expandBmp);
  add(expandImg_);
}

NFlipBar::NFlipper::~ NFlipper( )
{
}

void NFlipBar::NFlipper::onMousePress( int x, int y, int button )
{
  expanded_ = !expanded_;

  if (expanded_)
     expandImg_->setSharedBitmap(&expandedBmp);
  else
     expandImg_->setSharedBitmap(&expandBmp);

  flipBar_->onFlipClick();
}


int NFlipBar::NFlipper::preferredWidth( ) const
{
  return expandImg_->preferredWidth();
}

int NFlipBar::NFlipper::preferredHeight( ) const
{
  return expandImg_->preferredHeight();
}


// flipbar


NFlipBar::NFlipBar()
 : NPanel()
{
  setLayout( NAlignLayout() );

  flipper_ = new NFlipper(this);
  add(  flipper_, nAlLeft);

  header_ = new NPanel();
    header_->setLayout( NAlignLayout() );
  add( header_ , nAlClient);
}


NFlipBar::~NFlipBar()
{
}

void NFlipBar::onFlipClick( )
{
  change.emit(this);
  repaint();
}

NPanel* NFlipBar::header()
{
  return header_;
}

bool NFlipBar::expanded( ) const
{
   return flipper_->expanded_;
}

int NFlipBar::flipperWidth( ) const
{
  return flipper_->preferredWidth();
}













