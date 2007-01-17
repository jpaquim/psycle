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
#include "table.h"
#include "tablelayout.h"

namespace ngrs {

  Table::Table()
    : ScrollBox()
  {
    tablePane = new Panel();
    tablePane->setLayout(TableLayout(10,10));
    tablePane->setClientSizePolicy(nVertical | nHorizontal);
    setScrollPane(tablePane);
  }


  Table::~Table()
  {
  }

  void Table::add( VisualComponent * comp, int col, int row,  bool update )
  {
    tablePane->add(comp,AlignConstraint(nAlLeft,col,row),update);
  }

  void Table::removeChilds( )
  {
    tablePane->removeChilds();
  }

}
