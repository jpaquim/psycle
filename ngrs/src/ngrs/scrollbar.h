/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper                         *
 *   Made in Germany                                                       *
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
#ifndef NSCROLLBAR_H
#define NSCROLLBAR_H

#include "button.h"
#include "bevelborder.h"

/**
@author  Stefan
*/

namespace ngrs {

  enum NScrollPolicyType{ nDx = 1, nDy = 2 };

  class ScrollBar : public Panel {
  public:
    ScrollBar( );
    ScrollBar( int orientation );

    ~ScrollBar( );

    signal1<ScrollBar*> change;
    signal1<ScrollBar*> scroll;

    virtual void resize();

    void setOrientation( int orientation );
    int orientation( ) const;

    void setPos( double value );
    double pos( ) const;

    void setRange( double min, double max );
    double range() const;

    void setLargeChange( double step );
    double largeChange() const;

    void setSmallChange(double step);
    double smallChange() const;

    void setControl( VisualComponent* control, int scrollPolicy );

    void onSliderMove( const MoveEvent & ev );

  private:

    Image* inc;
    Image* dec;

    Bitmap up;
    Bitmap down;
    Bitmap left;
    Bitmap right;

    int orientation_;
    int scrollPolicy_;

    double pos_;
    double min_, max_;
    double smallChange_;
    double largeChange_;


    Button* incBtn;
    Button* decBtn;
    Button* slider_;

    Panel* sliderArea_;
    VisualComponent* control_;

    void init();

    void scrollComponent(VisualComponent* control, int dx, int dy);

    void onDecBtnClick(ButtonEvent* ev);
    void onIncBtnClick(ButtonEvent* ev);

    void onScrollAreaClick(ButtonEvent* ev);

    void updateSlider();
    void updateControl();
    void updateControlRange();

  };

}

#endif
