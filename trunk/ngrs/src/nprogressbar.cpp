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
#include "nprogressbar.h"
#include "nframeborder.h"
#include "nlabel.h"


NProgressBar::NProgressBar()
 : NPanel(), progress_(50) , min_(0),max_(100), step_(10), orientation_(nVertical)
{
  setBorder(border_ = new NFrameBorder());
    border()->setSpacing(NSize(0,0,0,0));

  progressGradient_ = new NGradient();
  add(progressGradient_);
  progressGradient_->setColor_1(NColor(200,200,205));
  progressGradient_->setColor_2(NColor(230,230,255));
  progressGradient_->setColor_3(NColor(200,200,205));

  label_ = new NLabel();
  label_->setHAlign(nAlCenter);
  add(label_);
  label_->setEvents(false);
}


NProgressBar::~NProgressBar()
{
  delete border_;
}


void NProgressBar::setText( const std::string & text )
{
  label_->setText(text);
}

std::string NProgressBar::text( )
{
  return label_->text();
}

void NProgressBar::resize( )
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

void NProgressBar::setMin( int min )
{
  min_ = min;
}

int NProgressBar::min( )
{
  return min_;
}

void NProgressBar::setMax( int max )
{
  max_ = max;
}

void NProgressBar::setStep( int step )
{
  step_ = step;
}

int NProgressBar::step( )
{
  return step_;
}

int NProgressBar::max( )
{
  return max_;
}

void NProgressBar::setValue( int value )
{
  progress_ = (int) (( value / ((double) (max_-min_))) * 100);
  resize();
}

int NProgressBar::value( )
{
  return (int) ((progress_ * (max_-min_)) / 100.0f);
}

void NProgressBar::setOrientation( int orientation )
{
  orientation_ = orientation;
}








