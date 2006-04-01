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

#include <npanel.h>
#include <ncheckbox.h>
#include <nlistlayout.h>
#include <nlistbox.h>
#include <nalignlayout.h>
#include <nflowlayout.h>
#include <nframeborder.h>
#include <ngridlayout.h>
#include <nimage.h>
#include <ngroupbox.h>

/**
@author Stefan
*/

class PatternView;

class SequencerBar : public NPanel
{
public:
    SequencerBar();
    SequencerBar(PatternView* view_);

    ~SequencerBar();

    void setPatternView(PatternView* patternView);

    void updateSequencer();

    NListBox* seqList();

    void onSelChangeSeqList(NItemEvent* sender);

private:

   PatternView* patternView_;

   void init();

   NPanel* seqPanel_;
   NListBox* seqList_;

   NButton* incshort_;
   NButton* decshort_;
   NButton* inclong_;
   NButton* declong_;
   NButton* seqnew_;
   NButton* seqduplicate_;
   NButton* seqins_;
   NButton* seqcut_;
   NButton* seqcopy_;
   NButton* seqpaste_;
   NButton* seqdelete_;
   NButton* seqclr_;
   NButton* seqsrt_;
   NButton* declen_;
   NButton* inclem_;

   NCheckBox* follow_;
   NCheckBox* multichannel_audition_;
   NCheckBox* record_noteoff_;
   NCheckBox* record_tweaks_;
   NCheckBox* notestoeffects_;
   NCheckBox* movecirsorpaste_;

   void onIncShort(NButtonEvent* ev);
   void onDecShort(NButtonEvent* ev);
   void onIncLong(NButtonEvent* ev);
   void onDecLong(NButtonEvent* ev);
   void onSeqNew(NButtonEvent* ev);
   void onSeqIns(NButtonEvent* ev);
   void onSeqCopy(NButtonEvent* ev);
   void onSeqPaste(NButtonEvent* ev);

   std::vector<int> seqCopyBuffer;

   void updatePlayOrder(bool mode);

   NFrameBorder* frBorder;
   NLayout* listLayout;
   NGridLayout* gridLayout;
};

#endif
