/***************************************************************************
*   Copyright (C) 2007 by Neil Mather   *
*   nmather@sourceforge   *
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

 #include <QGraphicsScene>
 #include <QGraphicsSceneMouseEvent>
 #include <QPainter>
 #include <QStyleOption>
 #include <QMessageBox>
 #include <QMouseEvent>

 #include "sequencerview.h"

 SequencerItem::SequencerItem()
 {
     setRect(QRectF(0, 0, 150, 30));
     setPen(QPen(Qt::white,1));
     setBrush(QBrush(Qt::red));
     setFlag(ItemIsMovable);
 }

void SequencerItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
    QGraphicsRectItem::paint( painter, option, widget );
    
    painter->drawText( boundingRect(), Qt::AlignCenter, QString::fromStdString( sequenceEntry_->pattern()->name() ) );
}

void SequencerItem::setSequenceEntry( psy::core::SequenceEntry *sequenceEntry )
{
    sequenceEntry_ = sequenceEntry;
}

void SequencerItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
    QGraphicsItem::mouseMoveEvent( event ); // Do normal move event...

    // But then constrain to parent.
    int widthOfThisItem = boundingRect().width();
    int widthOfParent = parentItem()->boundingRect().width();
    int maximumLeftPos = widthOfParent - widthOfThisItem;
    int currentLeftPos = pos().x();
    int desiredLeftPos = std::min( currentLeftPos, maximumLeftPos );
    int newLeftPos = std::max( 0, desiredLeftPos );
    setPos( newLeftPos, 0 );                 
}
