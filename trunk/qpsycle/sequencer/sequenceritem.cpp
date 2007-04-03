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

#include "sequenceritem.h"
#include "sequencerview.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QMenu>

SequencerItem::SequencerItem() 
{
    setFlag( ItemIsMovable );
    setFlag( ItemIsSelectable );
    beatPxLength_ = 5;
}

SequencerItem::~SequencerItem() {
}

QRectF SequencerItem::boundingRect() const
{
    return QRectF( 0, 0, sequenceEntry_->pattern()->beats()*beatPxLength_, parentItem()->boundingRect().height() );
}

void SequencerItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
    if ( isSelected() ) {
        painter->setPen( Qt::blue ); 
    } else {
        painter->setPen( Qt::white ); 
    }
    QColor col = QColorFromLongColor( sequenceEntry_->pattern()->category()->color() ); 
    painter->setBrush( col ); 
    painter->drawRect( boundingRect() ); // FIXME: need to take border width into account.
    painter->setPen( Qt::white ); 
    painter->drawText( boundingRect(), Qt::AlignCenter, QString::fromStdString( sequenceEntry_->pattern()->name() ) );
}

const QColor & SequencerItem::QColorFromLongColor( long longCol )
{
    unsigned int r, g, b;
    b = (longCol>>16) & 0xff;
    g = (longCol>>8 ) & 0xff;
    r = (longCol    ) & 0xff;

    return QColor( r, g, b );
}

void SequencerItem::setSequenceEntry( psy::core::SequenceEntry *sequenceEntry )
{
    sequenceEntry_ = sequenceEntry;
}

psy::core::SequenceEntry *SequencerItem::sequenceEntry() {
    return sequenceEntry_;
}

void SequencerItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    if ( event->modifiers() == Qt::NoModifier ) // Movement constrained to current line.
    {
        QGraphicsItem::mouseMoveEvent( event ); // Do normal move event.

        QList<QGraphicsItem*> selItems = scene()->selectedItems();
        QList<QGraphicsItem*>::iterator i;
        for (i = selItems.begin(); i != selItems.end(); ++i) // For each selected item...
        {
            QGraphicsItem *foo = *i;
            if ( qgraphicsitem_cast<SequencerItem *>( foo ) ) // Make sure it's a SeqItem.
            {
                SequencerItem *item = qgraphicsitem_cast<SequencerItem *>( foo ); 
                item->constrainToParent();
             
                // FIXME: maybe do this on mouseReleaseEvent?
                int newItemLeft = item->pos().x(); 
                item->sequenceEntry()->track()->MoveEntry( item->sequenceEntry(), newItemLeft / beatPxLength_ );
            }
        }
    }
    if ( event->modifiers() == Qt::ControlModifier ) // Movement allowed between lines.
    {
        QGraphicsItem::mouseMoveEvent( event ); // Do normal move event.

        constrainToParent();
     
        SequencerLine *parentLine = qgraphicsitem_cast<SequencerLine*>( parentItem() );
        SequencerLine *lineUnderCursor = 0;
        QList<QGraphicsItem*> itemsUnderCursor = scene()->items( event->scenePos() );
        for ( int i = 0; i < itemsUnderCursor.size(); ++i ) {
            if ( qgraphicsitem_cast<SequencerLine *>( itemsUnderCursor.at(i) ) ) { // Make sure it's a SeqLine.
                lineUnderCursor = qgraphicsitem_cast<SequencerLine*>( itemsUnderCursor.at(i) );
            }
        }
        if ( parentLine != lineUnderCursor && lineUnderCursor != 0 ) {
            setParentItem( lineUnderCursor );
            sequenceEntry()->setSequenceLine( lineUnderCursor->sequenceLine() );
        }

        // FIXME: maybe do this on mouseReleaseEvent?
        int newItemLeft = pos().x(); 
        sequenceEntry()->track()->MoveEntry( sequenceEntry(), newItemLeft / beatPxLength_ );
    }
}

void SequencerItem::constrainToParent() 
{
    // Constrain to parent.
    int widthOfThisItem = boundingRect().width();
    int widthOfParent = parentItem()->boundingRect().width();
    int maximumLeftPos = widthOfParent - widthOfThisItem;
    int currentLeftPos = pos().x();
    int desiredLeftPos = std::min( currentLeftPos, maximumLeftPos );
    int newLeftPos = std::max( 0, desiredLeftPos );

    setPos( newLeftPos, 0 );                 
    
    int newItemLeft = newLeftPos;
    if ( true /*gridSnap()*/ ) {
        int beatPos = pos().x() / beatPxLength_;
        newItemLeft = beatPos * beatPxLength_;
        setPos( newItemLeft, 0 );
    }
}

void SequencerItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
    QGraphicsItem::mousePressEvent( event ); // Do normal business...

    emit clicked(this);
}

void SequencerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
     QMenu menu;
     deleteEntryAction_ = new QAction( "Delete Entry", this );
     connect( deleteEntryAction_, SIGNAL( triggered() ), this, SLOT( onDeleteEntryActionTriggered() ) );
      menu.addAction( deleteEntryAction_ );
      QAction *a = menu.exec(event->screenPos());
}

void SequencerItem::onDeleteEntryActionTriggered()
{
    emit deleteRequest( this );
}
