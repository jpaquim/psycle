/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#ifndef NSLIDER_H
#define NSLIDER_H

#include "panel.h"
#include "button.h"

/**
@author  Stefan
*/

namespace ngrs {

  class Slider : public Panel
  {
    class SliderBtn : public Panel {
    public:
      SliderBtn( Slider* sl );
      ~SliderBtn();

      virtual void onMove(const MoveEvent & moveEvent);
      virtual void paint(Graphics& g);

    private:

      Slider* sl_;
    };

  public:
    Slider();

    ~Slider();

    sigslot::signal1<Slider*> change;
    sigslot::signal2<Slider*, Graphics&> customSliderPaint;

    void resize();
    void setOrientation(int orientation);
    int orientation() const;

    virtual void paint(Graphics& g);

    void setTrackLine(bool on);

    void setRange( double min, double max );
    void setPos( double pos );
    double pos() const;

    Panel* slider();

  private:

    bool trackLine_;

    double pos_;
    double min_, max_;

    SliderBtn* slider_;
    int orientation_;

    void updateSlider();

    void onSliderMove();
  };

}

#endif
