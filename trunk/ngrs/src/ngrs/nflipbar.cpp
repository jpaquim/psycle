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
#include "ngrs/nflipbar.h"
#include "ngrs/nalignlayout.h"

// flipper

NFlipBar::NFlipper::NFlipper( NFlipBar * flipBar ) : expanded_(true), flipBar_(flipBar)
{
  setWidth(10);
  setHeight(10);
  setBackground(NColor(255,255,255));
  setTransparent(false);
}

NFlipBar::NFlipper::~ NFlipper( )
{
}

void NFlipBar::NFlipper::onMousePress( int x, int y, int button )
{
  expanded_ = !expanded_;
  flipBar_->onFlipClick();
}

void NFlipBar::NFlipper::paint( NGraphics * g )
{
  g->setForeground(NColor(0,0,0));
  if (expanded_) 
    drawPlus(g);
  else
    drawMinus(g);
}

void NFlipBar::NFlipper::drawMinus( NGraphics * g )
{
  int h2 = spacingHeight()/2;
  g->drawLine(2,h2,spacingWidth()-3,h2);
}

void NFlipBar::NFlipper::drawPlus( NGraphics * g )
{
  int w2 = (spacingWidth())/2 ;
  int h2 = (spacingHeight())/2;

  g->drawLine(w2,2,w2,spacingHeight()-3); 
  g->drawLine(2,h2,spacingWidth()-3,h2);
}

// flipbar


NFlipBar::NFlipBar()
 : NPanel()
{
  add( flipper_ = new NFlipper(this) );
}


NFlipBar::~NFlipBar()
{
}

void NFlipBar::onFlipClick( )
{
  change.emit(this);
  repaint();
}

void NFlipBar::resize( )
{
  flipper_->setPosition(0,(clientHeight()-flipper_->height())/2,flipper_->width(), flipper_->height());
}

bool NFlipBar::expanded( ) const
{
   return flipper_->expanded_;
}

int NFlipBar::preferredWidth( ) const
{
  return 20;
}








