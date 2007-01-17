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
#ifndef SKIN_H
#define SKIN_H

/**
@author  Stefan
*/

#include "color.h"
#include "font.h"
#include "size.h"
#include "border.h"
#include "bitmap.h"

namespace ngrs {

  const int nVertical   = 16;
  const int nHorizontal = 32;

  class Skin {
  public:

    Skin();
    ~Skin();

    Skin( const Skin& src );
    const Skin& operator=( const Skin & rhs );

    void setBorder( const Border& border );
    Border* border() const;
    void setBackground( const Color& bgColor );
    const Color& background() const;
    void setForeground( const Color & fgColor );
    const Color& foreground() const;
    void setTranslucent(const Color & transColor, int percent);
    void setDisabledTextColor(const Color & color);
    const Color& disabledTextColor() const;

    const Color& transColor() const;

    void setFont(const Font& font);
    const Font& font() const;

    void setTextColor( const Color& color );
    const Color& textColor() const;
    bool overrideFontColor() const;

    void setSpacing(const Size& size);
    const Size& spacing() const;
    void setBitmap(const Bitmap& bitmap, int bitmapBgStyle);
    const Bitmap& bitmap() const;
    int bitmapBgStyle() const;

    void useParentBackground( bool on );
    bool parentBackground() const;
    void useParentForeground( bool on );
    bool parentForeground() const;
    void useParentFont( bool on );
    bool parentFont() const;
    void setTransparent( bool on );
    bool transparent() const;
    int translucent() const;

    void setGradient( const Color& start, const Color& mid, const Color& end, int gradientStyle, int gradientOrientation, int percent, int arcWidth, int arcHeight);
    const Color& gradientStartColor() const;
    const Color& gradientMidColor() const;
    const Color& gradientEndColor() const;
    int gradientPercent() const;
    int gradientStyle() const;
    int gradientOrientation() const;
    int gradientArcWidth() const;
    int gradientArcHeight() const;

    void setGradientStyle( int style );
    void setGradientPercent( int percent );
    void setGradientOrientation( int orientation );

  private:

    // gradient style information
    Color gradStartColor_;
    Color gradMidColor_;
    Color gradEndColor_;
    int gradientPercent_, gradientStyle_, gradientOrientation_;
    int arcWidth_;
    int arcHeight_;

    // fore- and bgcolor style
    Color bgColor_;
    Color fgColor_;
    bool transparent_;
    bool useParentBgColor_;
    bool useParentFgColor_;

    // translucent Background style
    Color transColor_;  // the transparent color mix
    int translucent_;    // the percent of translucent visibility

    // the Bitmap background style
    Bitmap bitmap_;
    int bitmapBgStyle_;

    // the Font style
    Font  font_;
    bool useParentFont_;

    // the border style
    Border* border_;

    // the spacing indent
    Size  spacing_;

    // color that will be used as foreground, when a component is disabled
    Color disabledTextColor_;

    // to overide textColor textColor

    Color textColor_;
    bool overrideFontColor_;
  };

}

#endif
