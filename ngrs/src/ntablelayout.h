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
#ifndef NTABLELAYOUT_H
#define NTABLELAYOUT_H

#include <nlayout.h>
#include <map>

/**
@author Stefan Nattkemper
*/
class NTableLayout : public NLayout
{
  class Row {
     public :
       Row();
       Row(int col, class NVisualComponent* comp);
       ~Row();

       int rowHeight() const;
       NVisualComponent* colAt(int index);

       void add(int col, class NVisualComponent* comp);

       std::map<int, class NVisualComponent*> colMap;

     private:

  };


public:
    NTableLayout();

    ~NTableLayout();

    void setRows(int number);
    void setColumns(int number);

    int defaultColWidth() const;
    int defaultRowHeight() const;

    virtual NTableLayout* clone()  const;   // Uses the copy constructor

    virtual void align(class NVisualComponent* parent);

    virtual int preferredWidth(const class NVisualComponent* target) const;
    virtual int preferredHeight(const class NVisualComponent* target) const;

    virtual void add(class NVisualComponent* comp);
    virtual void remove(class NVisualComponent* comp);
    virtual void removeAll();

private:

    int cols_;
    int rows_;

    std::map<int,Row> rows;
};

#endif
