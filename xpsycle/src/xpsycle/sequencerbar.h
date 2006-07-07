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
#ifndef SEQUENCERBAR_H
#define SEQUENCERBAR_H

#include "patterndata.h"

#include <map>
#include <ngrs/npanel.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nlistlayout.h>
#include <ngrs/nlistbox.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nflowlayout.h>
#include <ngrs/nimage.h>
#include <ngrs/ngroupbox.h>
#include <ngrs/nitem.h>


class N7SegDisplay;

/**
@author Stefan
*/

namespace psycle { namespace host {

class PatternView;

class SequencerBar : public NPanel
{


public:
    SequencerBar();

    ~SequencerBar();

    signal1<SinglePattern*> selected;
    signal1<SinglePattern*> added;

    void setPatternData(PatternData* data);

    bool followSong() const;

    void updateSequencer();

    void onSelChangeSeqList(NItemEvent* sender);

private:

    PatternData* patternData_;

    void init();

    int counter;

    NListBox* patternBox_;


    NCheckBox* follow_;
    NCheckBox* multichannel_audition_;
    NCheckBox* record_noteoff_;
    NCheckBox* record_tweaks_;
    NCheckBox* notestoeffects_;
    NCheckBox* movecursorpaste_;

    void onNewPattern( NButtonEvent * ev );
    void onDeletePattern( NButtonEvent* ev );

    void onItemSelected(NItemEvent* ev);
    void onPatternAdd(NButtonEvent* ev);

    void onMoveCursorPaste(NButtonEvent* ev);
    void onRecordTweakChange(NButtonEvent* ev);

    std::map<NCustomItem*, SinglePattern*> itemMap;
};

}}
#endif
