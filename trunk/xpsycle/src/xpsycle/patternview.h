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
#include <ngrs/npage.h>
#include <ngrs/npopupmenu.h>
#include <ngrs/nscrollbar.h>
#include <ngrs/nxmlparser.h>
#include <map>

/**
@author Stefan
*/

class NToolBar;
class NComboBox;
class NCheckBox;
class NSplitBar;
class ZoomBar;
class NItemEvent;

namespace psycle { namespace host {


class Song;

class UndoPattern : public SinglePattern {
public  :

	UndoPattern();
	UndoPattern( int patternId, const NSize & changedBlock, const PatCursor & cursor  );

	~UndoPattern();

	const NSize & changedBlock() const;
	const PatCursor & oldCursor();

	int patternId();

	private :

		NSize changedBlock_;
		int patternId_;
		PatCursor cursor_;

};

class PatternUndoManager : public std::vector<UndoPattern> {
public :

	PatternUndoManager( );
	~PatternUndoManager();

	void setSong( Song* pSong  );
	void setPattern( SinglePattern* pattern );
	void addUndo( const NSize & block, const PatCursor & cursor );
	void addUndo( const PatCursor & cursor );

  void doUndo();

private:

	SinglePattern* pattern_;
	Song* pSong_;

};


class PatternView : public NPanel
{
	public:

		class Header: public NPanel {
    public:
      Header(PatternView* pPatternView);
      ~Header();

      virtual void paint(NGraphics* g);

			void setHeaderCoordInfo( const HeaderCoordInfo & info );
  
      virtual void onMousePress(int x, int y, int button);
      virtual int preferredWidth();

      int skinColWidth();

    private:

			HeaderCoordInfo coords_;

      int skinColWidth_;
      PatternView* pView;

      void onSoloLedClick(int track);
      void onMuteLedClick(int track);
      void onRecLedClick(int track);

    };


    class LineNumber : public NPanel {
    public:
      LineNumber(PatternView* pPatternView);
      ~LineNumber();

      virtual void paint(NGraphics* g);

      void setDy(int dy);
      int dy() const;

      void setTextColor( const NColor& textColor );
			const NColor & textColor() const;

			virtual int preferredWidth() const;

    private:
      PatternView* pView;
      int dy_;

			NColor textColor_;
    };
		


    class PatternDraw : public CustomPatternView {
    public:

      PatternDraw(PatternView* pPatternView);
      ~PatternDraw();

			virtual int colWidth() const;
			virtual int rowHeight() const;
			virtual int lineNumber() const;
			virtual int beatZoom() const;

      virtual void customPaint(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);
      
      virtual void onMousePress(int x, int y, int button);
      virtual void onMousePressed(int x, int y, int button);
      virtual void onMouseOver	(int x, int y);
      virtual void onKeyPress(const NKeyEvent & event);
      virtual void onKeyRelease(const NKeyEvent & event);

      void copyBlock(bool cutit);
      void pasteBlock(int tx,int lx,bool mix,bool save = true);
      void deleteBlock();
      void transposeBlock(int trp);
      void scaleBlock(float factor);

			void setSharpMode( bool on );
			bool sharpMode() const;

      virtual void resize();

		protected:

			virtual int doSel(const PatCursor & p);
			virtual void drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);


    private:

      NPopupMenu* editPopup_;
      PatDlg* patDlg;
      PatternView* pView;      
       
      bool isBlockCopied;
      NSize blockLastOrigin;
			bool sharpMode_;

      void clearCursorPos();

      void onPopupBlockCopy(NButtonEvent* ev);
      void onPopupBlockCut(NButtonEvent* ev);
      void onPopupBlockPaste(NButtonEvent* ev);
      void onPopupBlockDelete(NButtonEvent* ev);
      void onPopupBlockMixPaste(NButtonEvent* ev);
      void onPopupTranspose1(NButtonEvent* ev);
      void onPopupTranspose12(NButtonEvent* ev);
      void onPopupTranspose_1(NButtonEvent* ev);
      void onPopupTranspose_12(NButtonEvent* ev);
      void onPopupPattern(NButtonEvent* ev);

			SinglePattern pasteBuffer;
      void onTagParse( const NXmlParser & parser, const std::string & tagName );
      float lastXmlLineBeatPos;
			int xmlTracks;
			float xmlBeats;

      void checkLeftScroll( const PatCursor & cursor );
			void checkRightScroll( const PatCursor & cursor );

    };

    class TweakHeader : public NPanel {
			public:

				TweakHeader(  PatternView* pPatternView );

				~TweakHeader();

        virtual int preferredWidth();
        virtual void paint( NGraphics* g );

        int skinColWidth() const;

      private:

        NRect bgCoords;
        NRect noCoords;

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

        virtual void customPaint(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);

				virtual void onKeyPress(const NKeyEvent & event);        

        virtual void resize();

    protected:

        virtual int doSel(const PatCursor & p);
        void drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);

    private:

        PatternView* pView;

        void checkLeftScroll( const PatCursor & cursor );
			  void checkRightScroll( const PatCursor & cursor );

		};

public:

		friend class Header;

    PatternView( Song * song);

    ~PatternView();

    Song* pSong();

    signal1<int> lineChanged;

    void setSeparatorColor(const NColor & separatorColor);
    const NColor & separatorColor();


    int rowHeight() const;

		int tweakColWidth() const;
		int colWidth() const;
		const std::map<int, TrackGeometry> & trackGeometrics() const;
    int headerWidth() const;
    int tweakHeaderWidth() const;
    int headerHeight() const;

    int lineNumber() const;
    int trackNumber() const;

    void setEditPosition(int pos);
    void setPrevEditPosition(int pos);
    int editPosition() const;
    int prevEditPosition() const;
    int playPos() const;

    void setPatternStep(int step);
    int patternStep() const;

    void updatePlayBar(bool followSong);

    void setEditOctave(int octave);
    int editOctave() const;
    
    NScrollBar* vScrBar();
    NScrollBar* hScrBar();

    void PlayNote(int note,int velocity,bool bTranspose,psycle::host::Machine*pMachine);
    void StopNote(int note,bool bTranspose=true,psycle::host::Machine* pMachine=NULL);

    void noteOffAny( const PatCursor & cursor );

    void copyBlock(bool cutit);
    void pasteBlock(int tx,int lx,bool mix,bool save = true);
    void blockTranspose(int trp);
    void deleteBlock();
    void doubleLength();
    void halveLength();

    void setMoveCursorWhenPaste( bool on);
    bool moveCursorWhenPaste() const;

    NRect repaintLineNumberArea    (int startLine,int endLine);
    void repaintLineNumber( int startLine, int endLine );

    void setPattern(SinglePattern* pattern);
    SinglePattern* pattern();

    void setBeatZoom(int tpb);
    int beatZoom() const;


    void setActiveMachineIdx(int idx);
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
  NXmlParser xmlParser;
	PatternUndoManager undoManager_;

  int editPosition_, prevEditPosition_;
  int playPos_;
  int editOctave_;
  int selectedMacIdx_;
  bool moveCursorWhenPaste_;

  PatternDraw* drawArea;
	TweakGUI* tweakGUI;
  TweakHeader* tweakHeader;

  NScrollBar* hBar;
  NScrollBar* tweakHBar;
  NCheckBox* sideBox;
  ZoomBar* zoomHBar;
  NScrollBar* vBar;
  Header*     header;
  LineNumber* lineNumber_;
  NColor      separatorColor_;
  NToolBar*   toolBar;
  NComboBox* patternCombo_;
  NComboBox* octaveCombo_;
  NComboBox*  meterCbx;
  NComboBox* trackCombo_;
  NSplitBar* splitBar;
  NPanel* tweakGroup;
	NPanel* lineHeaderLabel;
	NButton* sharpBtn_;

	PatternViewColorInfo colorInfo_;

  void enterNote( const PatCursor & cursor, int note );

  void resize();

  void onHScrollBar( NScrollBar* sender );
  void onHTweakScrollBar( NScrollBar* sender );
  void onVScrollBar( NScrollBar* sender );

  void initToolBar();
  void onAddBar(NButtonEvent* ev);
  void onDeleteBar(NButtonEvent* ev);

  /// multi-key playback state stuff
  int notetrack[MAX_TRACKS];
  int outtrack;

  void onZoomHBarPosChanged(ZoomBar* zoomBar, double newPos);
  void onPatternStepChange(NItemEvent* ev);
  void onOctaveChange(NItemEvent* ev);
  void onTrackChange(NItemEvent* ev);
  void onSideChange( NButtonEvent* ev );
	void onToggleSharpMode(NButtonEvent* ev);
			
	void checkUpScroll( const PatCursor & cursor );
	void checkDownScroll( const PatCursor & cursor );

	

  
};


}}
#endif

