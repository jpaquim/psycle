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

#include "sequencerline.h"
#include "sequencerview.h"
#include "sequenceritem.h"

#include <QGraphicsScene>

#include "psycore/patternsequence.h"

SequencerLine::SequencerLine( psy::core::SequenceLine * line )
{
    setSequenceLine( line );
    setRect(QRectF(0, 0, 500, 30));
    setPen(QPen(Qt::white,1));
    setBrush(QBrush(Qt::transparent));
}

void SequencerLine::setSequenceLine( psy::core::SequenceLine * line )
{
    seqLine_ = line;
}

psy::core::SequenceLine *SequencerLine::sequenceLine() 
{
    return seqLine_;
}

void SequencerLine::addItem( psy::core::SinglePattern* pattern )
{
    std::cout << "pat is " << pattern->name() << std::endl;
    qDebug("dude");
    std::cout << "dude0.5" << std::endl;
    double endTick = sequenceLine()->tickLength();

    qDebug("dude1");
    SequencerItem *item = new SequencerItem();
    qDebug("dude2");
    item->setSequenceEntry( sequenceLine()->createEntry(pattern, endTick) );
    qDebug("dude3");
    scene()->addItem( item );
    qDebug("dude4");
    item->setParentItem( this );
    qDebug("dude5");
    item->setPos(5 * endTick, 0);//, static_cast<int>( pattern->beats() * 5 ), 20 );
    qDebug("dude6");
//    item->click.connect(this,&SequencerGUI::SequencerLine::onSequencerItemClick);
    //item->setPos(200, /*sView->beatPxLength() * endTick)*/, 5, static_cast<int>( pattern->beats() * sView->beatPxLength() ) ,20);
}

