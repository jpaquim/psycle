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
#ifndef NSCROLLBAR_H
#define NSCROLLBAR_H

#include "nbutton.h"
#include "nbevelborder.h"

enum NScrollPolicyType{ nDx = 1, nDy = 2 };

/**
@author Stefan
*/

class NScrollBar : public NPanel
{
  class Slider : public NPanel {
  public:
      Slider(NScrollBar* sl);
      ~Slider();

      virtual void onMove(const NMoveEvent & moveEvent);

  private:

      NScrollBar* sl_;
      NBorder* bd_;
  };


public:
   NScrollBar();
   NScrollBar(int orientation);

   ~NScrollBar();

   signal1<NScrollBar*> change;
   signal1<NScrollBar*> scroll;

   virtual void resize();

   void setOrientation(int orientation);
   int orientation() const;

   void setPos(double value);
   double pos() const;

   void setRange(double min, double max);
   double range() const;

   void setLargeChange( double step );
   double largeChange() const;

   void setSmallChange(double step);
   double smallChange() const;

   void setControl(NVisualComponent* control, int scrollPolicy);
   void onSliderMove();

   
private:

   NImage* inc;
   NImage* dec;

   NBitmap up;
   NBitmap down;
   NBitmap left;
   NBitmap right;


   NSkin vSlSkin;
   NSkin hSlSkin;

   int orientation_;
   int scrollPolicy_;
   
   double pos_;
   double min_, max_;
   double smallChange_;
   double largeChange_;


   NButton* incBtn;
   NButton* decBtn;
   Slider* slider_;
   NPanel* sliderArea_;
   NVisualComponent* control_;

   void init();

   void scrollComponent(NVisualComponent* control, int dx, int dy);

   void onDecBtnClick(NButtonEvent* ev);
   void onIncBtnClick(NButtonEvent* ev);

   void onScrollAreaClick(NButtonEvent* ev);

   void updateSlider();
   void updateControl();
   void updateControlRange();

};

#endif
