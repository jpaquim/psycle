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
#include "patternboxproperties.h"

#include <map>
#include <ngrs/npanel.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nlistlayout.h>
#include <ngrs/ncustomtreeview.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nflowlayout.h>
#include <ngrs/nimage.h>
#include <ngrs/ngroupbox.h>
#include <ngrs/nitem.h>
#include <ngrs/ntreenode.h>

class N7SegDisplay;

/**
@author Stefan
*/

namespace psycle { namespace host {

class PatternView;


class CategoryTreeNode: public NTreeNode {
public:
    CategoryTreeNode(PatternCategory* cat);
    ~CategoryTreeNode();

    PatternCategory* category();

private:

    PatternCategory* cat_;

};


class CategoryItem : public NCustomItem {
public :
   CategoryItem(PatternCategory* category, const std::string & text);
   CategoryItem();
   ~CategoryItem();

   virtual void setText(const std::string & text);
   virtual std::string text() const;

   virtual void paint( NGraphics* g);

   PatternCategory* category();

private:

   NLabel* label_;

   PatternCategory* category_;

   void init();

};

class PatternItem : public NItem {
public:

   PatternItem(SinglePattern* pattern, const std::string & text);
   ~PatternItem();

   virtual void setText(const std::string & text);

private:

   SinglePattern* pattern_;

};

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

    NCustomTreeView* patternBox_;
    PatternBoxProperties* propertyBox_;


    NCheckBox* follow_;
    NCheckBox* multichannel_audition_;
    NCheckBox* record_noteoff_;
    NCheckBox* record_tweaks_;
    NCheckBox* notestoeffects_;
    NCheckBox* movecursorpaste_;

    void onNewCategory( NButtonEvent * ev );
    void onNewPattern( NButtonEvent* ev );

    void onItemSelected(NItemEvent* ev);
    void onPatternAdd(NButtonEvent* ev);

    void onMoveCursorPaste(NButtonEvent* ev);
    void onRecordTweakChange(NButtonEvent* ev);

    void onNameChanged(const std::string & name);

    std::map<NTreeNode*, PatternCategory*> categoryMap;
    std::vector<CategoryItem*> catItems;

    std::map<NCustomItem*, SinglePattern*> patternMap;

};

}}
#endif
