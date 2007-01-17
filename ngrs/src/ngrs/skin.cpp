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

#include "skin.h"

namespace ngrs {

  Skin::Skin( )
  {
    transparent_ = true;
    useParentBgColor_ = true;
    useParentFgColor_ = true;
    useParentFont_    = true;
    bitmapBgStyle_ = 0;
    translucent_ = 100;
    gradientStyle_ = 0;
    gradientOrientation_ = nHorizontal;
    gradientPercent_ = 50;
    border_ = 0;
    arcWidth_ = arcHeight_ = 10;
    disabledTextColor_.setRGB(180,180,180);
    overrideFontColor_ = false;
  }

  Skin::Skin( const Skin & src )
  {
    if (src.border())
      border_ = src.border()->clone();
    else
      border_ = 0;

    bgColor_ = src.background();
    fgColor_ = src.foreground();
    transColor_ = src.transColor();
    font_ = src.font();
    spacing_ = src.spacing();
    bitmap_ = src.bitmap();

    useParentBgColor_ = src.parentBackground();
    useParentFgColor_ = src.parentForeground();
    useParentFont_    = src.parentFont();
    transparent_      = src.transparent();
    translucent_      = src.translucent();

    gradStartColor_   = src.gradientStartColor();
    gradMidColor_     = src.gradientMidColor();
    gradEndColor_     = src.gradientEndColor();

    gradientPercent_     = src.gradientPercent();
    gradientStyle_       = src.gradientStyle();
    gradientOrientation_ = src.gradientOrientation();

    bitmapBgStyle_   = src.bitmapBgStyle();
    arcWidth_        = src.gradientArcWidth();
    arcHeight_       = src.gradientArcHeight();

    disabledTextColor_ = src.disabledTextColor();

    textColor_ = src.textColor();
    overrideFontColor_ = src.overrideFontColor();
  }

  const Skin & Skin::operator =( const Skin & rhs )
  {
    if (border_) delete border_;

    if (rhs.border())
      border_ = rhs.border()->clone();
    else
      border_ = 0;

    bgColor_ = rhs.background();
    fgColor_ = rhs.foreground();
    transColor_ = rhs.transColor();
    font_ = rhs.font();
    spacing_ = rhs.spacing();
    bitmap_ = rhs.bitmap();
    textColor_ = rhs.textColor();
    overrideFontColor_ = rhs.overrideFontColor();

    useParentBgColor_ = rhs.parentBackground();
    useParentFgColor_ = rhs.parentForeground();
    useParentFont_    = rhs.parentFont();
    transparent_      = rhs.transparent();
    translucent_      = rhs.translucent();

    gradStartColor_   = rhs.gradientStartColor();
    gradMidColor_     = rhs.gradientMidColor();
    gradEndColor_     = rhs.gradientEndColor();

    gradientPercent_     = rhs.gradientPercent();
    gradientStyle_       = rhs.gradientStyle();
    gradientOrientation_ = rhs.gradientOrientation();

    bitmapBgStyle_   = rhs.bitmapBgStyle();
    arcWidth_        = rhs.gradientArcWidth();
    arcHeight_       = rhs.gradientArcHeight();

    disabledTextColor_ = rhs.disabledTextColor();

    return *this;
  }

  Skin::~ Skin( )
  {
    if (border_) 
      delete border_;
  }

  void Skin::setBorder( const Border & border )
  {
    if (border_) 
      delete border_;
    border_ = border.clone();
  }

  Border * Skin::border( ) const
  {
    return border_;
  }

  void Skin::setBackground( const Color & bgColor )
  {
    bgColor_ = bgColor;
  }

  const Color & Skin::background( ) const
  {
    return bgColor_;
  }

  void Skin::setForeground( const Color & fgColor )
  {
    fgColor_ = fgColor;
  }

  const Color & Skin::foreground( ) const
  {
    return fgColor_;
  }

  void Skin::setTranslucent( const Color & transColor,  int percent )
  {
    transColor_ = transColor;
    translucent_ = percent;
  }

  const Color & Skin::transColor( ) const
  {
    return transColor_;
  }

  void Skin::setFont( const NFont & font )
  {
    font_ = font;
    useParentFont_ = false;
  }

  const NFont & Skin::font( ) const
  {
    return font_;
  }

  void Skin::setTextColor( const Color & color ) {
    textColor_ = color;
    overrideFontColor_ = true;
  }

  const Color & Skin::textColor() const {
    return textColor_;
  }

  bool Skin::overrideFontColor() const {
    return overrideFontColor_;
  }

  void Skin::setSpacing( const Size & size )
  {
    spacing_ = size;
  }

  const Size & Skin::spacing( ) const
  {
    return spacing_;
  }

  void Skin::setBitmap( const Bitmap & bitmap , int bitmapBgStyle)
  {
    bitmap_ = bitmap;
    bitmapBgStyle_ = bitmapBgStyle;
  }

  const Bitmap & Skin::bitmap( ) const
  {
    return bitmap_;
  }

  void Skin::useParentBackground( bool on )
  {
    useParentBgColor_ = on;
  }

  bool Skin::parentBackground( ) const
  {
    return useParentBgColor_;
  }

  void Skin::useParentForeground( bool on )
  {
    useParentFgColor_ = on;
  }

  bool Skin::parentForeground( ) const
  {
    return useParentFgColor_;
  }

  void Skin::useParentFont( bool on )
  {
    useParentFont_ = on;
  }

  bool Skin::parentFont( ) const
  {
    return useParentFont_;
  }

  void Skin::setTransparent( bool on )
  {
    transparent_ = on;
  }

  bool Skin::transparent( ) const
  {
    return transparent_;
  }

  void Skin::setGradient(const Color & start, const Color & mid, const Color & end, int gradientStyle, int gradientOrientation, int percent, int arcWidth, int arcHeight )
  {
    gradStartColor_  = start;
    gradMidColor_    = mid;
    gradEndColor_    = end;
    gradientPercent_ = percent;
    gradientStyle_   = gradientStyle;
    gradientOrientation_ = gradientOrientation;
    arcWidth_ = arcWidth;
    arcHeight_ = arcHeight;
  }

  int Skin::gradientStyle( ) const
  {
    return gradientStyle_;
  }

  int Skin::gradientOrientation( ) const
  {
    return gradientOrientation_;
  }

  int Skin::bitmapBgStyle( ) const
  {
    return bitmapBgStyle_;
  }

  const Color & Skin::gradientStartColor( ) const
  {
    return gradStartColor_;
  }

  const Color & Skin::gradientMidColor( ) const
  {
    return gradMidColor_;
  }

  const Color & Skin::gradientEndColor() const {
    return gradEndColor_;
  }

  int Skin::gradientPercent( ) const
  {
    return gradientPercent_;
  }

  int Skin::gradientArcWidth() const {
    return arcWidth_;
  }

  int Skin::gradientArcHeight() const {
    return arcHeight_;
  }

  void Skin::setGradientStyle(int style) {
    gradientStyle_ = style;
  }

  void Skin::setGradientPercent(int percent) {
    gradientPercent_ = percent;
  }

  void Skin::setGradientOrientation(int orientation) {
    gradientOrientation_ = orientation;
  }

  int Skin::translucent() const {
    return translucent_;
  }

  void Skin::setDisabledTextColor( const Color & color )
  {
    disabledTextColor_ = color;
  }

  const Color & Skin::disabledTextColor( ) const
  {
    return disabledTextColor_;
  }

}
