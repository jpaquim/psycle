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

      virtual void onMousePress(int x, int y, int button);
      virtual void onMousePressed(int x, int y, int button);

      virtual void onMove(const NMoveEvent & moveEvent);

  private:

      NScrollBar* sl_;
      NBorder* bd_;
  };


public:
    NScrollBar();
    NScrollBar(int orientation);

    ~NScrollBar();

   signal2<NObject*,int> posChange;

   virtual void resize();

   void setOrientation(int orientation);
   int orientation() const;

   void setStep(int step);
   int step() const;

   void setControl(NVisualComponent* control, int scrollPolicy);
   void onSliderMove();

   void setRange(int range);
   int range() const;

   void setPos(int value);
   int pos() const;

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
   int range_;
   int step_;
   int pos_;

   NButton* incBtn;
   NButton* decBtn;
   Slider* slider_;
   NPanel* sliderArea_;
   NVisualComponent* control_;

   void init();

   void scrollComponent(NVisualComponent* control, int dx, int dy);

   void onDecBtnClicked(NButtonEvent* ev);
   void onIncBtnClicked(NButtonEvent* ev);

};

#endif
