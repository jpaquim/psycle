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
#ifndef NSLIDER_H
#define NSLIDER_H

#include "npanel.h"
#include "nbutton.h"

/**
@author Stefan
*/
class NSlider : public NPanel
{
  class Slider : public NPanel {
  public:
      Slider(NSlider* sl);
      ~Slider();

      virtual void onMove(const NMoveEvent & moveEvent);
      virtual void paint(NGraphics* g);

  private:

      NSlider* sl_;
  };

public:
    NSlider();

    ~NSlider();

    sigslot::signal2<NSlider*,double> posChanged;
    sigslot::signal2<NSlider*, NGraphics*> customSliderPaint;

    void resize();
    void setOrientation(int orientation);

    virtual void paint(NGraphics* g);

    void setTrackLine(bool on);

    void setRange(double min, double max);
    void setPos(double pos);
    double pos();

    NPanel* slider();

private:

   bool trackLine_;

   double pos_;
   double min_, max_;

   Slider* slider_;
   int orientation_;

   void onSliderMove();
};

#endif
