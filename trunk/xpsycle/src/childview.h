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
#include "global.h"
#include <ntabbook.h>
#include <nfiledialog.h>
#include <ntimer.h>

class NewMachine;

/**
@author Stefan
*/


class ChildView : public NTabBook
{
public:
    ChildView();

    ~ChildView();

    void FileLoadSongNamed(std::string fName);
    void FileSaveSongNamed(std::string fName);

    void setTitleBarText( );

    // connect to signals
    void onFileLoadSong(NObject* sender);
    void onFileSaveSong(NObject* sender);

    void onPatternView(NObject* sender);

    signal0<> newSongLoaded;

    PatternView* patternView();
    MachineView* machineView();

    void playFromStart();
    void enableSound();

    NewMachine* newMachineDlg();

    NTimer timer;

private:

    Song* _pSong;
    NewMachine* newMachineDlg_;

    NFileDialog* getOpenFileName_;
    NFileDialog* getSaveFileName_;

    MachineView* machineView_;
    PatternView* patternView_;

    void OnFileLoadSongNamed(std::string fName, int fType);
    void onMachineViewDblClick(NButtonEvent* ev);

    void onTimer();

};



#endif
