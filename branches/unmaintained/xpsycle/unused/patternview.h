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
#ifndef PATTERNVIEW_H
#define PATTERNVIEW_H

#include "patdlg.h"
#include "song.h"
#include "singlepattern.h"
#include "custompatternview.h"
#include "skinreader.h"
#include <ngrs/popupmenu.h>
#include <ngrs/scrollbar.h>
#include <ngrs/xmlparser.h>
#include <map>

/**
@author  Stefan
*/

namespace ngrs {
	class ToolBar;
	class ComboBox;
	class NCheckBox;
	class SplitBar;
	class ItemEvent;
}

class ZoomBar;

namespace psycle { 
	namespace host {


		class Song;

		class UndoPattern : public SinglePattern {
		public  :

			UndoPattern();
			UndoPattern( int patternId, const ngrs::Size & changedBlock, const PatCursor & cursor  );

			~UndoPattern();

			const ngrs::Size & changedBlock() const;
			const PatCursor & oldCursor();

			int patternId();

		private :

			ngrs::Size changedBlock_;
			int patternId_;
			PatCursor cursor_;

		};

		class PatternUndoManager : public std::vector<UndoPattern> {
		public :

			PatternUndoManager( );
			~PatternUndoManager();

			void setSong( Song* pSong  );
			void setPattern( SinglePattern* pattern );
			void addUndo( const ngrs::Size & block, const PatCursor & cursor );
			void addUndo( const PatCursor & cursor );

			void doUndo();

		private:

			SinglePattern* pattern_;
			Song* pSong_;

		};


		class PatternView : public ngrs::Panel
		{
		public:

			class Header: public ngrs::Panel {
			public:
				Header(PatternView* pPatternView);
				~Header();

				virtual void paint( ngrs::Graphics& g );

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


			class LineNumber : public ngrs::Panel {
			public:
				LineNumber( PatternView* pPatternView );
				~LineNumber();

				virtual void paint( ngrs::Graphics& g );

				void setDy( int dy );
				int dy() const;

				void setTextColor( const ngrs::Color& textColor );
				const ngrs::Color & textColor() const;

				virtual int preferredWidth() const;

			private:

				PatternView* pView;
				int dy_;
				ngrs::Color textColor_;

			};



			class PatternDraw : public CustomPatternView {
			public:

				PatternDraw( PatternView* pPatternView );
				~PatternDraw();

				virtual int colWidth() const;
				virtual int rowHeight() const;
				virtual int lineNumber() const;
				virtual int beatZoom() const;

				virtual void customPaint( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack );

				virtual void onMousePress( int x, int y, int button );
				virtual void onMousePressed( int x, int y, int button );
				virtual void onMouseOver( int x, int y );
				virtual void onKeyPress( const ngrs::KeyEvent & event );
				virtual void onKeyRelease( const ngrs::KeyEvent & event );

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
				virtual void drawPattern( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack );


			private:

				ngrs::NPopupMenu* editPopup_;
				PatDlg* patDlg;
				PatternView* pView;      

				bool isBlockCopied;
				ngrs::Size blockLastOrigin;
				bool sharpMode_;

				void clearCursorPos();

				void onPopupBlockCopy( ngrs::ButtonEvent* ev );
				void onPopupBlockCut( ngrs::ButtonEvent* ev );
				void onPopupBlockPaste( ngrs::ButtonEvent* ev );
				void onPopupBlockDelete( ngrs::ButtonEvent* ev );
				void onPopupBlockMixPaste( ngrs::ButtonEvent* ev );
				void onPopupTranspose1( ngrs::ButtonEvent* ev );
				void onPopupTranspose12( ngrs::ButtonEvent* ev );
				void onPopupTranspose_1( ngrs::ButtonEvent* ev );
				void onPopupTranspose_12( ngrs::ButtonEvent* ev );
				void onPopupPattern( ngrs::ButtonEvent* ev );

				SinglePattern pasteBuffer;
				void onTagParse( const ngrs::XmlParser & parser, const std::string & tagName );
				float lastXmlLineBeatPos;
				int xmlTracks;
				float xmlBeats;

				void checkLeftScroll( const PatCursor & cursor );
				void checkRightScroll( const PatCursor & cursor );

			};

			class TweakHeader : public ngrs::Panel {
			public:

				TweakHeader(  PatternView* pPatternView );

				~TweakHeader();

				virtual int preferredWidth();
				virtual void paint( ngrs::Graphics& g );

				int skinColWidth() const;

			private:

				ngrs::Rect bgCoords;
				ngrs::Rect noCoords;

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

				virtual void customPaint( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack );

				virtual void onKeyPress( const ngrs::KeyEvent & event );

				virtual void resize();

			protected:

				virtual int doSel( const PatCursor & p );
				void drawPattern( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack );

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

			sigslot::signal1<int> lineChanged;

			void setSeparatorColor( const ngrs::Color & separatorColor );
			const ngrs::Color & separatorColor();


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

			ngrs::ScrollBar* vScrBar();
			ngrs::ScrollBar* hScrBar();

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

			ngrs::Rect repaintLineNumberArea( int startLine, int endLine );
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
			ngrs::XmlParser xmlParser;
			PatternUndoManager undoManager_;

			int editPosition_, prevEditPosition_;
			int playPos_;
			int editOctave_;
			int selectedMacIdx_;
			bool moveCursorWhenPaste_;

			PatternDraw* drawArea;
			TweakGUI* tweakGUI;
			TweakHeader* tweakHeader;

			ngrs::ScrollBar* hBar;
			ngrs::ScrollBar* tweakHBar;
			ngrs::NCheckBox* sideBox;
			ZoomBar* zoomHBar;
			ngrs::ScrollBar* vBar;
			Header*     header;
			LineNumber* lineNumber_;
			ngrs::Color      separatorColor_;
			ngrs::ToolBar*   toolBar;
			ngrs::ComboBox* patternCombo_;
			ngrs::ComboBox* octaveCombo_;
			ngrs::ComboBox*  meterCbx;
			ngrs::ComboBox* trackCombo_;
			ngrs::SplitBar* splitBar;
			ngrs::Panel* tweakGroup;
			ngrs::Panel* lineHeaderLabel;
			ngrs::Button* sharpBtn_;

			PatternViewColorInfo colorInfo_;

			void enterNote( const PatCursor & cursor, int note );

			void resize();

			void onHScrollBar( ngrs::ScrollBar* sender );
			void onHTweakScrollBar( ngrs::ScrollBar* sender );
			void onVScrollBar( ngrs::ScrollBar* sender );

			void initToolBar();
			void onAddBar( ngrs::ButtonEvent* ev );
			void onDeleteBar( ngrs::ButtonEvent* ev );

			/// multi-key playback state stuff
			int notetrack[MAX_TRACKS];
			int outtrack;

			void onZoomHBarPosChanged( ZoomBar* zoomBar, double newPos );
			void onPatternStepChange( ngrs::ItemEvent* ev );
			void onOctaveChange( ngrs::ItemEvent* ev );
			void onTrackChange( ngrs::ItemEvent* ev );
			void onSideChange( ngrs::ButtonEvent* ev );
			void onToggleSharpMode( ngrs::ButtonEvent* ev );

			void checkUpScroll( const PatCursor & cursor );
			void checkDownScroll( const PatCursor & cursor );

		};

	}
}
#endif
