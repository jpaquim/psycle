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
#ifndef NBORDER_H
#define NBORDER_H

#include "nsize.h"
#include "ngraphics.h"
#include "nshape.h"
#include "ncolor.h"

/**
@author Stefan
*/
class NBorder{
public:
    NBorder();

    virtual ~NBorder();

    virtual void paint(NGraphics* g, const NShape & geometry);
    void setSpacing(NSize size);

    const NSize & spacing();

    void setColor(const NColor & color);
    const NColor & color();

    virtual NBorder* clone()  const = 0;   // Uses the copy constructor

private:

   NColor color_;
   NSize sizing_;

};

#endif
