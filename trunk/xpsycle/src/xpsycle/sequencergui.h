/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include <ngrs/npanel.h>
#include <list>

/**
@author Stefan Nattkemper
*/

class NScrollBar;
class NLabel;
class NToolBar;
class NListBox;
class ZoomBar;

namespace psycle {
	namespace host {

class SequencerBeatChangeLineal;

class SequencerGUI;


class SequencerItem : public NPanel {
      public:
       SequencerItem( SequencerGUI* seqGui );
       ~SequencerItem();

       signal1<SequencerItem*> click;

       virtual void paint( NGraphics* g);
       virtual void onMousePress(int x, int y, int button);
       virtual void resize();
       virtual void onMove(const NMoveEvent & moveEvent);
       virtual void onMoveEnd(const NMoveEvent & moveEvent);

       void setText(const std::string & text);

       void setSequenceEntry(SequenceEntry* entry);
       SequenceEntry* sequenceEntry();

       void setSelected(bool on);
       bool selected();

      private:

        bool selected_;

        SequenceEntry* sequenceEntry_;
        SequencerGUI* sView;

};

class SequencerGUI : public NPanel
{
  public:
  class SequencerLine : public NPanel {
    public :

     SequencerLine( SequencerGUI* seqGui );
     ~SequencerLine();

    signal1<SequencerLine*> click;
    signal1<SequencerItem*> itemClick;

    virtual void paint(NGraphics* g);
    virtual void onMousePress(int x, int y, int button);
    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual void removeChild(NVisualComponent* item);

    void setSequenceLine(SequenceLine* line);
    SequenceLine* sequenceLine();

    void addItem(SinglePattern* pattern);
    void removeItems(SinglePattern* pattern);

    std::vector<SequencerItem*> itemsByPattern(SinglePattern* pattern);

     virtual void resize();

    std::list<SequencerItem*> items;

    void onSequencerItemClick(SequencerItem* item);

   private:

      SequenceLine* seqLine_;
      SequencerGUI* sView;

  };


class SequencerBeatLineal : public NPanel {
  public :
     SequencerBeatLineal(SequencerGUI* seqGui);
     ~SequencerBeatLineal();

     virtual void paint(NGraphics* g);
     virtual int preferredHeight() const;

  private:

     SequencerGUI* sView;

     void drawLineal(NGraphics* g, int dx);

};


class Area : public NPanel {
  public :
     Area(SequencerGUI* seqGui);
     ~Area();

    void drawTimeGrid(NGraphics* g);

    virtual void paint(NGraphics* g);
    virtual int preferredWidth() const;
    virtual int preferredHeight() const;
    virtual void resize();

  private:

     SequencerGUI* sView;

};


public:
    SequencerGUI();

    ~SequencerGUI();

    void setPatternSequence(PatternSequence* sequence);
    PatternSequence* patternSequence();

    void addPattern(SinglePattern* pattern);
    void removePattern(SinglePattern* pattern);

    std::vector<SequencerItem*> guiItemsByPattern(SinglePattern* pattern);

    int beatPxLength() const;

    SequencerLine* selectedLine_;

    void update();

    virtual void resize();

    void addSequencerLine();

private:

    int counter;
    int beatPxLength_;

    SequencerBeatLineal* beatLineal_;
    SequencerBeatChangeLineal* beatChangeLineal_;
    SequencerLine* lastLine;
    SequencerItem* selectedItem_;
    PatternSequence* patternSequence_;

    NScrollBar* hBar;
    NScrollBar* vBar;

    ZoomBar* zoomHBar;

    Area* scrollArea_;
    NToolBar* toolBar_;
    NListBox* patternBox_;

    void onNewTrack(NButtonEvent* ev);
    void onInsertTrack(NButtonEvent* ev);
    void onDeleteTrack(NButtonEvent* ev);

    void onNewPattern(NButtonEvent* ev);
    void onDeleteEntry(NButtonEvent* ev);

    void onSequencerLineClick(SequencerLine* line);
    void onSequencerItemClick(SequencerItem* item);
    void onZoomHBarPosChanged(ZoomBar* zoomBar, double newPos);
    void onHScrollBar(NObject* sender, int pos);
    void onVScrollBar(NObject* sender, int pos);

    std::vector<SequencerLine*> lines;
};

}}

#endif
