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
#include "zoombar.h"
#include <ngrs/nbutton.h>
#include <ngrs/nslider.h>
#include <ngrs/nalignlayout.h>

ZoomBar::ZoomBar()
 : NPanel()
{
  init();
}

ZoomBar::~ZoomBar()
{
}

void ZoomBar::init( )
{
  orientation_ = nHorizontal;

  setLayout( NAlignLayout() );

  decBtn = new NButton("-");
    decBtn->setFlat(false);
    decBtn->setPreferredSize(15,15);
  add(decBtn, nAlLeft);

  incBtn = new NButton("+");
    incBtn->setFlat(false);
    incBtn->setPreferredSize(15,15);
  add(incBtn, nAlRight);

  zoomSlider = new NSlider();
    zoomSlider->setOrientation(nHorizontal);
    zoomSlider->setPreferredSize(100,15);
    zoomSlider->posChanged.connect(this, &ZoomBar::onPosChanged);
  add(zoomSlider, nAlClient);
}

void ZoomBar::setOrientation( int orientation )
{
  orientation_ = orientation;
}

int ZoomBar::orientation( ) const
{
  return orientation_;
}

void ZoomBar::onPosChanged( NSlider * slider, double pos )
{
  posChanged.emit(this, pos);
}

void ZoomBar::setRange( double min, double max )
{
  zoomSlider->setRange(min, max);
}

void ZoomBar::setPos( double pos )
{
  zoomSlider->setPos( pos );
}

double ZoomBar::pos( ) const
{
  return zoomSlider->pos();
}


