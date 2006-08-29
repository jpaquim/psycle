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
class ZoomBar;
class NItemEvent;

namespace psycle { namespace host {


class Song;

class PatternView : public NPanel
{
    class Header: public NPanel {
    public:
      Header(PatternView* pPatternView);
      ~Header();

      virtual void paint(NGraphics* g);

      void setSkin();

      NRect bgCoords;
      NRect noCoords;
      NRect sRecCoords;
      NPoint dRecCoords;
      NRect sMuteCoords;
      NPoint dMuteCoords;
      NRect sSoloCoords;
      NPoint dSoloCoords;
      NPoint dgX0Coords;
      NPoint dg0XCoords;

      virtual void onMousePress(int x, int y, int button);
      virtual int preferredWidth();

      int skinColWidth();

    private:

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

    private:
      PatternView* pView;
      int dy_;
    };


		class TweakHeader : public NPanel {
			public:

				TweakHeader();

				~TweakHeader();

		};


    class PatternDraw : public CustomPatternView {
    public:

      PatternDraw(PatternView* pPatternView);
      ~PatternDraw();

			virtual int colWidth() const;
			virtual int rowHeight() const;
			virtual int lineNumber() const;
			virtual int trackNumber() const;
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

		protected:

				virtual NPoint3D intersectCell( int x, int y );
				virtual void drawColumnGrid( NGraphics* g, int startLine, int endLine, int startTrack, int endTrack );
				virtual void drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);

    private:

      NPopupMenu* editPopup_;
      PatDlg* patDlg;
      PatternView* pView;

			NPoint3D selCursor; // for keyboard drag

      void drawText(NGraphics* g, int track, int line, int eventOffset, const std::string & text);
      void drawData(NGraphics* g, int track, int line, int eventOffset, const std::string & text);

      void drawCellBg(NGraphics* g, int track, int line, int col, const NColor & bgColor);
        

      bool isBlockCopied;
      NSize blockLastOrigin;


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

    };


    class TweakGUI : public NPanel {
		public:
				TweakGUI( PatternView* pPatternView);

				~TweakGUI();			
		};

public:
    PatternView( Song * song);

    ~PatternView();

    Song* pSong();

    signal1<int> lineChanged;

    void setSeparatorColor(const NColor & separatorColor);
    const NColor & separatorColor();


    int rowHeight() const;
    int colWidth() const;
    int headerHeight() const;
    int noteCellWidth() const;
    int cellWidth() const;
    int cellCount() const;

    int lineNumber() const;
    int trackNumber() const;

    void addEvent( int byteLength );
    int colStartFromEvent(int event);
    int eventFromCol(int col) const;
    int eventLength(int event) const;
    std::vector<int> eventSize;

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

    const NPoint3D & cursor() const;
    void setCursor(const NPoint3D & cursor);
    void moveCursor(int dx, int dy, int dz);

    NScrollBar* vScrBar();
    NScrollBar* hScrBar();

    void PlayNote(int note,int velocity,bool bTranspose,psycle::host::Machine*pMachine);
    void StopNote(int note,bool bTranspose=true,psycle::host::Machine* pMachine=NULL);

    void noteOffAny();
    void clearCursorPos();

    void copyBlock(bool cutit);
    void pasteBlock(int tx,int lx,bool mix,bool save = true);
    void blockTranspose(int trp);
    void deleteBlock();
    void doubleLength();
    void halveLength();

    void setMoveCursorWhenPaste( bool on);
    bool moveCursorWhenPaste() const;

    NRect repaintLineNumberArea    (int startLine,int endLine);

    void setPattern(SinglePattern* pattern);
    SinglePattern* pattern();

    void setBeatZoom(int tpb);
    int beatZoom() const;


    void setActiveMachineIdx(int idx);
    int selectedMachineIndex() const;

    void clearCurr();

private:

  Song* _pSong;
  SinglePattern* pattern_;
  NXmlParser xmlParser;

  int editPosition_, prevEditPosition_;
  int playPos_;
  int editOctave_;
  int patternStep_;
  int selectedMacIdx_;
  bool moveCursorWhenPaste_;

  NPoint3D cursor_;

  PatternDraw* drawArea;
	TweakGUI* tweakGUI;

  NScrollBar* hBar;
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

  std::string noteToString(int value);

  void enterNote( int note );

  void resize();

  void onHScrollBar(NObject* sender, int pos);
  void onVScrollBar(NObject* sender, int pos);

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

  
};


}}
#endif

