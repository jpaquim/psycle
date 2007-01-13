/***************************************************************************
*   Copyright (C) 2006 by Stefan Nattkemper   *
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
#ifndef INSTRUMENTEDITOR_H
#define INSTRUMENTEDITOR_H

#include <ngrs/nwindow.h>

/**
  @author Stefan Nattkemper
*/

namespace ngrs {
  class NLabel;
  class NButton;
  class NEdit;
  class NComboBox;
  class NSlider;
  class NCheckBox;
  class NItemEvent;
}

namespace psycle {
  namespace host{

    class EnvDialog;
    class Song;
    
    class InstrumentEditor : public ngrs::NWindow
    {
    public:
      InstrumentEditor( Song * song );

      ~InstrumentEditor();

      void setInstrument( int index );
      int instrumentIndex();

      virtual int onClose();

    private:

      Song* _pSong;

      ngrs::NLabel* instNumberLbl;
      ngrs::NButton* incInstBtn;
      ngrs::NButton* decInstBtn;
      ngrs::NEdit* instNameEd;
      ngrs::NButton* killBtn;
      ngrs::NComboBox* newNoteActionCb;
      ngrs::NSlider* panningSlider;
      ngrs::NLabel* panningLbl;
      ngrs::NCheckBox* rndPanningCbx;
      ngrs::NCheckBox* rndVCFCutCbx;
      ngrs::NCheckBox* rndVCFResoCbx;
      ngrs::NCheckBox* playSampleFitCbx;
      ngrs::NEdit* patRowEdt;
      ngrs::NSlider* volumeSlider;
      ngrs::NLabel* volumeLbl;
      ngrs::NSlider* fineTuneSlider;
      ngrs::NLabel* fineTuneLbl;
      ngrs::NButton* octDecBtn;
      ngrs::NButton* noteDecBtn;
      ngrs::NButton* noteIncBtn;
      ngrs::NButton* octIncBtn;
      ngrs::NLabel* octLbl;
      ngrs::NLabel* loopLbl;
      ngrs::NLabel* loopAtFromLbl;
      ngrs::NLabel* loopAtToLbl;
      ngrs::NLabel* lenLbl;

      EnvDialog* envelopeEditor;

      void init();
      std::string noteToString( int value );
      void onBtnPress( ngrs::NButtonEvent* ev );
      void onComboSelected( ngrs::NItemEvent* ev );
      void onSliderMove( ngrs::NSlider* sender );
      void onShowEnvelopeEditor( ngrs::NButtonEvent* ev );
      void onPatRowEdit( const ngrs::NKeyEvent& event );
    };

  }
}

#endif
