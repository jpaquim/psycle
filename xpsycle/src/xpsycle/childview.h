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


namespace psycle {
	namespace host {


class WaveEdFrame;
class SequencerGUI;

/**
@author Stefan
*/


class ChildView : public NPanel
{
public:
    ChildView( );

    ~ChildView();

    Song* song();

    void setTitleBarText( );
    
    void onPatternView(NObject* sender);

    // connect to signals
    signal1<Machine*> newMachineAdded;
    signal1<Machine*> machineSelected;
    signal1<NButtonEvent*> machineViewDblClick;

    PatternView* patternView();
    MachineView* machineView();
    SequencerBar* sequencerBar();
    WaveEdFrame* waveEditor();
    SequencerGUI* sequencerView();

    void play();
    void playFromStart();
    void enableSound();

    NTimer timer;

    void onMachineViewDblClick(NButtonEvent* ev);

		void showMachineView();
		void showPatternView();
    void showSequencerView();

		void update();

private:

    Song* _pSong;

    NTabBook* tabBook_;

//    NFileDialog* getSaveFileName_;

    SequencerBar* sequencerBar_;
    MachineView* machineView_;
    PatternView* patternView_;
    SequencerGUI* sequencerView_;


    WaveEdFrame* waveEd_;

    std::string OnFileLoadSongNamed(const std::string & fName, int fType);

    void onTimer();
    void onTweakSlide(int machine, int command, int value);

    void onMachineSelected( Machine* mac );

};

}
}

#endif
