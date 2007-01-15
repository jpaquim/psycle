/***************************************************************************
*   Copyright (C) 2006 by Stefan Nattkemper  *
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

#include "preset.h"
#include <ngrs/nwindow.h>
#include <ngrs/nlabel.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ndialog.h>


/**
  @author Stefan Nattkemper
*/

namespace ngrs {
  class NTogglePanel;
  class NButton;
  class NSlider;
}

namespace psycle {
  namespace host {

    class Machine;

    const int K_XSIZE = 28;
    const int K_YSIZE = 28;
    const int K_NUMFRAMES = 63;
    const int W_ROWWIDTH = 150;


    class NewNameDlg : public ngrs::NDialog {
    public:
      NewNameDlg();
      ~NewNameDlg();
    };

    class Cell : public ngrs::NPanel {
    public:
      Cell();

      ~Cell();

      virtual void paint( ngrs::Graphics& g );

      virtual int preferredWidth() const;
      virtual int preferredHeight() const;

    };

    class Knob: public Cell {
    public:

      Knob(int param);

      sigslot::signal3<Knob*,int,int> valueChanged;

      void setValue(int value);
      void setValueAsText(const std::string & text);

      void setRange(int min, int max);
      void setText( const std::string& text );

      virtual void onMousePress( int x, int y, int button );
      virtual void onMouseOver( int x, int y);
      virtual void onMousePressed( int x, int y, int button );

      virtual void paint( ngrs::Graphics& g );
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

      //  y position of the mouse when starting a mousedrag (knob tweak)
      int sourcepoint;
      //  value of the knob in that position
      int tweakbase;

      ngrs::NLabel* label;
      ngrs::NLabel* vLabel;

    };


    class Header: public Cell {
    public:
      Header();

      void setText( const std::string& text );

      virtual void resize();

    private:
      ngrs::NLabel* label;
    };


    class FrameMachine : public ngrs::NWindow {
    public:
      FrameMachine( Machine* pMachine );

      ~FrameMachine();

      virtual int onClose();
      virtual void setVisible( bool on );

      Machine* pMac();
      void updateValues();

      sigslot::signal3<int,int,int> patternTweakSlide;

    private:

      std::map< ngrs::NButton*, Preset > presetMap;
      std::map<int, Knob*> knobMap;
      std::map<int, Header*> headerMap;
      Machine* pMachine_;

      ngrs::NPanel* knobPanel;
      ngrs::NTogglePanel* prsPanel;
      ngrs::NButton* defaultPrsBtn;
      ngrs::NSlider* prsBtnSlider;
      
      void init();
      void initParameterGUI();
      
      void onKnobValueChange( Knob* sender, int value, int param );

      bool loadPresets();
      Preset knobsPreset();

      void onPrsClick( ngrs::NButtonEvent* ev );
      void onLeftBtn( ngrs::NButtonEvent* ev );
      void onRightBtn( ngrs::NButtonEvent* ev );      
      void onAddPrs( ngrs::NButtonEvent* ev );
      void onLoadPrs( ngrs::NButtonEvent* ev );

    };

  }
}

#endif
