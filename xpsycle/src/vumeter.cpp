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
#include "vumeter.h"

VuMeter::VuMeter()
 : NPanel(), l_(1),r_(1)
{
  setTransparent(false);
  //setBackground(NColor(0,0,0));
}


VuMeter::~VuMeter()
{
}

void VuMeter::paint( NGraphics * g )
{
  int log_l=(int) (100*log10f(l_));
  int log_r=(int) (100*log10f(r_));
  log_l=log_l;
  if ( log_l < 0 )log_l=0;
  log_r=log_r;
  if ( log_r < 0 )log_r=0;

  g->setForeground(vu1);
  if (log_l) {
    g->fillRect(0,0,spacingWidth() - log_l/8, 4);
  }

  if (log_r) {
    g->fillRect(0,5,spacingWidth() - log_r/8 ,4);
  }
}

void VuMeter::setPegel( float l, float r )
{
  if(l<1) l=1;
  if(r<1) r=1;

  l_ = l;
  r_ = r;

}


