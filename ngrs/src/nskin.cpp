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

#include "nskin.h"

NSkin::NSkin( )
{
  transparent = true;
  useParentBgColor = true;
  useParentFgColor = true;
  useParentFont    = true;
  bitmapBgStyle = 0;
  translucent = 100;
  gradientStyle = 0;
  gradientOrientation = nHorizontal;
  gradientPercent = 50;
  border = 0;
  arcWidth = arcHeight = 10;
}

NSkin::NSkin( const NSkin & src )
{
  /*if (src.border)
    border = src.border.clone();
  else
    border = 0;

  bgColor = src.bgColor;
  fgColor = src.fgColor;
  transColor = src.transColor;
  font = src.font;
  spacing = src.spacing;
  bitmap = src.bitmap;

  useParentBgColor = src.useParentBgColor;
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
   int arcHeight;*/
}

const NSkin & NSkin::operator =( const NSkin & rhs )
{
}

NSkin::~ NSkin( )
{
}

