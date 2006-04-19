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
#include <npage.h>
#include <nbitmap.h>
#include <npoint3d.h>
#include <npopupmenu.h>
#include <nscrollbar.h>
#include <map>

/**
@author Stefan
*/

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

     NBitmap bitmap;

     virtual int preferredWidth();
     int skinColWidth();

   private:

     int skinColWidth_;
     PatternView* pView;
   };




   class LineNumber : public NPanel {
   public:
     LineNumber(PatternView* pPatternView);
     ~LineNumber();

     virtual void paint(NGraphics* g);

     void setDy(int dy);
     int dy();

   private:
      PatternView* pView;
      int dy_;
   };

   class PatternDraw : public NPanel {
   public:
     PatternDraw(PatternView* pPatternView);
     ~PatternDraw();


     virtual void paint(NGraphics* g);

     void setDy(int dy);
     void setDx(int dx);

     int dy();
     int dx();

     virtual void onMousePress(int x, int y, int button);
     virtual void onMousePressed(int x, int y, int button);
     virtual void onMouseOver(int x, int y);
     virtual void onKeyPress(const NKeyEvent & event);
     virtual void onKeyRelease(const NKeyEvent & event);

     NRect repaintTrackArea    (int startLine,int endLine,int startTrack, int endTrack);
     NPoint linesFromRepaint(const NRegion & repaintArea);
     NPoint tracksFromRepaint(const NRegion & repaintArea);


   private:

    NPopupMenu* editPopup_;
    PatDlg* patDlg;
    PatternView* pView;
    int dx_, dy_;

    bool doDrag_;
    bool doSelect_;
    bool doShiftSel_;

    NSize selection_;
    NSize oldSelection_; // we cut motionButton Events, so not every mousemotion is recognized
    NPoint3D selStartPoint_;
    NPoint3D selCursor; // for keyboard drag

    void drawText(NGraphics* g, int track, int line, int eventOffset, const std::string & text);
    void drawData(NGraphics* g, int track, int line, int eventOffset, const std::string & text);

    void drawCellBg(NGraphics* g, int track, int line, int col, const NColor & bgColor);

    void drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);
    void drawSelBg(NGraphics* g, const NSize & selArea);


    std::map<char,int> keyMap_;
    void initKeyMap();
    int charToNote(char c);

    NPoint3D intersectCell(int x, int y);
    void clearOldSelection();

    bool isBlockCopied;
    int blockNTracks;
    int blockNLines;
    NSize blockLastOrigin;

    void startSel(const NPoint3D & p);
    void doSel(const NPoint3D & p);
    void endSel();
 
    void onPopupBlockCopy(NButtonEvent* ev);
    void onPopupBlockCut(NButtonEvent* ev);
    void onPopupBlockPaste(NButtonEvent* ev);
    void onPopupBlockDelete(NButtonEvent* ev);
    void onPopupTranspose1(NButtonEvent* ev);
    void onPopupTranspose12(NButtonEvent* ev);
    void onPopupTranspose_1(NButtonEvent* ev);
    void onPopupTranspose_12(NButtonEvent* ev);
    void onPopupPattern(NButtonEvent* ev);


    void copyBlock(bool cutit);
    void pasteBlock(int tx,int lx,bool mix,bool save = true);
    void blockTranspose(int trp);

    unsigned char blockBufferData[EVENT_SIZE*MAX_LINES*MAX_TRACKS];
  };

public:
    PatternView();

    ~PatternView();

    void setSeparatorColor(const NColor & separatorColor);
    const NColor & separatorColor();

    int rowHeight();
    int colWidth();
    int headerHeight();
    int noteCellWidth();
    int cellWidth();
    int cellCount();

    int lineNumber();
    int trackNumber();

    void addEvent( int byteLength );
    int colStartFromEvent(int event);
    int eventFromCol(int col);
    int eventLength(int event);
    std::vector<int> eventSize;

    void setEditPosition(int pos);
    void setPrevEditPosition(int pos);
    int editPosition();
    int prevEditPosition();
    int playPos();

    void updatePlayBar(bool followSong);

    void setEditOctave(int octave);
    int editOctave();

    const NPoint3D & cursor() const;
    void setCursor(const NPoint3D & cursor);
    void moveCursor(int dx, int dy, int dz);

    NScrollBar* vScrBar();
    NScrollBar* hScrBar();

    void PlayNote(int note,int velocity,bool bTranspose,Machine*pMachine);
    void noteOffAny();
    void clearCursorPos();

private:

  int editPosition_, prevEditPosition_;
  int playPos_;
  int editOctave_;

  NPoint3D cursor_;
  PatternDraw* drawArea;

  NScrollBar* hBar;
  NScrollBar* vBar;
  Header*     header;
  LineNumber* lineNumber_;
  NColor      separatorColor_;

  std::string noteToString(int value);

  void resize();

  void onHScrollBar(NObject* sender, int pos);
  void onVScrollBar(NObject* sender, int pos);

  /// multi-key playback state stuff
  int notetrack[MAX_TRACKS];
  int outtrack;
};

#endif
