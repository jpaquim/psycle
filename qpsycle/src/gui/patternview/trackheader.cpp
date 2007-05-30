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
#include <psycore/signalslib.h>

#include "trackheader.h"
#include "patterndraw.h"
#include "patterngrid.h"
#include "patternview.h"

#include <QScrollBar>
#include <QDebug>
#include <QPainter>
#include <QGraphicsRectItem>

TrackHeader::TrackHeader( PatternDraw * pPatternDraw, QWidget *parent ) 
    : pDraw(pPatternDraw), QWidget(parent)
{
}

TrackHeader::~ TrackHeader( )
{ 
}

void TrackHeader::paintEvent( QPaintEvent *event ) 
{
    int trackHeight = 20;
    int numTracks = pDraw->patternView()->numberOfTracks();

    QPainter painter(this);
    painter.setBrush( QBrush( QColor(30,30,30) ) );
    painter.drawRect( 0, 0, width(), trackHeight );

/*    for ( int i = 0; i < numTracks; i++ )
    {
        int trackWidth = pDraw->xEndByTrack( i ) - pDraw->xOffByTrack( i );
        painter->setPen( QPen( Qt::black ) );
        painter->setBrush( QBrush( Qt::black ) );
        painter->drawRect( i*trackWidth, 0, trackWidth, trackHeight ); 
        painter->setPen( QPen( Qt::white ) );
        painter->drawText( i*trackWidth+5, 15, QString::number(i) );
        painter->setPen( QPen( Qt::gray ) );
        painter->setBrush( QBrush( Qt::red ) );
        painter->drawEllipse( (i+1)*trackWidth - 15, 5, 10, 10 ); 
        painter->setBrush( QBrush( Qt::yellow ) );
        painter->drawEllipse( (i+1)*trackWidth - 30, 5, 10, 10 ); 
        painter->setBrush( QBrush( Qt::green ) );
        painter->drawEllipse( (i+1)*trackWidth - 45, 5, 10, 10 ); 
    }*/

    //g.setForeground(pDraw->patternView()->separatorColor());

    int scrollDx = pDraw->horizontalScrollBar()->value();
    int spacingWidth = 5;
    int startTrack = pDraw->findTrackByXPos( scrollDx );
    std::map<int, TrackGeometry>::const_iterator it;
    it = pDraw->trackGeometrics().lower_bound( startTrack );

    for ( ; it != pDraw->trackGeometrics().end() && it->first <= pDraw->patternGrid()->endTrackNumber(); it++) 
    {
        const TrackGeometry & trackGeometry = it->second;

        int xOff = trackGeometry.left() - scrollDx + spacingWidth;

        painter.setPen( QPen ( Qt::white ) );
        QString text = QString::number( it->first );
        QRectF textBound( xOff, 0, trackGeometry.width(), trackHeight );
        painter.drawText( textBound, text, QTextOption( Qt::AlignCenter ) );
        if (it->first!=0) {
            painter.drawLine( xOff, 0, xOff, height() ); // col seperator*/
        }
    }
}
