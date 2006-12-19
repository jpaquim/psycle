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

#ifdef _MSC_VER
#undef min 
#undef max
#endif

NTableLayout::NTableLayout()
 : NLayout(), cols_(1), rows_(1),hgap_(0),vgap_(0),minCellWidth_(20),minCellHeight_(20)

{
}

NTableLayout::NTableLayout( int cols, int rows ) : NLayout(), cols_(cols), rows_(rows),hgap_(0),vgap_(0),minCellWidth_(20),minCellHeight_(20)
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
   colMaxWidthCache.clear();

   int yp = vgap_;

   std::map<int,Row>::iterator rowIt = rows.begin();
   int lastRowIndex = 0;
   for ( ; rowIt != rows.end(); rowIt++ ) {
      Row row = rowIt->second;
      int newRowIndex = rowIt->first;
      if ((newRowIndex-lastRowIndex) > 0) {
         yp += (defaultRowHeight()+vgap_) * (newRowIndex-lastRowIndex);
      }
      lastRowIndex = newRowIndex + 1;
      std::map<int,NVisualComponent*>::iterator colIt = row.colMap.begin();
      int rowHeight = row.rowMaxHeight();
      int xp = hgap_;
      int lastIndex = 0;
      for ( ; colIt != row.colMap.end(); colIt++) {
         NVisualComponent* visual = colIt->second;
         int newIndex = visual->alignConstraint().col();
         xp+= colWidthBetween(lastIndex,newIndex);
         int colWidth = colMaxWidth(newIndex);
         visual->setPosition(xp,yp,colWidth,rowHeight);
         xp+=colWidth+hgap_;
         lastIndex = newIndex+1;
      }
      yp+=rowHeight+vgap_;
   }
}

int NTableLayout::preferredWidth( const NVisualComponent * target ) const
{
  return hgap_ + colWidthBetween(0,cols_);
}

int NTableLayout::preferredHeight( const NVisualComponent * target ) const
{
  int yp = vgap_;
  std::map<int,Row>::const_iterator rowIt = rows.begin();
  int lastRowIndex = 0;
  for ( ; rowIt != rows.end(); rowIt++ ) {
    Row row = rowIt->second;
    int rowHeight = row.rowMaxHeight();
    int newRowIndex = rowIt->first;
    if ((newRowIndex-lastRowIndex) > 0) {
      yp += (defaultRowHeight()+vgap_) * (newRowIndex-lastRowIndex);
    }
    lastRowIndex = newRowIndex + 1;
    yp+=rowHeight+vgap_;
  }
  if (lastRowIndex < rows_-1) {
    yp += (defaultRowHeight()+vgap_) * (rows_-lastRowIndex);
  }
  return yp;
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


int NTableLayout::Row::rowMaxHeight( ) const
{
  int maxHeight = defaultRowHeight();

  std::map<int, NVisualComponent*>::const_iterator colIt = colMap.begin();
  for ( ; colIt != colMap.end(); colIt++ ) {
      NVisualComponent* comp = colIt->second;
      maxHeight = std::max(maxHeight, comp->preferredHeight());
   }

  return maxHeight;
}

int NTableLayout::Row::defaultRowHeight( ) const
{
  return 20;
}


void NTableLayout::Row::add( int col, NVisualComponent * comp )
{
  colMap[col] = comp;
}

NVisualComponent * NTableLayout::Row::colAt( int index )
{
  std::map<int,class NVisualComponent*>::iterator itr;
  if ( (itr = colMap.find( index )) == colMap.end() ) {
    return 0;
  } else {
    return itr->second;
  }
}

int NTableLayout::defaultColWidth( ) const
{
  return minCellWidth_;
}

int NTableLayout::defaultRowHeight( ) const
{
  return minCellHeight_;
}

int NTableLayout::colMaxWidth( int col ) const
{
   int maxWidth = defaultColWidth();
   std::map<int,int>::iterator itr;
   if ( (itr = colMaxWidthCache.find( col )) == colMaxWidthCache.end() ) {
     std::map<int,Row>::const_iterator rowIt = rows.begin();
     for ( ; rowIt != rows.end(); rowIt++ ) {
       Row row = rowIt->second;
       NVisualComponent* comp = row.colAt(col);
       if (comp) maxWidth = std::max(maxWidth,comp->preferredWidth());
     }
     colMaxWidthCache[col] = maxWidth;
   } else {
     maxWidth = itr->second;
   }

   return maxWidth;
}

int NTableLayout::colWidthBetween( int colStart, int colEnd ) const
{
   int width = 0;
   for (int i = colStart; i < colEnd; i++) {
      width += colMaxWidth(i) + hgap_;
   }
   return width;
}


int NTableLayout::findVerticalStart( long comparator , NVisualComponent* owner)
{
  return 0;
}

void NTableLayout::setHGap( int hgap )
{
  hgap_ = hgap;
}

int NTableLayout::hGap( ) const
{
  return hgap_;
}


void NTableLayout::setVGap( int vgap )
{
  vgap_ = vgap;
}

int NTableLayout::vGap( ) const
{
  return vgap_;
}

void NTableLayout::setMinimumCellSize( int width, int height )
{
  minCellWidth_ = width;
  minCellHeight_ = height;
}











