/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
*   psycle.sourceforge.net   *
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

#include "psycore/patternsequence.h"

#include "sequencerline.h"
#include "sequencerview.h"
#include "sequenceritem.h"
#include "sequencerdraw.h"

#include <QGraphicsScene>
#include <boost/bind.hpp>

SequencerLine::SequencerLine( SequencerDraw *sDraw)
{
    sDraw_ = sDraw; // FIXME: don't really want this to be in here..  Feels too tightly-coupled...
    setRect(QRectF(0, 0, sDraw_->width(), 30));
    setPen(QPen(Qt::white,1));
    setBrush(QBrush(Qt::transparent));
}

SequencerLine::~SequencerLine() {
  printf("~SequencerLine %p\n",this);
}

void SequencerLine::setSequenceLine( psy::core::SequenceLine * line )
{
  seqLine_ = line;
  // Iterate the sequence entries and add them.
  psy::core::SequenceLine::iterator iter = seqLine_->begin();
  for(; iter!= seqLine_->end(); ++iter) {
    psy::core::SequenceEntry* entry = iter->second;
    addEntry(entry);
  }
}

psy::core::SequenceLine *SequencerLine::sequenceLine() 
{
    return seqLine_;
}

void SequencerLine::addItem( psy::core::SinglePattern* pattern )
{
  printf("SequencerLine::addItem called\n");
  double endTick = sequenceLine()->tickLength();
  
  SequencerItem *item = new SequencerItem();
  psy::core::SequenceEntry* entry =
    sequenceLine()->createEntry(pattern, endTick);
  addEntry(entry);
}

void SequencerLine::addEntry( psy::core::SequenceEntry* entry)
{
  SequencerItem* item = new SequencerItem();
  item->setSequenceEntry( entry );
  item->setParentItem( this );
  items_.push_back( item );
  connect( item, SIGNAL( deleteRequest( SequencerItem* ) ), 
           sDraw_, SLOT( onSequencerItemDeleteRequest( SequencerItem* ) ) );
  connect( item, SIGNAL( clicked( SequencerItem*) ),
           this, SLOT( onItemClicked( SequencerItem*) ) );
  item->setPos( entry->tickPosition() * sDraw_->beatPxLength(), 0 );

  entry->wasDeleted.connect(boost::bind(&SequencerLine::removeEntry,this,_1));
  assert(scene());
  scene()->addItem( item );
  scene()->update();
}

void SequencerLine::removeEntry(psy::core::SequenceEntry* entry) {
  printf("SequencerLine(this=%p)::removeEntry(%p)",this,entry);
  for(items_iterator i=items_.begin();i!=items_.end();++i) {
    printf("item: %p\n",*i); // gives address 0x0000001 which is clearly wrong!
    assert(*i);
    if((*i)->sequenceEntry() == entry) {
      scene()->removeItem(*i);
      delete *i;
      // need to make line redraw itself?
      items_.erase(i);
      scene()->update();
      return;
    }
  }
}

void SequencerLine::onItemClicked(SequencerItem* item) {
  emit clicked ( this );
}

void SequencerLine::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    emit clicked( this );
}
