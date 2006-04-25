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
#include "global.h"
#include "configuration.h"
#include "machine.h"
#include <cmath>


VuMeter::VuMeter()
 : NPanel(), l_(1),r_(1)
{
  setTransparent(false);

  vuprevR = 0;
  vuprevL = 0;

  vu1 = Global::pConfig()->vu1;
  vu2 = Global::pConfig()->vu2;
  vu3 = Global::pConfig()->vu3;

}


VuMeter::~VuMeter()
{
}

void VuMeter::paint( NGraphics * g )
{
  /*bool clip = ((Master*)Global::pSong()->_pMachine[MASTER_INDEX])->_clip;


  if (clip) {
    g->setForeground(vu3);
    g->fillRect(0,0,9,20);
  } else  {
    g->setForeground(vu2);
    g->fillRect(0,0,9,20);
  } */

  int log_l=(int) (100*std::log10(l_));
  int log_r=(int) (100*std::log10(r_));

  log_l= std::max( log_l - 225 , 0);
  log_r= std::max( log_r - 255 , 0);


  if (log_r || vuprevR) {
     g->setForeground(vu1);
     g->fillRect(0,5,log_r,4);

     if (vuprevR > log_r ) {
        g->setForeground(vu3);
        g->fillRect(log_r,5,vuprevR-log_r,4);
        g->setForeground(vu2);
        g->fillRect(vuprevR,5,225-vuprevR,4);
        vuprevR-=2;
     } else
     {
        g->setForeground(vu2);
        g->fillRect(log_r,5,225-log_r,4);
        vuprevR = log_r;
     }
   }
   else {
     g->setForeground(vu2);
     g->fillRect(0,5,225,4);
   }

   if (log_l || vuprevL) {
    g->setForeground(vu1);
    g->fillRect(0,0,log_l, 4);
    if (vuprevL > log_l ) {
      g->setForeground(vu3);
      g->fillRect(log_l,0,vuprevL-log_l,4);
      g->setForeground(vu2);
      g->fillRect(vuprevL,0,225-vuprevL,4);
      vuprevL-=2;
    } else {
       g->setForeground(vu2);
       g->fillRect(log_l,0,225-log_l,4);
       vuprevL = log_l;
    }
  }
  else {
    g->setForeground(vu2);
    g->fillRect(0,0,225,4);
  }

}

void VuMeter::setPegel( float l, float r )
{
  if(l<1) l=1;
  if(r<1) r=1;

  l_ = l;
  r_ = r;

}


