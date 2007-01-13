/***************************************************************************
*   Copyright (C) 2006 by Stefan ngrs::Nattkemper   *
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

#include "singlepattern.h"
#include "patternsequence.h"
#include "audiodriver.h"
#include <ngrs/npanel.h>
#include <ngrs/nbutton.h>
#include <ngrs/nline.h>
#include <ngrs/nhint.h>
#include <list>

/**
@author Stefan Nattkemper
*/

namespace ngrs {
  class NScrollBar;
  class NLabel;
  class NToolBar;
  class NListBox;  
  class NCheckBox;
  class NLine;
  class NEdit;
}

class ZoomBar;

namespace psycle {
  namespace host {

    class SequencerBeatChangeLineal;

    class SequencerGUI;

    class SequencerLoopItem : public ngrs::NPanel {
    public:

      SequencerLoopItem( SequencerGUI* seqGui );

      ~SequencerLoopItem();

      virtual void paint( ngrs::NGraphics* g);

      virtual int preferredWidth() const;
      virtual int preferredHeight() const;

      virtual void resize();

    private:

      SequencerGUI* sView;
      ngrs::NEdit* loopEdit;

    };

    class SequencerItem : public ngrs::NPanel {
    public:
      SequencerItem( SequencerGUI* seqGui );
      ~SequencerItem();

      signal1<SequencerItem*> click;

      virtual void paint( ngrs::NGraphics* g);
      virtual void onMousePress(int x, int y, int button);
      virtual void resize();
      virtual void onMove(const ngrs::NMoveEvent & moveEvent);
      virtual void onMoveEnd(const ngrs::NMoveEvent & moveEvent);
      virtual void onMoveStart(const ngrs::NMoveEvent & moveEvent);

      void setText(const std::string & text);

      void setSequenceEntry(SequenceEntry* entry);
      SequenceEntry* sequenceEntry();

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

      SequenceEntry* sequenceEntry_;
      SequencerGUI* sView;

      ngrs::NRegion entriesInRegion();
      ngrs::NRegion oldDrag;

      ngrs::NHint* hint_;

      int oldLeft;

    };

    class SequencerGUI : public ngrs::NPanel
    {
    public:
      class SequencerLine : public ngrs::NPanel {
      public :

        SequencerLine( SequencerGUI* seqGui );
        ~SequencerLine();

        signal1<SequencerLine*> click;
        signal1<SequencerItem*> itemClick;

        virtual void paint( ngrs::NGraphics* g );
        virtual void onMousePress( int x, int y, int button );
        virtual int preferredWidth() const;
        virtual int preferredHeight() const;

        virtual void removeChild( ngrs::NVisualComponent* item );

        void setSequenceLine( SequenceLine* line );
        SequenceLine* sequenceLine();

        void addItem( SinglePattern* pattern );
        void removeItems( SinglePattern* pattern );

        std::vector<SequencerItem*> itemsByPattern( SinglePattern* pattern );

        virtual void resize();

        std::list<SequencerItem*> items;

        void onSequencerItemClick( SequencerItem* item );

      private:

        SequenceLine* seqLine_;
        SequencerGUI* sView;

      };


      class SequencerBeatLineal : public ngrs::NPanel {
      public :
        SequencerBeatLineal( SequencerGUI* seqGui );
        ~SequencerBeatLineal();

        virtual void paint( ngrs::NGraphics* g );
        virtual int preferredHeight() const;

      private:

        SequencerGUI* sView;

        void drawLineal( ngrs::NGraphics* g, int dx );

      };


      class Area : public ngrs::NPanel {
      public :
        Area( SequencerGUI* seqGui );
        ~Area();

        void drawTimeGrid( ngrs::NGraphics* g );

        virtual void paint( ngrs::NGraphics* g );
        virtual int preferredWidth() const;
        virtual int preferredHeight() const;
        virtual void resize();

        virtual void removeChilds();

        ngrs::NPanel* vLine();
        ngrs::NPanel* vLine() const;

        NPanel* pLine(); // playpos line
        ngrs::NPanel* pLine() const;

        bool lockPlayLine() const;

        virtual void onMove(const ngrs::NMoveEvent & moveEvent);
        virtual void onMoveEnd(const ngrs::NMoveEvent & moveEvent);
        virtual void onMoveStart(const ngrs::NMoveEvent & moveEvent);

      private:

        SequencerGUI* sView;

        ngrs::NPanel* vLine_;
        ngrs::NPanel* pLine_; // playPosLine

        double playPos_;
        double newBeatPos_;

        bool lockPlayLine_;
        bool playing_;

      };

    public:

      SequencerGUI();

      ~SequencerGUI();

      signal1<SequencerItem*> entryClick;

      void setPatternSequence( PatternSequence* sequence );
      PatternSequence* patternSequence();

      void addPattern( SinglePattern* pattern );
      void removePattern( SinglePattern* pattern );

      std::vector<SequencerItem*> guiItemsByPattern( SinglePattern* pattern );

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

      AudioDriver oldDriver;

      int counter;
      int beatPxLength_;
      double oldPlayPos_;

      ngrs::NTimer recStatusTimer;

      ngrs::NCheckBox* snapToGridCheck_;
      SequencerBeatLineal* beatLineal_;
      SequencerBeatChangeLineal* beatChangeLineal_;
      SequencerLine* lastLine;
      std::vector<SequencerItem*> selectedItems_;
      PatternSequence* patternSequence_;
      std::vector<SequencerLine*> lines;

      ngrs::NScrollBar* hBar;
      ngrs::NScrollBar* vBar;
      ngrs::NButton* renderBtn;

      ZoomBar* zoomHBar;

      Area* scrollArea_;
      ngrs::NToolBar* toolBar_;
      ngrs::NListBox* patternBox_;

      void onInsertTrack( ngrs::NButtonEvent* ev );
      void onDeleteTrack( ngrs::NButtonEvent* ev );
      void onMoveDownTrack( ngrs::NButtonEvent* ev );
      void onMoveUpTrack( ngrs::NButtonEvent* ev );

      void onNewPattern( ngrs::NButtonEvent* ev );
      void onDeleteEntry( ngrs::NButtonEvent* ev );
      void onAddLoop( ngrs::NButtonEvent* ev );
      void onRefreshGUI( ngrs::NButtonEvent* ev );

      void onSequencerLineClick( SequencerLine* line );
      void onSequencerItemClick( SequencerItem* item );
      void onZoomHBarPosChanged( ZoomBar* zoomBar, double newPos );
      void onHScrollBar( ngrs::NScrollBar* sender );
      void onVScrollBar( ngrs::NScrollBar* sender );

      void onRenderAsWave( ngrs::NButtonEvent* ev );
      void onRecordingTimer();

    };

  }
}

#endif
