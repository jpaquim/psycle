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
}

NSkin::NSkin( const NSkin & src )
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
}

const NSkin & NSkin::operator =( const NSkin & rhs )
{
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
  return *this;
}

NSkin::~ NSkin( )
{
  if (border_) delete border_;
}

void NSkin::setBorder( const NBorder & border )
{
  border_ = border.clone();
}

NBorder * NSkin::border( ) const
{
  return border_;
}

void NSkin::setBackground( const NColor & bgColor )
{
  bgColor_ = bgColor;
}

const NColor & NSkin::background( ) const
{
  return bgColor_;
}

void NSkin::setForeground( const NColor & fgColor )
{
  fgColor_ = fgColor;
}

const NColor & NSkin::foreground( ) const
{
  return fgColor_;
}

void NSkin::setTranslucent( const NColor & transColor,  int percent )
{
  transColor_ = transColor;
  translucent_ = percent;
}

const NColor & NSkin::transColor( ) const
{
  return transColor_;
}

void NSkin::setFont( const NFont & font )
{
  font_ = font;
}

const NFont & NSkin::font( ) const
{
  return font_;
}

void NSkin::setSpacing( const NSize & size )
{
  spacing_ = size;
}

const NSize & NSkin::spacing( ) const
{
  return spacing_;
}

void NSkin::setBitmap( const NBitmap & bitmap , int bitmapBgStyle)
{
  bitmap_ = bitmap;
  bitmapBgStyle_ = bitmapBgStyle;
}

const NBitmap & NSkin::bitmap( ) const
{
  return bitmap_;
}

void NSkin::useParentBackground( bool on )
{
  useParentBgColor_ = on;
}

bool NSkin::parentBackground( ) const
{
  return useParentBgColor_;
}

void NSkin::useParentForeground( bool on )
{
  useParentFgColor_ = on;
}

bool NSkin::parentForeground( ) const
{
  return useParentFgColor_;
}

void NSkin::useParentFont( bool on )
{
  useParentFont_ = on;
}

bool NSkin::parentFont( ) const
{
  return useParentFont_;
}

void NSkin::setTransparent( bool on )
{
  transparent_ = on;
}

bool NSkin::transparent( ) const
{
  return transparent_;
}

void NSkin::setGradient(const NColor & start, const NColor & mid, const NColor & end, int gradientStyle, int gradientOrientation, int percent, int arcWidth, int arcHeight )
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

int NSkin::gradientStyle( ) const
{
  return gradientStyle_;
}

int NSkin::gradientOrientation( ) const
{
  return gradientOrientation_;
}

int NSkin::bitmapBgStyle( ) const
{
  return bitmapBgStyle_;
}

const NColor & NSkin::gradientStartColor( ) const
{
  return gradStartColor_;
}

const NColor & NSkin::gradientMidColor( ) const
{
  return gradMidColor_;
}

const NColor & NSkin::gradientEndColor() const {
  return gradEndColor_;
}

int NSkin::gradientPercent( ) const
{
  return gradientPercent_;
}

int NSkin::gradientArcWidth() const {
  return arcWidth_;
}

int NSkin::gradientArcHeight() const {
  return arcHeight_;
}

void NSkin::setGradientStyle(int style) {
  gradientStyle_ = style;
}

void NSkin::setGradientPercent(int percent) {
  gradientPercent_ = percent;
}

void NSkin::setGradientOrientation(int orientation) {
  gradientOrientation_ = orientation;
}

int NSkin::translucent() const {
  return translucent_;
}
