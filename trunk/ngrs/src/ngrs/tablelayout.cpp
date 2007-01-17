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
#include "tablelayout.h"
#include "visualcomponent.h"

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace ngrs {

  TableLayout::TableLayout()
    : Layout(), cols_(1), rows_(1),hgap_(0),vgap_(0),minCellWidth_(20),minCellHeight_(20)

  {
  }

  TableLayout::TableLayout( int cols, int rows ) : Layout(), cols_(cols), rows_(rows),hgap_(0),vgap_(0),minCellWidth_(20),minCellHeight_(20)
  {
  }

  TableLayout::~TableLayout()
  {
  }

  TableLayout * TableLayout::clone( ) const
  {
    return new TableLayout(*this);
  }

  void TableLayout::align( VisualComponent * parent )
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
      std::map<int,VisualComponent*>::iterator colIt = row.colMap.begin();
      int rowHeight = row.rowMaxHeight();
      int xp = hgap_;
      int lastIndex = 0;
      for ( ; colIt != row.colMap.end(); colIt++) {
        VisualComponent* visual = colIt->second;
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

  int TableLayout::preferredWidth( const VisualComponent * target ) const
  {
    return hgap_ + colWidthBetween(0,cols_);
  }

  int TableLayout::preferredHeight( const VisualComponent * target ) const
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

  void TableLayout::add( VisualComponent * comp )
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

  void TableLayout::remove( VisualComponent * comp )
  {
  }

  void TableLayout::removeAll( )
  {
  }

  void TableLayout::setRows( int number )
  {
    rows_ = number;
  }

  void TableLayout::setColumns( int number )
  {
    cols_ = number;
  }

  TableLayout::Row::Row( )
  {
  }

  TableLayout::Row::~ Row( )
  {
  }

  TableLayout::Row::Row( int col, VisualComponent * comp )
  {
    colMap[col] = comp;
  }


  int TableLayout::Row::rowMaxHeight( ) const
  {
    int maxHeight = defaultRowHeight();

    std::map<int, VisualComponent*>::const_iterator colIt = colMap.begin();
    for ( ; colIt != colMap.end(); colIt++ ) {
      VisualComponent* comp = colIt->second;
      maxHeight = std::max(maxHeight, comp->preferredHeight());
    }

    return maxHeight;
  }

  int TableLayout::Row::defaultRowHeight( ) const
  {
    return 20;
  }


  void TableLayout::Row::add( int col, VisualComponent * comp )
  {
    colMap[col] = comp;
  }

  VisualComponent * TableLayout::Row::colAt( int index )
  {
    std::map<int,class VisualComponent*>::iterator itr;
    if ( (itr = colMap.find( index )) == colMap.end() ) {
      return 0;
    } else {
      return itr->second;
    }
  }

  int TableLayout::defaultColWidth( ) const
  {
    return minCellWidth_;
  }

  int TableLayout::defaultRowHeight( ) const
  {
    return minCellHeight_;
  }

  int TableLayout::colMaxWidth( int col ) const
  {
    int maxWidth = defaultColWidth();
    std::map<int,int>::iterator itr;
    if ( (itr = colMaxWidthCache.find( col )) == colMaxWidthCache.end() ) {
      std::map<int,Row>::const_iterator rowIt = rows.begin();
      for ( ; rowIt != rows.end(); rowIt++ ) {
        Row row = rowIt->second;
        VisualComponent* comp = row.colAt(col);
        if (comp) maxWidth = std::max(maxWidth,comp->preferredWidth());
      }
      colMaxWidthCache[col] = maxWidth;
    } else {
      maxWidth = itr->second;
    }

    return maxWidth;
  }

  int TableLayout::colWidthBetween( int colStart, int colEnd ) const
  {
    int width = 0;
    for (int i = colStart; i < colEnd; i++) {
      width += colMaxWidth(i) + hgap_;
    }
    return width;
  }


  int TableLayout::findVerticalStart( long comparator , VisualComponent* owner)
  {
    return 0;
  }

  void TableLayout::setHGap( int hgap )
  {
    hgap_ = hgap;
  }

  int TableLayout::hGap( ) const
  {
    return hgap_;
  }


  void TableLayout::setVGap( int vgap )
  {
    vgap_ = vgap;
  }

  int TableLayout::vGap( ) const
  {
    return vgap_;
  }

  void TableLayout::setMinimumCellSize( int width, int height )
  {
    minCellWidth_ = width;
    minCellHeight_ = height;
  }

}
