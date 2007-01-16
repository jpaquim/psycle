/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "progressbar.h"
#include "frameborder.h"
#include "label.h"

namespace ngrs {

  ProgressBar::ProgressBar()
    : Panel(), progress_(50) , min_(0),max_(100), step_(10), orientation_(nVertical)
  {
    FrameBorder fr;
    fr.setSpacing( Size(0,0,0,0) );
    setBorder( fr );


    progressGradient_ = new Gradient();
    add(progressGradient_);
    progressGradient_->setColor_1(Color(200,200,205));
    progressGradient_->setColor_2(Color(230,230,255));
    progressGradient_->setColor_3(Color(200,200,205));

    label_ = new Label();
    label_->setHAlign(nAlCenter);
    add(label_);
    label_->setEvents(false);
  }


  ProgressBar::~ProgressBar()
  {

  }


  void ProgressBar::setText( const std::string & text )
  {
    label_->setText(text);
  }

  std::string ProgressBar::text( )
  {
    return label_->text();
  }

  void ProgressBar::resize( )
  { 
    if (orientation_ == nVertical) {
      int progressWidth = (int) ((progress_ / 100.0f) * clientWidth());
      progressGradient_->setPosition(0,0,progressWidth,clientHeight());
    } else {
      int progressHeight = (int) ((progress_ / 100.0f) * clientHeight());
      progressGradient_->setPosition(0,progressHeight,clientWidth(),clientHeight()-progressHeight);
    }
    label_->setPosition(0,0,spacingWidth(),spacingHeight());
  }

  void ProgressBar::setMin( int min )
  {
    min_ = min;
  }

  int ProgressBar::min( ) const
  {
    return min_;
  }

  void ProgressBar::setMax( int max )
  {
    max_ = max;
  }

  int ProgressBar::max( ) const
  {
    return max_;
  }

  void ProgressBar::setStep( int step )
  {
    step_ = step;
  }

  int ProgressBar::step( ) const
  {
    return step_;
  }

  void ProgressBar::setValue( int value )
  {
    progress_ = (int) (( value / ((double) (max_-min_))) * 100);
    resize();
  }

  int ProgressBar::value( ) const
  {
    return (int) ((progress_ * (max_-min_)) / 100.0f);
  }

  void ProgressBar::setOrientation( int orientation )
  {
    orientation_ = orientation;
  }

  void ProgressBar::setRange( int min, int max )
  {
    min_ = min;
    max_ = max;
  }

}
