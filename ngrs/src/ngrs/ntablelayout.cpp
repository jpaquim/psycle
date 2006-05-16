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
#include "ngrs/ntablelayout.h"
#include "ngrs/nvisualcomponent.h"

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
   int xp = 0;
   int yp = 0;

   std::map<int,Row>::iterator rowIt = rows.begin();
   for ( ; rowIt != rows.end(); rowIt++ ) {
      Row row = rowIt->second;
      std::map<int,NVisualComponent*>::iterator colIt = row.colMap.begin();
      int rowHeight = defaultRowHeight();
      xp=0;
      for ( ; colIt != row.colMap.end(); colIt++) {
         NVisualComponent* visual = colIt->second;
         visual->setPosition(xp,yp,defaultColWidth(),defaultRowHeight());
         xp+=defaultColWidth();
      }
      yp+=defaultRowHeight();
   }
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

    std::map<int,Row>::iterator itr;
    if ( (itr = rows.find( row )) == rows.end() ) {
       rows[row] = Row(col,comp);
    } else {
       itr->second.add(col,comp);
    }
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

NTableLayout::Row::Row( )
{
}

NTableLayout::Row::~ Row( )
{
}

NTableLayout::Row::Row( int col, NVisualComponent * comp )
{
  colMap[col] = comp;
}

int NTableLayout::Row::rowHeight( ) const
{
   //return colWidth_;
}

void NTableLayout::Row::add( int col, NVisualComponent * comp )
{
  colMap[col] = comp;
}

NVisualComponent * NTableLayout::Row::colAt( int index )
{
  std::map<int,NVisualComponent*>::iterator itr;
  if ( (itr = colMap.find( index )) == colMap.end() ) {
    return 0;
  } else {
    return itr->second;
  }
}

int NTableLayout::defaultColWidth( ) const
{
  return 100;
}

int NTableLayout::defaultRowHeight( ) const
{
  return 20;
}








