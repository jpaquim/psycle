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
#include "ntable.h"
#include "ntablelayout.h"

NTable::NTable()
 : NScrollBox()
{
  tablePane = new NPanel();
    tablePane->setLayout(NTableLayout(10,10));
    tablePane->setClientSizePolicy(nVertical | nHorizontal);
  setScrollPane(tablePane);
}


NTable::~NTable()
{
}

void NTable::add( NVisualComponent * comp, int col, int row,  bool update )
{
  tablePane->add(comp,NAlignConstraint(nAlLeft,col,row),update);
}

void NTable::removeChilds( )
{
  tablePane->removeChilds();
}


