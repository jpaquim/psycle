/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
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

#include "psycore/song.h"
#include "psycore/player.h"

#include "sequencerview.h"
#include "sequencerdraw.h"
#include "sequencerline.h"
#include "sequenceritem.h"
#include "sequencerarea.h"
#include "beatruler.h"

#include <iostream>
#include <vector>
#include <boost/bind.hpp>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QGraphicsItem>

 SequencerDraw::SequencerDraw( SequencerView *seqView )
 {
    seqView_ = seqView;
    beatPxLength_ = 5;
    lineHeight_ = 30;

    QGraphicsScene *scene_ = new QGraphicsScene(this);
    scene_->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene_->setBackgroundBrush(Qt::black);

    setAlignment ( Qt::AlignLeft | Qt::AlignTop );
    setScene(scene_);
    setSceneRect( 0,0, width(), height() );

    seqArea_ = new SequencerArea( this );

    psy::core::PatternSequence* patternSequence = sequencerView()->song()->patternSequence();

    std::vector<psy::core::SequenceLine*>::iterator it = patternSequence->begin();
    for ( ; it < patternSequence->end(); it++) {
        psy::core::SequenceLine* seqLine = *it;
        onNewLineCreated(seqLine);
    }
    
    if(!lines_.empty()) {
      setSelectedLine(lines_[0]);
    }

    pLine_ = new PlayLine();
    pLine_->setRect( 0, 0, 1, height() );
    connect( pLine_, SIGNAL( playLineMoved( double ) ),
             this, SLOT( onPlayLineMoved( double ) ) );
    scene_->addItem( pLine_ );

    BeatRuler *beatRuler = new BeatRuler( this );

    scene_->addItem( beatRuler );
    beatRuler->setPos( 0, 0 );
    scene_->addItem( seqArea_ );
    seqArea_->setPos( 0, 30 );

    patternSequence->newLineCreated.connect
      (boost::bind(&SequencerDraw::onNewLineCreated,this,_1));
    patternSequence->newLineInserted.connect
      (boost::bind(&SequencerDraw::onNewLineInserted,this,_1,_2));
    /*
      Does not work because of a boost bug in boost 1.33
      as of Mar 28 2007.
      This bug is fixed in boost SVN. I put a workaround in
      makeSequencerLine instead.
      / Magnus

    patternSequence->lineRemoved.connect
      (boost::bind(&SequencerDraw::onLineRemoved,this,_1));
    */

    patternSequence->linesSwapped.connect
      (boost::bind(&SequencerDraw::onLinesSwapped,this,_1,_2));
}

int SequencerDraw::beatPxLength( ) const
{
    return beatPxLength_;
}

void SequencerDraw::setSelectedLine( SequencerLine *line ) 
{
  printf("setSelectedLine %p\n", line);
    selectedLine_ = line;
}

SequencerLine* SequencerDraw::selectedLine() 
{
    return selectedLine_;
}

void SequencerDraw::addPattern( psy::core::SinglePattern *pattern )
{
  printf("SequencerDraw::addPattern called");
    if ( selectedLine() ) {
        selectedLine()->addItem( pattern );
    }
}

std::vector<SequencerLine*> SequencerDraw::lines()
{
    return lines_;
}

void SequencerDraw::insertTrack()
{
    if ( lines_.size() == 0 ) {
    //    addSequencerLine();
//        resize();
//        repaint();
    } else if ( selectedLine() ) {
          // will cause onNewLineInserted to be fired:
          seqView_->song()->patternSequence()->insertNewLine( selectedLine()->sequenceLine() );
/*            int index = selectedLine_->zOrder();
            scrollArea_->insert(line, index);
            scrollArea_->resize();
*/
    }
}

void SequencerDraw::deleteTrack() {
  if(selectedLine()) {
    // will trigger onLineRemoved
    seqView_->song()->patternSequence()->removeLine(selectedLine()->sequenceLine());
  }
}

void SequencerDraw::moveDownTrack() {
  if(selectedLine()) {
    // will trigger onLinesSwapped
    seqView_->song()->patternSequence()->moveDownLine(selectedLine()->sequenceLine());
  }
}
void SequencerDraw::moveUpTrack() {
  if(selectedLine()) {
    // will trigger onLinesSwapped
    seqView_->song()->patternSequence()->moveUpLine(selectedLine()->sequenceLine());
  }
}

SequencerLine* SequencerDraw::makeSequencerLine(psy::core::SequenceLine* seqLine) {
  SequencerLine* line = new SequencerLine( this);
  scene()->addItem(line);
  line->setSequenceLine(seqLine);
  line->setParentItem( seqArea_ );
  connect( line, SIGNAL( clicked( SequencerLine* ) ), this, SLOT( onSequencerLineClick( SequencerLine* ) ) );
  seqLine->wasDeleted.connect(boost::bind(&SequencerDraw::onLineRemoved,this,_1));
  return line;
}

void SequencerDraw::onSequencerLineClick( SequencerLine *line )
{
    QRectF oldLineRect = selectedLine()->mapToScene( selectedLine()->boundingRect() ).boundingRect();
    QRectF newLineRect = line->mapToScene( line->boundingRect() ).boundingRect();
    setSelectedLine( line );
    scene()->update( oldLineRect );
    scene()->update( newLineRect );
}

void SequencerDraw::onSequencerItemDeleteRequest( SequencerItem *item )
{
    psy::core::SequenceEntry *entry = item->sequenceEntry();
    if ( psy::core::Player::Instance()->loopSequenceEntry() == entry ) {
        psy::core::Player::Instance()->setLoopSequenceEntry( 0 );
    }
    entry->track()->removeEntry(entry); // Remove from the song's pattern sequence.
    scene()->removeItem( item ); // Remove from the GUI. FIXME: think we need to delete the object itself here too.
}

void SequencerDraw::onNewLineCreated(psy::core::SequenceLine* seqLine)
{
  SequencerLine* line = makeSequencerLine(seqLine);
  line->setPos( 0, lines_.size()*lineHeight_ );
  lines_.push_back(line);
  setSelectedLine( line );
}

void SequencerDraw::onNewLineInserted(psy::core::SequenceLine* seqLine, psy::core::SequenceLine* position)
{
  printf("onNewLineInserted(%p,%p)\n",seqLine,position);
  SequencerLine* line = makeSequencerLine(seqLine);
  int numLines = lines_.size();
  for(int i=0;i<numLines;i++) {
    if (lines_[i]->sequenceLine() == position) {
      printf("%ith line\n",i);
      SequencerLine* l=line;
      for(int j=i;j<numLines;j++) {
        l->setPos( 0, j*lineHeight_ );
        std::swap(l,lines_[j]);
      }
      l->setPos( 0, numLines*lineHeight_ );
      lines_.push_back(l);
      break;
    }
  }

  setSelectedLine( line );
}

void SequencerDraw::onLineRemoved(psy::core::SequenceLine* seqLine)
{
  if (selectedLine() && selectedLine()->sequenceLine() == seqLine)
    setSelectedLine( NULL );
  int numLines = lines_.size();
  for(int i=0;i<numLines;i++) {
    if (lines_[i]->sequenceLine() == seqLine) {
      for(int j=i;j<numLines-1;j++) {
        std::swap(lines_[j],lines_[j+1]);
        lines_[j]->setPos( 0, j*lineHeight_ );
      }
      delete lines_.back();
      lines_.pop_back();
      break;
    }
  }
}

void SequencerDraw::onLinesSwapped(psy::core::SequenceLine* a,
                                   psy::core::SequenceLine* b)
{
  lines_iterator ita = lines_.end();
  lines_iterator itb = lines_.end();
  for(lines_iterator i=lines_.begin();i != lines_.end();i++) {
    psy::core::SequenceLine* sl = (*i)->sequenceLine();
    if (sl == a)
      ita = i;
    if (sl == b)
      itb = i;
  }
  if (ita != lines_.end() && itb != lines_.end()) {
    QPointF posa = (*ita)->pos();
    QPointF posb = (*itb)->pos();
    (*ita)->setPos(posb);
    (*itb)->setPos(posa);
    std::swap(*ita,*itb);
  }
}

void SequencerDraw::onPlayLineMoved( double newXPos )
{
    psy::core::Player::Instance()->stop();
    psy::core::Player::Instance()->setPlayPos( newXPos / beatPxLength_ );
}

PlayLine::PlayLine()
{ 
    setFlags( ItemIsMovable | ItemIsFocusable );
    setCursor( Qt::SizeHorCursor );
    setPen( QColor( Qt::red ) );
    setBrush( QColor( Qt::red ) );
    setZValue( 100 );
}

void PlayLine::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->buttons() & Qt::LeftButton )
    {
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        setPos( std::max( 0, (int)newPos.x() ), 0 ); // x can't be less than 0; y is always 0.
    } else {
        event->ignore();
    }
}

void PlayLine::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    QGraphicsItem::mouseReleaseEvent( event );
    emit playLineMoved( pos().x() );
}
