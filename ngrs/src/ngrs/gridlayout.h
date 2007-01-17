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
#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include "layout.h"

/**
@author  Stefan
*/

namespace ngrs {

  class GridLayout : public Layout
  {
  public:
    GridLayout();
    GridLayout(int cols, int rows);
    virtual GridLayout* clone()  const;   // Uses the copy constructor

    ~GridLayout();

    void setRows(int number);
    void setColumns(int number);

    void setHgap(int hgap);
    void setVgap(int vgap);

    virtual void align(VisualComponent* parent);

    virtual int preferredWidth(const VisualComponent* target) const;
    virtual int preferredHeight(const VisualComponent* target) const;

  private:

    int cols_;
    int rows_;
    int hgap_;
    int vgap_;

    int findMaxColWidth() const;
    int findMaxRowHeight() const;

  };

}

#endif
