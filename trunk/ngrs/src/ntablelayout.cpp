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
#include "ntablelayout.h"
#include "nvisualcomponent.h"

NTableLayout::NTableLayout()
 : NLayout(), cols_(1), rows_(1)
{
}


NTableLayout::~NTableLayout()
{
}

NTableLayout * NTableLayout::clone( ) const
{
  return new NTableLayout(*this);
}

void NTableLayout::align( NVisualComponent * parent )
{
}

int NTableLayout::preferredWidth( const NVisualComponent * target ) const
{
  return 10;
}

int NTableLayout::preferredHeight( const NVisualComponent * target ) const
{
  return 10;
}

void NTableLayout::add( NVisualComponent * comp )
{
  if (comp) {
    int col = comp->alignConstraint().col();
    int row = comp->alignConstraint().row();

    columns[col] = Col(row,comp);
  }
}

void NTableLayout::remove( NVisualComponent * comp )
{
}

void NTableLayout::removeAll( )
{
}

void NTableLayout::setRows( int number )
{
  cols_ = number;
}

void NTableLayout::setColumns( int number )
{
  rows_ = number;
}

NTableLayout::Col::Col( )
{
  colWidth_ = 100; // test value
}

NTableLayout::Col::~ Col( )
{
}

NTableLayout::Col::Col( int row, NVisualComponent * comp )
{
  colMap[row] = comp;
}

int NTableLayout::Col::colWidth( ) const
{
   return colWidth_;
}




