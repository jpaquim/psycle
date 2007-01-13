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
#ifndef PATTERNVIEW_H
#define PATTERNVIEW_H

#include "patdlg.h"
#include "song.h"
#include "singlepattern.h"
#include "custompatternview.h"
#include "skinreader.h"
#include <ngrs/npopupmenu.h>
#include <ngrs/nscrollbar.h>
#include <ngrs/nxmlparser.h>
#include <map>

/**
@author Stefan
*/

namespace ngrs {
  class NToolBar;
  class NComboBox;
  class NCheckBox;
  class NSplitBar;
  class NItemEvent;
}

class ZoomBar;

namespace psycle { 
  namespace host {


    class Song;

    class UndoPattern : public SinglePattern {
    public  :

      UndoPattern();
      UndoPattern( int patternId, const ngrs::NSize & changedBlock, const PatCursor & cursor  );

      ~UndoPattern();

      const ngrs::NSize & changedBlock() const;
      const PatCursor & oldCursor();

      int patternId();

    private :

      ngrs::NSize changedBlock_;
      int patternId_;
      PatCursor cursor_;

    };

    class PatternUndoManager : public std::vector<UndoPattern> {
    public :

      PatternUndoManager( );
      ~PatternUndoManager();

      void setSong( Song* pSong  );
      void setPattern( SinglePattern* pattern );
      void addUndo( const ngrs::NSize & block, const PatCursor & cursor );
      void addUndo( const PatCursor & cursor );

      void doUndo();

    private:

      SinglePattern* pattern_;
      Song* pSong_;

    };


    class PatternView : public ngrs::NPanel
    {
    public:

      class Header: public ngrs::NPanel {
      public:
        Header(PatternView* pPatternView);
        ~Header();

        virtual void paint( ngrs::NGraphics* g );

        void setHeaderCoordInfo( const HeaderCoordInfo & info );

        virtual void onMousePress(int x, int y, int button);
        virtual int preferredWidth();

        int skinColWidth();

      private:

        HeaderCoordInfo coords_;

        int skinColWidth_;
        PatternView* pView;

        void onSoloLedClick( int track );
        void onMuteLedClick( int track );
        void onRecLedClick( int track );

      };


      class LineNumber : public ngrs::NPanel {
      public:
        LineNumber( PatternView* pPatternView );
        ~LineNumber();

        virtual void paint( ngrs::NGraphics* g );

        void setDy( int dy );
        int dy() const;

        void setTextColor( const ngrs::NColor& textColor );
        const ngrs::NColor & textColor() const;

        virtual int preferredWidth() const;

      private:

        PatternView* pView;
        int dy_;
        ngrs::NColor textColor_;

      };



      class PatternDraw : public CustomPatternView {
      public:

        PatternDraw( PatternView* pPatternView );
        ~PatternDraw();

        virtual int colWidth() const;
        virtual int rowHeight() const;
        virtual int lineNumber() const;
        virtual int beatZoom() const;

        virtual void customPaint( ngrs::NGraphics* g, int startLine, int endLine, int startTrack, int endTrack );

        virtual void onMousePress( int x, int y, int button );
        virtual void onMousePressed( int x, int y, int button );
        virtual void onMouseOver( int x, int y );
        virtual void onKeyPress( const ngrs::NKeyEvent & event );
        virtual void onKeyRelease( const ngrs::NKeyEvent & event );

        void copyBlock( bool cutit );
        void pasteBlock( int tx, int lx, bool mix, bool save = true );
        void deleteBlock();
        void transposeBlock( int trp );
        void scaleBlock( float factor );

        void setSharpMode( bool on );
        bool sharpMode() const;

        virtual void resize();

      protected:

        virtual int doSel( const PatCursor & p );
        virtual void selectAll( const PatCursor & cursor );
        virtual void drawPattern( ngrs::NGraphics* g, int startLine, int endLine, int startTrack, int endTrack );


      private:

        ngrs::NPopupMenu* editPopup_;
        PatDlg* patDlg;
        PatternView* pView;      

        bool isBlockCopied;
        ngrs::NSize blockLastOrigin;
        bool sharpMode_;

        void clearCursorPos();

        void onPopupBlockCopy( ngrs::NButtonEvent* ev );
        void onPopupBlockCut( ngrs::NButtonEvent* ev );
        void onPopupBlockPaste( ngrs::NButtonEvent* ev );
        void onPopupBlockDelete( ngrs::NButtonEvent* ev );
        void onPopupBlockMixPaste( ngrs::NButtonEvent* ev );
        void onPopupTranspose1( ngrs::NButtonEvent* ev );
        void onPopupTranspose12( ngrs::NButtonEvent* ev );
        void onPopupTranspose_1( ngrs::NButtonEvent* ev );
        void onPopupTranspose_12( ngrs::NButtonEvent* ev );
        void onPopupPattern( ngrs::NButtonEvent* ev );

        SinglePattern pasteBuffer;
        void onTagParse( const ngrs::NXmlParser & parser, const std::string & tagName );
        float lastXmlLineBeatPos;
        int xmlTracks;
        float xmlBeats;

        void checkLeftScroll( const PatCursor & cursor );
        void checkRightScroll( const PatCursor & cursor );

      };

      class TweakHeader : public ngrs::NPanel {
      public:

        TweakHeader(  PatternView* pPatternView );

        ~TweakHeader();

        virtual int preferredWidth();
        virtual void paint( ngrs::NGraphics* g );

        int skinColWidth() const;

      private:

        ngrs::NRect bgCoords;
        ngrs::NRect noCoords;

        PatternView* pView;

      };


      class TweakGUI : public CustomPatternView {
      public:
        TweakGUI( PatternView* pPatternView);

        ~TweakGUI();			

        virtual int colWidth() const;
        virtual int rowHeight() const;
        virtual int lineNumber() const;
        virtual int trackNumber() const;
        virtual int beatZoom() const;

        virtual void customPaint( ngrs::NGraphics* g, int startLine, int endLine, int startTrack, int endTrack );

        virtual void onKeyPress( const ngrs::NKeyEvent & event );

        virtual void resize();

      protected:

        virtual int doSel( const PatCursor & p );
        void drawPattern( ngrs::NGraphics* g, int startLine, int endLine, int startTrack, int endTrack );

      private:

        PatternView* pView;

        void checkLeftScroll( const PatCursor & cursor );
        void checkRightScroll( const PatCursor & cursor );

      };

    public:

      friend class Header;

      PatternView( Song * song );

      ~PatternView();

      Song* pSong();

      signal1<int> lineChanged;

      void setSeparatorColor( const ngrs::NColor & separatorColor );
      const ngrs::NColor & separatorColor();


      int rowHeight() const;

      int tweakColWidth() const;
      int colWidth() const;
      const std::map<int, TrackGeometry> & trackGeometrics() const;
      int headerWidth() const;
      int tweakHeaderWidth() const;
      int headerHeight() const;

      int lineNumber() const;
      int trackNumber() const;

      void setEditPosition( int pos );
      void setPrevEditPosition( int pos );
      int editPosition() const;
      int prevEditPosition() const;
      int playPos() const;

      void setPatternStep( int step );
      int patternStep() const;

      void updatePlayBar( bool followSong );

      void setEditOctave( int octave );
      int editOctave() const;

      ngrs::NScrollBar* vScrBar();
      ngrs::NScrollBar* hScrBar();

      void PlayNote( int note, int velocity, bool bTranspose, psycle::host::Machine*pMachine);
      void StopNote( int note, bool bTranspose=true, psycle::host::Machine* pMachine=NULL);

      void noteOffAny( const PatCursor & cursor );

      void copyBlock( bool cutit );
      void pasteBlock( int tx, int lx, bool mix, bool save = true );
      void blockTranspose( int trp );
      void deleteBlock();
      void doubleLength();
      void halveLength();

      void setMoveCursorWhenPaste( bool on );
      bool moveCursorWhenPaste() const;

      ngrs::NRect repaintLineNumberArea( int startLine, int endLine );
      void repaintLineNumber( int startLine, int endLine );

      void setPattern( SinglePattern* pattern );
      SinglePattern* pattern();

      void setBeatZoom( int tpb );
      int beatZoom() const;


      void setActiveMachineIdx( int idx );
      int selectedMachineIndex() const;

      void clearCurr();

      virtual void setFocus();

      void updateRange();
      void updateSkin();

      void setColorInfo( const PatternViewColorInfo & info );
      const PatternViewColorInfo & colorInfo() const;

      void onTick( double sequenceStart );
      void onStartPlayBar();
      void onEndPlayBar();

      PatternUndoManager & undoManager();

      void doUndo();

    private:

      Song* _pSong;
      SinglePattern* pattern_;
      ngrs::NXmlParser xmlParser;
      PatternUndoManager undoManager_;

      int editPosition_, prevEditPosition_;
      int playPos_;
      int editOctave_;
      int selectedMacIdx_;
      bool moveCursorWhenPaste_;

      PatternDraw* drawArea;
      TweakGUI* tweakGUI;
      TweakHeader* tweakHeader;

      ngrs::NScrollBar* hBar;
      ngrs::NScrollBar* tweakHBar;
      ngrs::NCheckBox* sideBox;
      ZoomBar* zoomHBar;
      ngrs::NScrollBar* vBar;
      Header*     header;
      LineNumber* lineNumber_;
      ngrs::NColor      separatorColor_;
      ngrs::NToolBar*   toolBar;
      ngrs::NComboBox* patternCombo_;
      ngrs::NComboBox* octaveCombo_;
      ngrs::NComboBox*  meterCbx;
      ngrs::NComboBox* trackCombo_;
      ngrs::NSplitBar* splitBar;
      ngrs::NPanel* tweakGroup;
      ngrs::NPanel* lineHeaderLabel;
      ngrs::NButton* sharpBtn_;

      PatternViewColorInfo colorInfo_;

      void enterNote( const PatCursor & cursor, int note );

      void resize();

      void onHScrollBar( ngrs::NScrollBar* sender );
      void onHTweakScrollBar( ngrs::NScrollBar* sender );
      void onVScrollBar( ngrs::NScrollBar* sender );

      void initToolBar();
      void onAddBar( ngrs::NButtonEvent* ev );
      void onDeleteBar( ngrs::NButtonEvent* ev );

      /// multi-key playback state stuff
      int notetrack[MAX_TRACKS];
      int outtrack;

      void onZoomHBarPosChanged( ZoomBar* zoomBar, double newPos );
      void onPatternStepChange( ngrs::NItemEvent* ev );
      void onOctaveChange( ngrs::NItemEvent* ev );
      void onTrackChange( ngrs::NItemEvent* ev );
      void onSideChange( ngrs::NButtonEvent* ev );
      void onToggleSharpMode( ngrs::NButtonEvent* ev );

      void checkUpScroll( const PatCursor & cursor );
      void checkDownScroll( const PatCursor & cursor );

    };

  }
}
#endif
