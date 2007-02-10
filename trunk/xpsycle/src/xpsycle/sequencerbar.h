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
#ifndef SEQUENCERBAR_H
#define SEQUENCERBAR_H

#include "patternsequence.h"
#include "patternboxproperties.h"
#include "childview.h"

#include <map>
#include <ngrs/panel.h>
#include <ngrs/checkbox.h>
#include <ngrs/listlayout.h>
#include <ngrs/ncustomtreeview.h>
#include <ngrs/alignlayout.h>
#include <ngrs/flowlayout.h>
#include <ngrs/image.h>
#include <ngrs/groupbox.h>
#include <ngrs/item.h>
#include <ngrs/ntreenode.h>
#include <ngrs/popupmenu.h>

/**
@author  Stefan
*/

namespace ngrs {
  class ObjectInspector;
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


    class CategoryItem : public ngrs::CustomItem {
    public :
      CategoryItem(PatternCategory* category, const std::string & text);
      CategoryItem();
      ~CategoryItem();

      virtual void setText(const std::string & text);
      virtual std::string text() const;

      virtual void paint( ngrs::Graphics& g);

      PatternCategory* category();

    private:

      ngrs::Label* label_;

      PatternCategory* category_;

      void init();

    };

    class PatternItem : public ngrs::Item {
    public:

      PatternItem(SinglePattern* pattern, const std::string & text);
      ~PatternItem();

      virtual void setText(const std::string & text);

    private:

      SinglePattern* pattern_;

    };

    class SequencerBar : public ngrs::Panel
    {



    public:
      SequencerBar();

      ~SequencerBar();

      sigslot::signal1<SinglePattern*> removed;

      void setSequenceGUI( SequencerGUI* sequenceGUI );
      void setPatternView( PatternView* patternView );

      bool followSong() const;

      void update();

      void onSelChangeSeqList( ngrs::ItemEvent* sender );

      void setEntry( ngrs::Object* obj );
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
      ngrs::ObjectInspector* entryBox_;

      ngrs::NCheckBox* follow_;
      ngrs::NCheckBox* multichannel_audition_;
      ngrs::NCheckBox* record_noteoff_;
      ngrs::NCheckBox* record_tweaks_;
      ngrs::NCheckBox* notestoeffects_;
      ngrs::NCheckBox* movecursorpaste_;


      void onNewCategory( ngrs::ButtonEvent * ev );
      void onNewPattern( ngrs::ButtonEvent* ev );
      void onClonePattern( ngrs::ButtonEvent* ev );
      void onDeletePattern( ngrs::ButtonEvent* ev );

      void onItemSelected( ngrs::ItemEvent* ev );
      void onPatternItemDblClick( ngrs::ButtonEvent * ev );
      void onPatternAdd( ngrs::ButtonEvent* ev );
      void switchPatternViewPattern( ngrs::CustomItem* item );

      void onMoveCursorPaste( ngrs::ButtonEvent* ev );
      void onRecordTweakChange( ngrs::ButtonEvent* ev );

      void onNameChanged( const std::string & name );

      std::map<ngrs::NTreeNode*, PatternCategory*> categoryMap;
      std::vector<CategoryItem*> catItems;

      std::map<ngrs::CustomItem*, SinglePattern*> patternMap;

    };

  }
}
#endif
