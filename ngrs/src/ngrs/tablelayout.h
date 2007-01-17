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
#ifndef NTABLELAYOUT_H
#define NTABLELAYOUT_H

#include "layout.h"
#include <map>

/**
@author  Stefan Nattkemper
*/

namespace ngrs {

  class TableLayout : public Layout
  {
    class Row {
    public :
      Row();
      Row(int col, class VisualComponent* comp);
      ~Row();

      int rowMaxHeight() const;
      int defaultRowHeight() const;

      VisualComponent* colAt(int index);

      void add(int col, class VisualComponent* comp);

      std::map<int, class VisualComponent*> colMap;

    private:

    };


  public:
    TableLayout();
    TableLayout(int cols, int rows);

    ~TableLayout();

    void setRows(int number);
    void setColumns(int number);

    void setMinimumCellSize(int width, int height);

    void setHGap(int hgap);
    int hGap() const;
    void setVGap(int vgap);
    int vGap() const;

    int defaultColWidth() const;
    int defaultRowHeight() const;

    virtual TableLayout* clone()  const;   // Uses the copy constructor

    virtual void align(class VisualComponent* parent);

    virtual int preferredWidth(const class VisualComponent* target) const;
    virtual int preferredHeight(const class VisualComponent* target) const;

    virtual void add(class VisualComponent* comp);
    virtual void remove(class VisualComponent* comp);
    virtual void removeAll();

  private:

    int cols_;
    int rows_;

    int hgap_;
    int vgap_;

    int minCellWidth_;
    int minCellHeight_;

    int colMaxWidth(int col) const;
    int colWidthBetween(int colStart, int colEnd) const;

    std::map<int,Row> rows;
    mutable std::map<int,int> colMaxWidthCache;

    int findVerticalStart( long comparator , class VisualComponent* owner);
  };

}

#endif
