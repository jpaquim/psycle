/***************************************************************************
  *   Copyright (C) 2006 by  Stefan   *
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
#include <ngrs/tabbook.h>
#include <ngrs/timer.h>
#include <ngrs/tabbook.h>

/**
  @author  Stefan
*/

namespace psycle {
  namespace host {

    class SequencerGUI;
    class VirtualPattern;
    class SequencerBar;


    class ChildView : public ngrs::Panel {
    public:
      ChildView( );

      ~ChildView();

      Song* song();

      void setTitleBarText( );

      void onPatternView(Object* sender);

      // connect to signals
      sigslot::signal1<Machine*> newMachineAdded;
      sigslot::signal1<Machine*> machineSelected;
      sigslot::signal1<ngrs::ButtonEvent*> machineViewDblClick;

      PatternView* patternView();
      MachineView* machineView();
      SequencerBar* sequencerBar();
      SequencerGUI* sequencerView();
      VirtualPattern* virtualPattern();

      void play();
      void playFromStart();
      void stop();

      ngrs::Timer timer;

      void onMachineViewDblClick( ngrs::ButtonEvent* ev );

      void showMachineView();
      void showPatternView();
      void showWaveView();
      void showSequencerView();

      void update();

    private:

      Song* _pSong;

      ngrs::TabBook* tabBook_;

      // ngrs::FileDialog* getSaveFileName_;

      SequencerBar* sequencerBar_;
      MachineView* machineView_;
      PatternView* patternView_;
      SequencerGUI* sequencerView_;
      VirtualPattern* virtualPattern_;

      ngrs::DockPanel* macDock;
      ngrs::DockPanel* patDock;
      ngrs::DockPanel* seqDock;

      void onTimer();
      void onTweakSlide( int machine, int command, int value );

      void onMachineSelected( Machine* mac );
      void onTabChange( ngrs::ButtonEvent* ev );

    };

  }
}

#endif
