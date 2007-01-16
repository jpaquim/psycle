/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#ifndef NSIZE_H
#define NSIZE_H

#include "color.h"
#include "size.h"

/**
@author Stefan
*/

namespace ngrs {

  class Size{

  public:

    Size();
    Size(int left, int top, int right, int bottom);

    ~Size();

    // setter

    void setSize(int left, int top, int right, int bottom);
    void setSize(const Size & size);
    void setLeft(int left);
    void setTop(int top);
    void setRight(int right);
    void setBottom(int bottom);

    // getter

    int left()   const;
    int top()    const;
    int right()  const;
    int bottom() const;

    static Size zero;

    bool allZero();

    bool operator==(const Size & size) const;
    bool operator!=(const Size & size) const;

    Size clipBox(const Size & size);

    std::string toString();

  private:

    int left_;
    int top_;
    int right_;
    int bottom_;

    std::string stringify( int x );
  };

}

#endif
