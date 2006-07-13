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
#include "sequenceritemgui.h"
#include "sequencergui.h"
#include "patternsequence.h"
#include <ngrs/npanel.h>
#include <ngrs/nlabel.h>
#include <ngrs/nframeborder.h>


namespace psycle {
	namespace host {


// this is the gui class of one pattern entry
SequencerItem::SequencerItem( SequencerGUI* seqGui )
{
  caption_ = new NLabel("Pattern");
    caption_->setVAlign(nAlCenter);
    caption_->setHAlign(nAlCenter);
  add(caption_);

  setBorder (NFrameBorder());
  setMoveable(nMvHorizontal);

  setTransparent(false);

  sequenceEntry_ = 0;

  sView = seqGui;
}

SequencerItem::~ SequencerItem( )
{
}

void SequencerItem::setSequenceEntry( SequenceEntry * sequenceEntry )
{
  sequenceEntry_ = sequenceEntry;

  //if (sequenceEntry_) {
     //caption_->setText( sequenceEntry_->pattern()->name() );
 // }
}

SequenceEntry * SequencerItem::sequenceEntry( )
{
  return sequenceEntry_;
}


void SequencerItem::resize( )
{
  caption_->setPosition(0,0,clientWidth(), clientHeight());
}

void SequencerItem::onMove( const NMoveEvent & moveEvent )
{
  sequenceEntry_->setTickPosition( left() / (double) sView->beatPxLength() );
/*  caption_->setText( sequenceEntry_->pattern()->name() + ":" + stringify(sequenceEntry_->tickPosition()));*/
}

void SequencerItem::onMousePress( int x, int y, int button )
{
}

// PatternSequencerItemGUI

PatternSequencerItem::PatternSequencerItem( SequencerGUI * seqGui ) :
   SequencerItem( seqGui )
{
}

PatternSequencerItem::~ PatternSequencerItem( )
{
}

// GlobalSequencerItemGUI

GlobalSequencerItem::GlobalSequencerItem( SequencerGUI * seqGui ) :
   SequencerItem( seqGui )
{
}

GlobalSequencerItem::~ GlobalSequencerItem( )
{
}

// GlobalSequencerBpmItemGUI

GlobalBpmSequencerItem::GlobalBpmSequencerItem( SequencerGUI * seqGui ) :
  GlobalSequencerItem( seqGui )
{
}

GlobalBpmSequencerItem::~ GlobalBpmSequencerItem( )
{
}


}}

