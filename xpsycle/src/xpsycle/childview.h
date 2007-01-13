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
#ifndef CHILDVIEW_H
#define CHILDVIEW_H

#include "machineview.h"
#include "patternview.h"
#include "song.h"
#include "sequencerbar.h"
#include "global.h"
#include <ngrs/ntabbook.h>
#include <ngrs/ntimer.h>
#include <ngrs/ntabbook.h>


/**
  @author Stefan
*/

namespace psycle {
  namespace host {

    class WaveEdFrame;
    class SequencerGUI;
    class VirtualPattern;
    class SequencerBar;


    class ChildView : public ngrs::NPanel {
    public:
      ChildView( );

      ~ChildView();

      Song* song();

      void setTitleBarText( );

      void onPatternView(NObject* sender);

      // connect to signals
      signal1<Machine*> newMachineAdded;
      signal1<Machine*> machineSelected;
      signal1<ngrs::NButtonEvent*> machineViewDblClick;

      PatternView* patternView();
      MachineView* machineView();
      SequencerBar* sequencerBar();
      WaveEdFrame* waveEditor();
      SequencerGUI* sequencerView();
      VirtualPattern* virtualPattern();

      void play();
      void playFromStart();
      void stop();

      ngrs::NTimer timer;

      void onMachineViewDblClick( ngrs::NButtonEvent* ev );

      void showMachineView();
      void showPatternView();
      void showWaveView();
      void showSequencerView();

      void update();

    private:

      Song* _pSong;

      ngrs::NTabBook* tabBook_;

      // ngrs::NFileDialog* getSaveFileName_;

      SequencerBar* sequencerBar_;
      MachineView* machineView_;
      PatternView* patternView_;
      SequencerGUI* sequencerView_;
      VirtualPattern* virtualPattern_;

      ngrs::NDockPanel* macDock;
      ngrs::NDockPanel* patDock;
      ngrs::NDockPanel* seqDock;

      WaveEdFrame* waveEd_;

      void onTimer();
      void onTweakSlide( int machine, int command, int value );

      void onMachineSelected( Machine* mac );
      void onTabChange( ngrs::NButtonEvent* ev );

    };

  }
}

#endif
