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
#include "ntoolbarseparator.h"
#include "nflowlayout.h"

NToolBarSeparator::NToolBarSeparator()
 : NPanel()
{
  setTransparent(false);
  setAlign(nAlHeight);
}


NToolBarSeparator::~NToolBarSeparator()
{
}

void NToolBarSeparator::paint( NGraphics * g )
{
  g->setForeground(NColor(background().red() - 40,background().green() - 40, background().blue()-40));
  g->drawLine(0,0,0,clientHeight());
  g->setForeground(NColor(background().red() + 40,background().green() + 40, background().blue()+40));
  g->drawLine(1,0,1,clientHeight());
}

int NToolBarSeparator::preferredWidth( ) const
{
  return 3;
}

int NToolBarSeparator::preferredHeight( ) const
{
  return 15;
}


