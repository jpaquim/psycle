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
#ifndef SEQUENCERITEMGUI_H
#define SEQUENCERITEMGUI_H

#include <ngrs/npanel.h>

/**
@author Stefan Nattkemper
*/

class NLabel;

namespace psycle {
	namespace host {


class SequencerGUI;
class SequenceEntry;

class SequencerItem : public NPanel {
  public:
    SequencerItem( SequencerGUI* seqGui );
    ~SequencerItem();

    virtual void onMousePress(int x, int y, int button);
    virtual void resize();
    virtual void onMove(const NMoveEvent & moveEvent);

    void setText(const std::string & text);

    virtual void setSequenceEntry(SequenceEntry* entry);
    SequenceEntry* sequenceEntry();

  private:

    NLabel* caption_;
    SequenceEntry* sequenceEntry_;

    SequencerGUI* sView;

};

class PatternSequencerItem : public SequencerItem {
  public:
    PatternSequencerItem( SequencerGUI* seqGui );
    ~PatternSequencerItem();
};

class GlobalSequencerItem : public SequencerItem {
  public:
     GlobalSequencerItem( SequencerGUI* seqGui );
     ~GlobalSequencerItem();
    };

class GlobalBpmSequencerItem : public GlobalSequencerItem {
public :
  GlobalBpmSequencerItem( SequencerGUI* seqGui );
  ~GlobalBpmSequencerItem();
};


}}
#endif
