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
#include <npanel.h>


/**
@author Stefan Nattkemper
*/

class NScrollBox;
class NLabel;
class NToolBar;
class NListBox;

namespace psycle {
	namespace host {

class SequencerGUI : public NPanel
{
  public:
  class SequencerLine : public NPanel {


    class SequencerItem : public NPanel {
      public:
       SequencerItem( SequencerGUI* seqGui );
       ~SequencerItem();

       virtual void onMousePress(int x, int y, int button);
       virtual void resize();
       virtual void onMove(const NMoveEvent & moveEvent);

       void setText(const std::string & text);

       void setSequenceEntry(SequenceEntry* entry);
       SequenceEntry* sequenceEntry();

      private:

        NLabel* caption_;
        SequenceEntry* sequenceEntry_;

        SequencerGUI* sView;

    };

    public :

     SequencerLine( SequencerGUI* seqGui );
     ~SequencerLine();

    signal1<SequencerLine*> click;

    virtual void paint(NGraphics* g);

    virtual void onMousePress(int x, int y, int button);

    void setSequenceLine(SequenceLine* line);
    SequenceLine* sequenceLine();

    void addItem(SinglePattern* pattern);

   private:

      bool lock;

      SequenceLine* seqLine_;
      SequencerGUI* sView;

      std::vector<SequencerItem*> items;

      void onDeleteEntry(SequenceEntry* entry);

  };


class Area : public NPanel {
  public :
     Area(SequencerGUI* seqGui);
     ~Area();

    void drawTimeGrid(NGraphics* g);

    virtual void paint(NGraphics* g);


  private:

     SequencerGUI* sView;

};


public:
    SequencerGUI();

    ~SequencerGUI();

    void setPatternSequence(PatternSequence* sequence);
    void addPattern(SinglePattern* pattern);

    int beatPxLength() const;

    SequencerLine* selectedLine_;

private:

    int counter;
    int beatPxLength_;

    SequencerLine* lastLine;
    PatternSequence* patternSequence_;

    void addSequencerLine();

    NScrollBox* scrollBox_;
    Area* scrollArea_;
    NToolBar* toolBar_;
    NListBox* patternBox_;

    void onNewTrack(NButtonEvent* ev);
    void onNewPattern(NButtonEvent* ev);
    void onSequencerLineClick(SequencerLine* line);

};

}}

#endif
