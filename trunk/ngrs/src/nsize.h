/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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

#include "ncolor.h"
#include "nfont.h"
#include "nsize.h"


/**
@author Stefan
*/
class NSize{

public:

    NSize();
    NSize(int left, int top, int right, int bottom);

    ~NSize();

    // setter

    void setSize(int left, int top, int right, int bottom);
    void setSize(const NSize & size);
    void setLeft(int left);
    void setTop(int top);
    void setRight(int right);
    void setBottom(int bottom);

    // getter

    int left()   const;
    int top()    const;
    int right()  const;
    int bottom() const;

    static NSize zero;

    bool allZero();

    bool operator==(const NSize & size) const;
    bool operator!=(const NSize & size) const;

    NSize clipBox(const NSize & size);

    std::string toString();

private:

    int left_;
    int top_;
    int right_;
    int bottom_;

    std::string stringify( int x );
};

#endif
