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
#ifndef NSKIN_H
#define NSKIN_H

/**
@author Stefan
*/

#include "ncolor.h"
#include "nfont.h"
#include "nsize.h"
#include "nborder.h"
#include "nbitmap.h"

const int nVertical   = 16;
const int nHorizontal = 32;

class NSkin {
public:

   NSkin();
   ~NSkin();

   NSkin(const NSkin & src);
   const NSkin & operator= (const NSkin & rhs);


   NBorder* border;
   NColor bgColor;
   NColor fgColor;
   NColor transColor;
   NFont  font;
   NSize  spacing;
   NBitmap bitmap;

   bool useParentBgColor;
   bool useParentFgColor;
   bool useParentFont;
   bool transparent;
   int translucent;

   NColor gradStartColor;
   NColor gradMidColor;
   NColor gradEndColor;

   int gradientPercent;
   int gradientStyle;
   int gradientOrientation;

   int bitmapBgStyle;
   int arcWidth;
   int arcHeight;
};

#endif
