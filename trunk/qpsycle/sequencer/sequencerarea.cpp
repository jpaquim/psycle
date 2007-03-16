/***************************************************************************
*   Copyright (C) 2006 by  Neil Mather   *
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

 #include "sequencerarea.h"
 #include "sequencerview.h"
 #include "sequencerline.h"
 #include "sequenceritem.h"

 #include "psycore/patternsequence.h"

 #include <vector>

 SequencerArea::SequencerArea( SequencerDraw *seqDrawIn )
    : seqDraw_( seqDrawIn )
 {
    int width = seqDraw_->sequencerView()->width();
    int height = seqDraw_->sequencerView()->height();
     setRect( 0, 0, width, height );
     setBrush( QBrush( Qt::transparent ) );

    beatPxLength_ = sequencerDraw()->beatPxLength();
 }


void SequencerArea::paint( QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
    drawTimegrid( painter );
}

void SequencerArea::drawTimegrid( QPainter *painter )
{
    QRectF br = boundingRect();
    int start = ( br.left() /*- absoluteLeft() + scrollDx()*/) / beatPxLength_;
    int end   = ( br.left() + boundingRect().width() /*- absoluteLeft() + scrollDx()*/ ) / beatPxLength_;

    painter->setPen( QColor( 30, 30, 30 ) );
    for (int i = start ; i <= end ; i++) {
        if ( beatPxLength_ > 3 || (beatPxLength_ <= 3 && (!( i %16)))  ) {
            painter->drawLine( i*beatPxLength_, 0/*-scrollDy()*/,
                               i*beatPxLength_, br.height()/*+scrollDy()*/ );
        }
    }
}
