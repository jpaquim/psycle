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

#include "patternsequence.h"
#include "patternboxproperties.h"
#include "childview.h"

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
#include <ngrs/npopupmenu.h>

/**
@author Stefan
*/

namespace ngrs {
  class NObjectInspector;
}

namespace psycle { 
  namespace host {

    class PatternView;
    class SequencerGUI;

    class CategoryTreeNode: public ngrs::NTreeNode {
    public:
      CategoryTreeNode( PatternCategory* cat );
      ~CategoryTreeNode();

      PatternCategory* category();

    private:

      PatternCategory* cat_;

    };


    class CategoryItem : public ngrs::NCustomItem {
    public :
      CategoryItem(PatternCategory* category, const std::string & text);
      CategoryItem();
      ~CategoryItem();

      virtual void setText(const std::string & text);
      virtual std::string text() const;

      virtual void paint( ngrs::NGraphics* g);

      PatternCategory* category();

    private:

      ngrs::NLabel* label_;

      PatternCategory* category_;

      void init();

    };

    class PatternItem : public ngrs::NItem {
    public:

      PatternItem(SinglePattern* pattern, const std::string & text);
      ~PatternItem();

      virtual void setText(const std::string & text);

    private:

      SinglePattern* pattern_;

    };

    class SequencerBar : public ngrs::NPanel
    {



    public:
      SequencerBar();

      ~SequencerBar();

      signal1<SinglePattern*> removed;

      void setSequenceGUI( SequencerGUI* sequenceGUI );
      void setPatternView( PatternView* patternView );

      bool followSong() const;

      void update();

      void onSelChangeSeqList( ngrs::NItemEvent* sender );

      void setEntry( ngrs::NObject* obj );
      void setChildView( class ChildView* view );

      void selectNextPattern();
      void selectPrevPattern();

      void setSkin();

    private:

      SequencerGUI* seqGui;
      PatternView* patView;
      ChildView* childView_;

      void init();

      int counter;

      ngrs::NCustomTreeView* patternBox_;
      PatternBoxProperties* propertyBox_;
      ngrs::NObjectInspector* entryBox_;

      ngrs::NCheckBox* follow_;
      ngrs::NCheckBox* multichannel_audition_;
      ngrs::NCheckBox* record_noteoff_;
      ngrs::NCheckBox* record_tweaks_;
      ngrs::NCheckBox* notestoeffects_;
      ngrs::NCheckBox* movecursorpaste_;


      void onNewCategory( ngrs::NButtonEvent * ev );
      void onNewPattern( ngrs::NButtonEvent* ev );
      void onClonePattern( ngrs::NButtonEvent* ev );
      void onDeletePattern( ngrs::NButtonEvent* ev );

      void onItemSelected( ngrs::NItemEvent* ev );
      void onPatternItemDblClick( ngrs::NButtonEvent * ev );
      void onPatternAdd( ngrs::NButtonEvent* ev );
      void switchPatternViewPattern( ngrs::NCustomItem* item );

      void onMoveCursorPaste( ngrs::NButtonEvent* ev );
      void onRecordTweakChange( ngrs::NButtonEvent* ev );

      void onNameChanged( const std::string & name );

      std::map<ngrs::NTreeNode*, PatternCategory*> categoryMap;
      std::vector<CategoryItem*> catItems;

      std::map<ngrs::NCustomItem*, SinglePattern*> patternMap;

    };

  }
}
#endif
