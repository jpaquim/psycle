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

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <iostream>
#include <vector>

#include "sequencerview.h"
#include "sequencerdraw.h"
#include "sequencerline.h"
#include "sequenceritem.h"
#include "sequencerarea.h"
#include "beatruler.h"

#include "psycore/player.h"


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

    bool isFirst = true;
    int count = 0;
    std::vector<psy::core::SequenceLine*>::iterator it = sequencerView()->song()->patternSequence()->begin();
    for ( ; it < sequencerView()->song()->patternSequence()->end(); it++) {
        psy::core::SequenceLine* seqLine = *it;
        SequencerLine* line = new SequencerLine( this, seqLine );
        line->setParentItem( seqArea_ );
        lines_.push_back(line);
        line->setPos( 0, count*lineHeight_ );
        if (isFirst) {
            setSelectedLine( line ); 
            isFirst = false;
        }
        connect( line, SIGNAL( clicked( SequencerLine* ) ), this, SLOT( onSequencerLineClick( SequencerLine* ) ) );
//        lastLine_ = line;

        // Now iterate the sequence entries.
        psy::core::SequenceLine::iterator iter = seqLine->begin();
        for(; iter!= seqLine->end(); ++iter)
        {
            psy::core::SequenceEntry* entry = iter->second;
            SequencerItem* item = new SequencerItem();
            item->setParentItem( line );
            connect( item, SIGNAL( deleteRequest( SequencerItem* ) ), 
                     this, SLOT( onSequencerItemDeleteRequest( SequencerItem* ) ) );
            item->setSequenceEntry( entry );
			item->setPos( entry->tickPosition() * beatPxLength_, 0 );
        }
        count++;
    }
    pLine_ = new PlayLine();
//    pLine_->setCursor( Qt::SizeHorCursor );
    pLine_->setRect( 0, 0, 1, height() );
    connect( pLine_, SIGNAL( playLineMoved( double ) ),
             this, SLOT( onPlayLineMoved( double ) ) );
    scene_->addItem( pLine_ );

    BeatRuler *beatRuler = new BeatRuler( this );

    scene_->addItem( beatRuler );
    beatRuler->setPos( 0, 0 );
    scene_->addItem( seqArea_ );
    seqArea_->setPos( 0, 30 );


}

int SequencerDraw::beatPxLength( ) const
{
    return beatPxLength_;
}

void SequencerDraw::setSelectedLine( SequencerLine *line ) 
{
    selectedLine_ = line;
}

SequencerLine* SequencerDraw::selectedLine() 
{
    return selectedLine_;
}

void SequencerDraw::addPattern( psy::core::SinglePattern *pattern )
{
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
    } else
        if ( selectedLine() ) {
            psy::core::SequenceLine *seqLine = seqView_->song()->patternSequence()->insertNewLine( selectedLine()->sequenceLine() ); 
            SequencerLine* line = new SequencerLine( this, seqLine );
            lines_.push_back( line );

            scene()->addItem( line );
            line->setParentItem( seqArea_ );
            line->setPos( 0, (lines_.size()-1) * lineHeight_ );
            //line->itemClick.connect(this, &SequencerGUI::onSequencerItemClick);
            connect( line, SIGNAL( clicked( SequencerLine* ) ), this, SLOT( onSequencerLineClick( SequencerLine* ) ) );
            setSelectedLine( line );
/*            int index = selectedLine_->zOrder();
            scrollArea_->insert(line, index);
            scrollArea_->resize();
            lastLine = line;*/
//            resize();
 //           scrollArea_->repaint();
        }
}

void SequencerDraw::onSequencerLineClick( SequencerLine *line )
{
    setSelectedLine( line );
}

void SequencerDraw::onSequencerItemDeleteRequest( SequencerItem *item )
{
    psy::core::SequenceEntry *entry = item->sequenceEntry();
    entry->track()->removeEntry(entry); // Remove from the song's pattern sequence.
    scene()->removeItem( item ); // Remove from the GUI. FIXME: think we need to delete the object itself here too.
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
    QGraphicsItem::mouseMoveEvent( event );
    setPos( pos().x(), 0 );
}

void PlayLine::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
    QGraphicsItem::mouseReleaseEvent( event );
    emit playLineMoved( pos().x() );
}
