/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
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
#ifndef FRAMEMACHINE_H
#define FRAMEMACHINE_H

#include "presetsdlg.h"
#include <nwindow.h>
#include <nlabel.h>
#include <nframeborder.h>
#include <ngridlayout.h>
#include <nbitmap.h>
#include <ndialog.h>

const int K_XSIZE = 28;
const int K_YSIZE = 28;
const int K_NUMFRAMES = 63;
const int W_ROWWIDTH = 150;



class Machine;
class NTogglePanel;
class NButton;
class NSlider;


/**
@author Stefan
*/

class NewNameDlg : public NDialog {
public:
  NewNameDlg();
  ~NewNameDlg();
};

class Cell : public NPanel {
public:
   Cell() {
     setBorder(NFrameBorder());
   }

   virtual void paint(NGraphics* g);

   virtual ~Cell() {}

   virtual int preferredWidth() const {
      return 100;
   }

   virtual int preferredHeight() const {
      return K_YSIZE;
   }
};

class Knob: public Cell {
public:

   Knob(int param);

   signal3<Knob*,int,int> valueChanged;

   void setValue(int value);
   void setValueAsText(const std::string & text);

   void setRange(int min, int max);
   void setText(const std::string & text);

   virtual void onMousePress(int x, int y, int button);
   virtual void onMouseOver(int x, int y);
   virtual void onMousePressed(int x, int y, int button);

   virtual void paint(NGraphics* g);
   virtual void resize();

   virtual int preferredWidth() const;
   virtual int preferredHeight() const;


private:

   int param_;
   int max_range;
   int min_range;
   int value_;
   std::string tvalue;

   bool istweak;
   bool finetweak;
   bool ultrafinetweak;

   int sourcepoint;

   static int c;
   static NBitmap kbitmap;

   NLabel* label;
   NLabel* vLabel;

};


class Header: public Cell {
public:
     Header();

     void setText(const std::string & text) {
       label->setText(text);
     }

     virtual void resize() {
       int ch = clientHeight();
       int lh = label->preferredHeight();
       label->setPosition(0,(ch - lh) / 2,clientWidth(),lh);
     }

private:

    NLabel* label;
};

class FrameMachine : public NWindow
{
public:
    FrameMachine(Machine* pMachine);

    ~FrameMachine();

    virtual int onClose();

    Machine* pMac();
    void updateValues();

private:

   std::map<NButton*,Preset> presetMap;

   Machine* pMachine_;
   NPanel* knobPanel;
   NTogglePanel* prsPanel;
   NButton* defaultPrsBtn;
   NSlider* prsBtnSlider;

   NGridLayout* gridLayout;

   Preset knobsPreset();

   void onItemClicked(NEvent* menuEv, NButtonEvent* itemEv);
   void init();
   void initParameterGUI();

   void onPrsClick(NButtonEvent* ev);

   void onKnobValueChange(Knob* sender, int value, int param);

   void onLeftBtn(NButtonEvent* ev);
   void onRightBtn(NButtonEvent* ev);

   void loadPresets();
   void onAddPrs(NButtonEvent* ev);
   void onLoadPrs(NButtonEvent* ev);



};

#endif
