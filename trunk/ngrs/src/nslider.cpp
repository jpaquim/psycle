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
#include "nslider.h"
#include "nframeborder.h"

NSlider::NSlider()
 : NPanel(), trackLine_(true), pos_(0) , min_(0), max_(100), orientation_(nVertical)
{
  slider_ = new Slider(this);
  add(slider_);
  slider_->setMoveable(NMoveable(nMvVertical + nMvParentLimit));
  setTransparent(true);
}


NSlider::~NSlider()
{
}

void NSlider::resize( )
{
  if (orientation_ == nVertical) {
    slider_->setLeft(0);
    slider_->setWidth(clientWidth());
  } else {
    slider_->setTop(0);
    slider_->setHeight(clientHeight());
  }
}

NSlider::Slider::Slider( NSlider* sl)
{
  setTransparent(false);
  setBorder(new NFrameBorder(),true);
  sl_ = sl;
}

NSlider::Slider::~ Slider( )
{
}

void NSlider::Slider::paint( NGraphics * g )
{
  sl_->customSliderPaint.emit(sl_,g);
}

void NSlider::Slider::onMove( const NMoveEvent & moveEvent )
{
  sl_->onSliderMove();
}

void NSlider::onSliderMove( )
{
   double range = max_ - min_;
   if (orientation_ == nVertical)
     pos_ = ( range / (clientHeight()- slider_->height()) ) * slider_->top();
   else
     pos_ = ( range / (clientWidth() - slider_->width())  ) * slider_->left();
   posChanged.emit(this,pos_);
}

void NSlider::setOrientation( int orientation )
{
  orientation_ = orientation;
  slider_->setMoveable(NMoveable(nMvHorizontal + nMvParentLimit));
  slider_->setWidth(10);
  slider_->setHeight(5);
  slider_->setLeft(0);
  resize();
}

void NSlider::paint( NGraphics * g )
{
  if (trackLine_) {
    g->setForeground(NColor(230,230,230));
    if (orientation_ == nVertical) {
     int d = 2;
     g->drawRect(clientWidth()/2 - d,0,2*d,clientHeight());
    } else {
     int d = 2;
     g->drawRect(0,clientHeight()/2 - d,clientWidth()-1,2*d);
   }
  }
}

void NSlider::setTrackLine( bool on )
{
  trackLine_ = on;
}

void NSlider::setRange( double min, double max )
{
  min = min_; max_ = max;
}

void NSlider::setPos( double pos )
{
  double range = max_ - min_;
  if (orientation_ == nVertical)
     slider_->setTop(  (int) (pos  / ((range / (clientHeight()- slider_->height()))) ));
  else
     slider_->setLeft( (int) (pos  / ((range / (clientWidth() - slider_->width()))) ));
  pos_ = pos;
}

double NSlider::pos( )
{
  return pos_;
}

NPanel * NSlider::slider( )
{
  return slider_;
}






