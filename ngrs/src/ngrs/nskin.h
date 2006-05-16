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

#include "ngrs/ncolor.h"
#include "ngrs/nfont.h"
#include "ngrs/nsize.h"
#include "ngrs/nborder.h"
#include "ngrs/nbitmap.h"

const int nVertical   = 16;
const int nHorizontal = 32;

class NSkin {
public:

   NSkin();
   ~NSkin();

   NSkin(const NSkin & src);
   const NSkin & operator= (const NSkin & rhs);

   void setBorder(const NBorder & border);
   NBorder* border() const;
   void setBackground(const NColor & bgColor);
   const NColor & background() const;
   void setForeground(const NColor & fgColor);
   const NColor & foreground() const;
   void setTranslucent(const NColor & transColor, int percent);
   const NColor & transColor() const;
   void setFont(const NFont & font);
   const NFont & font() const;
   void setSpacing(const NSize & size);
   const NSize & spacing() const;
   void setBitmap(const NBitmap & bitmap, int bitmapBgStyle);
   const NBitmap & bitmap() const;
   int bitmapBgStyle() const;

   void useParentBackground(bool on);
   bool parentBackground() const;
   void useParentForeground(bool on);
   bool parentForeground() const;
   void useParentFont(bool on);
   bool parentFont() const;
   void setTransparent(bool on);
   bool transparent() const;
   int translucent() const;

   void setGradient(const NColor & start, const NColor & mid, const NColor & end, int gradientStyle, int gradientOrientation, int percent, int arcWidth, int arcHeight);
   const NColor & gradientStartColor() const;
   const NColor & gradientMidColor() const;
   const NColor & gradientEndColor() const;
   int gradientPercent() const;
   int gradientStyle() const;
   int gradientOrientation() const;
   int gradientArcWidth() const;
   int gradientArcHeight() const;

   void setGradientStyle(int style);
   void setGradientPercent(int percent);
   void setGradientOrientation(int orientation);

   private:

      // gradient style information
      NColor gradStartColor_;
      NColor gradMidColor_;
      NColor gradEndColor_;
      int gradientPercent_, gradientStyle_, gradientOrientation_;
      int arcWidth_;
      int arcHeight_;

      // fore- and bgcolor style
      NColor bgColor_;
      NColor fgColor_;
      bool transparent_;
      bool useParentBgColor_;
      bool useParentFgColor_;

      // translucent Background style
      NColor transColor_;  // the transparent color mix
      int translucent_;    // the percent of translucent visibility

      // the Bitmap background style
      NBitmap bitmap_;
      int bitmapBgStyle_;

      // the Font style
      NFont  font_;
      bool useParentFont_;

      // the border style
      NBorder* border_;

      // the spacing indent
      NSize  spacing_;
};

#endif
