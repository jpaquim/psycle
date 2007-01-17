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
#include "slider.h"
#include "frameborder.h"

namespace ngrs {

  Slider::Slider()
    : Panel(), trackLine_(true), pos_(0), min_(0), max_(100), orientation_(nVertical)
  {
    slider_ = new SliderBtn( this );
    add( slider_ );
    slider_->setMoveable( Moveable( nMvVertical | nMvParentLimit ) );
    setTransparent(true);
  }


  Slider::~Slider()
  {
  }

  void Slider::resize( )
  {
    if (orientation_ == nVertical) {
      if (clientWidth() > 0) slider_->setWidth(clientWidth());
      updateSlider();
    } else {
      slider_->setHeight(clientHeight());
      updateSlider();
    }
  }

  Slider::SliderBtn::SliderBtn( Slider* sl )
  {
    setTransparent(false);
    setBorder(FrameBorder());
    sl_ = sl;
  }

  Slider::SliderBtn::~ SliderBtn( )
  {
  }

  void Slider::SliderBtn::paint( Graphics& g )
  {
    sl_->customSliderPaint.emit(sl_,g);
  }

  void Slider::SliderBtn::onMove( const MoveEvent & moveEvent )
  {
    sl_->onSliderMove();
  }

  void Slider::onSliderMove( )
  {
    double range = max_ - min_;

    if ( range != 0 ) {
      switch (orientation_) {
       case nVertical :
         pos_ = ( range / (clientHeight()- slider_->height()) ) * slider_->top();
         break;
       default :
         pos_ = ( range / (clientWidth() - slider_->width())  ) * slider_->left();
      }

      change.emit( this );
    }
  }

  void Slider::setOrientation( int orientation )
  {
    orientation_ = orientation;
    slider_->setMoveable(Moveable(nMvHorizontal + nMvParentLimit));
    slider_->setWidth(10);
    slider_->setHeight(5);
    slider_->setLeft(0);
    resize();
  }

  int Slider::orientation( ) const
  {
    return orientation_;
  }

  void Slider::paint( Graphics& g )
  {
    if (trackLine_) {
      g.setForeground(Color(230,230,230));
      if (orientation_ == nVertical) {
        int d = 2;
        g.drawRect(clientWidth()/2 - d,0,2*d,clientHeight());
      } else {
        int d = 2;
        g.drawRect(0,clientHeight()/2 - d,clientWidth()-1,2*d);
      }
    }
  }

  void Slider::setTrackLine( bool on )
  {
    trackLine_ = on;
  }

  void Slider::setRange( double min, double max )
  {
    min = min_; max_ = max;
  }

  void Slider::setPos( double pos )
  {
    pos_ = pos;
    updateSlider(); 
    change.emit( this );  
  }

  void Slider::updateSlider()
  {
    double range = max_ - min_;

    if (range == 0) return;

    if (orientation_ == nVertical)
      slider_->setTop(  (int) (pos_  / ((range / (clientHeight()- slider_->height()))) ));
    else 
      slider_->setLeft( (int) (pos_  / ((range / (clientWidth() - slider_->width()))) ));
  }

  double Slider::pos( ) const
  {
    return pos_;
  }

  Panel * Slider::slider( )
  {
    return slider_;
  }

}
