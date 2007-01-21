/***************************************************************************
*   Copyright (C) 2006 by  Stefan ngrs::Nattkemper   *
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
#ifndef SEQUENCERGUI_H
#define SEQUENCERGUI_H

#include <psycore/singlepattern.h>
#include <psycore/patternsequence.h>
#include <ngrs/panel.h>
#include <ngrs/button.h>
#include <ngrs/line.h>
#include <ngrs/hint.h>
#include <list>

/**
@author  Stefan Nattkemper
*/

namespace ngrs {
  class ScrollBar;
  class Label;
  class ToolBar;
  class ListBox;  
  class NCheckBox;
  class Line;
  class Edit;
}

class ZoomBar;

namespace psy {
  namespace host {

    class SequencerBeatChangeLineal;

    class SequencerGUI;

    class SequencerLoopItem : public ngrs::Panel {
    public:

      SequencerLoopItem( SequencerGUI* seqGui );

      ~SequencerLoopItem();

      virtual void paint( ngrs::Graphics& g);

      virtual int preferredWidth() const;
      virtual int preferredHeight() const;

      virtual void resize();

    private:

      SequencerGUI* sView;
      ngrs::Edit* loopEdit;

    };

    class SequencerItem : public ngrs::Panel {
    public:
      SequencerItem( SequencerGUI* seqGui );
      ~SequencerItem();

      sigslot::signal1<SequencerItem*> click;

      virtual void paint( ngrs::Graphics& g);
      virtual void onMousePress(int x, int y, int button);
      virtual void resize();
      virtual void onMove(const ngrs::MoveEvent & moveEvent);
      virtual void onMoveEnd(const ngrs::MoveEvent & moveEvent);
      virtual void onMoveStart(const ngrs::MoveEvent & moveEvent);

      void setText(const std::string & text);

      void setSequenceEntry( psy::core::SequenceEntry* entry );
      psy::core::SequenceEntry* sequenceEntry();

      void setSelected(bool on);
      bool selected();

      void setStart(float start);
      float start() const;

      void setEndOffset(float start);
      float endOffset() const;

      void setTranspose( int offset );
      int transpose() const;

      ngrs::NHint* hint();


    private:

      bool selected_;

      psy::core::SequenceEntry* sequenceEntry_;
      SequencerGUI* sView;

      ngrs::Region entriesInRegion();
      ngrs::Region oldDrag;

      ngrs::NHint* hint_;

      int oldLeft;

    };

    class SequencerGUI : public ngrs::Panel
    {
    public:
      class SequencerLine : public ngrs::Panel {
      public :

        SequencerLine( SequencerGUI* seqGui );
        ~SequencerLine();

        sigslot::signal1<SequencerLine*> click;
        sigslot::signal1<SequencerItem*> itemClick;

        virtual void paint( ngrs::Graphics& g );
        virtual void onMousePress( int x, int y, int button );
        virtual int preferredWidth() const;
        virtual int preferredHeight() const;

        virtual void removeChild( ngrs::VisualComponent* item );

        void setSequenceLine( psy::core::SequenceLine* line );
        psy::core::SequenceLine* sequenceLine();

        void addItem( const std::list<psy::core::SinglePattern>::iterator& patternItr );
        void removeItems( const std::list<psy::core::SinglePattern>::iterator& patternItr );

        std::vector<SequencerItem*> itemsByPattern( const std::list<psy::core::SinglePattern>::iterator& patternItr );

        virtual void resize();

        std::list<SequencerItem*> items;

        void onSequencerItemClick( SequencerItem* item );

      private:

        psy::core::SequenceLine* seqLine_;
        SequencerGUI* sView;

      };


      class SequencerBeatLineal : public ngrs::Panel {
      public :
        SequencerBeatLineal( SequencerGUI* seqGui );
        ~SequencerBeatLineal();

        virtual void paint( ngrs::Graphics& g );
        virtual int preferredHeight() const;

      private:

        SequencerGUI* sView;

        void drawLineal( ngrs::Graphics& g, int dx );

      };


      class Area : public ngrs::Panel {
      public :
        Area( SequencerGUI* seqGui );
        ~Area();

        void drawTimeGrid( ngrs::Graphics& g );

        virtual void paint( ngrs::Graphics& g );
        virtual int preferredWidth() const;
        virtual int preferredHeight() const;
        virtual void resize();

        virtual void removeChilds();

        ngrs::Panel* vLine();
        ngrs::Panel* vLine() const;

        Panel* pLine(); // playpos line
        ngrs::Panel* pLine() const;

        bool lockPlayLine() const;

        virtual void onMove(const ngrs::MoveEvent & moveEvent);
        virtual void onMoveEnd(const ngrs::MoveEvent & moveEvent);
        virtual void onMoveStart(const ngrs::MoveEvent & moveEvent);

      private:

        SequencerGUI* sView;

        ngrs::Panel* vLine_;
        ngrs::Panel* pLine_; // playPosLine

        double playPos_;
        double newBeatPos_;

        bool lockPlayLine_;
        bool playing_;

      };

    public:

      SequencerGUI();

      ~SequencerGUI();

      sigslot::signal1<SequencerItem*> entryClick;

      void setPatternSequence( psy::core::PatternSequence* sequence );
      psy::core::PatternSequence* patternSequence();

      void addPattern( const std::list<psy::core::SinglePattern>::iterator& patternItr );
      void removePattern( const std::list<psy::core::SinglePattern>::iterator& patternItr );

      std::vector<SequencerItem*> guiItemsByPattern( const std::list<psy::core::SinglePattern>::iterator& patternItr );

      int beatPxLength() const;

      SequencerLine* selectedLine_;

      void update();

      virtual void resize();

      void addSequencerLine();

      const std::vector<SequencerItem*> & selectedItems();

      Area* scrollArea();

      void deselectAll();

      bool gridSnap() const;

      void updateSkin();

      void updatePlayPos();

    private:

      int counter;
      int beatPxLength_;
      double oldPlayPos_;

      ngrs::Timer recStatusTimer;

      ngrs::NCheckBox* snapToGridCheck_;
      SequencerBeatLineal* beatLineal_;
      SequencerBeatChangeLineal* beatChangeLineal_;
      SequencerLine* lastLine;
      std::vector<SequencerItem*> selectedItems_;
      psy::core::PatternSequence* patternSequence_;
      std::vector<SequencerLine*> lines;

      ngrs::ScrollBar* hBar;
      ngrs::ScrollBar* vBar;
      ngrs::Button* renderBtn;

      ZoomBar* zoomHBar;

      Area* scrollArea_;
      ngrs::ToolBar* toolBar_;
      ngrs::ListBox* patternBox_;

      void onInsertTrack( ngrs::ButtonEvent* ev );
      void onDeleteTrack( ngrs::ButtonEvent* ev );
      void onMoveDownTrack( ngrs::ButtonEvent* ev );
      void onMoveUpTrack( ngrs::ButtonEvent* ev );

      void onNewPattern( ngrs::ButtonEvent* ev );
      void onDeleteEntry( ngrs::ButtonEvent* ev );
      void onAddLoop( ngrs::ButtonEvent* ev );
      void onRefreshGUI( ngrs::ButtonEvent* ev );

      void onSequencerLineClick( SequencerLine* line );
      void onSequencerItemClick( SequencerItem* item );
      void onZoomHBarPosChanged( ZoomBar* zoomBar, double newPos );
      void onHScrollBar( ngrs::ScrollBar* sender );
      void onVScrollBar( ngrs::ScrollBar* sender );

      void onRenderAsWave( ngrs::ButtonEvent* ev );
      void onRecordingTimer();

    };

  }
}

#endif
