/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "flipbar.h"
#include "alignlayout.h"
#include "image.h"
#include "bitmap.h"

namespace ngrs {

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
    "            "
  };

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
    "            "
  };



  FlipBar::NFlipper::NFlipper( FlipBar * flipBar ) : expanded_(false), flipBar_(flipBar)
  {
    setLayout( AlignLayout() );

    expandBmp.createFromXpmData(expand_xpm);
    expandedBmp.createFromXpmData(expanded_xpm);

    expandImg_ = new Image();
    expandImg_->setVAlign(nAlCenter);
    expandImg_->setHAlign(nAlCenter);
    expandImg_->setSharedBitmap(&expandBmp);
    add(expandImg_, nAlClient);
  }

  FlipBar::NFlipper::~ NFlipper( )
  {
  }

  void FlipBar::NFlipper::onMousePress( int x, int y, int button )
  {
    setExpanded(!expanded_);
    flipBar_->onFlipClick();
  }

  void FlipBar::NFlipper::setExpanded( bool on )
  {
    expanded_ = on;
    if (expanded_)
      expandImg_->setSharedBitmap(&expandedBmp);
    else
      expandImg_->setSharedBitmap(&expandBmp);

  }


  int FlipBar::NFlipper::preferredWidth( ) const
  {
    return expandImg_->preferredWidth();
  }

  int FlipBar::NFlipper::preferredHeight( ) const
  {
    return expandImg_->preferredHeight();
  }


  // flipbar


  FlipBar::FlipBar()
    : Panel()
  {
    setLayout( AlignLayout() );

    flipper_ = new NFlipper(this);
    add(  flipper_, nAlLeft);

    header_ = new Panel();
    header_->setLayout( AlignLayout() );
    add( header_ , nAlClient);
  }


  FlipBar::~FlipBar()
  {
  }

  void FlipBar::onFlipClick( )
  {
    change.emit(this);
    repaint();
  }

  Panel* FlipBar::header()
  {
    return header_;
  }

  int FlipBar::flipperWidth( ) const
  {
    return flipper_->preferredWidth();
  }

  int  FlipBar::flipperHeight() const {
    return flipper_->preferredHeight();
  }

  void FlipBar::setExpanded( bool on )
  {
    flipper_->setExpanded(on);
  }

  bool FlipBar::expanded( ) const
  {
    return flipper_->expanded_;
  }

}
