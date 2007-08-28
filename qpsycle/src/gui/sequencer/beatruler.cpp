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
#include "qpsyclePch.hpp"

#include "beatruler.h"
#include "sequencerdraw.h"

#include <QGraphicsScene>

BeatRuler::BeatRuler( SequencerDraw* seqDraw )
{
	sDraw_ = seqDraw;
}

BeatRuler::~BeatRuler( )
{
}

QRectF BeatRuler::boundingRect() const
{
	int width = std::max( sDraw_->width(), (int)scene()->width() );
	return QRectF( 0, 0, width, preferredHeight() ) ;
}

void BeatRuler::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	Q_UNUSED( option ); Q_UNUSED( widget );

	int cw = boundingRect().width();
	int ch = boundingRect().height();


	painter->setBrush( QColor( 230,230,230 ) );
	painter->setPen( QColor( 230,230,230 ) );
	painter->fillRect( boundingRect(), QColor( 230, 230, 230 ) );

	int start = 0;
	int end   = sDraw_->width();

	for (int i = start ; i < end ; i++) {
		if (! (i % 16)) {
			painter->setPen( QColor( 180, 180, 180 ) );
			painter->drawLine( i * sDraw_->beatPxLength(), ch-10, 
						i * sDraw_->beatPxLength(), ch-1 );
			QString beatLabel = QString::number(i/4);
			QRectF textRect = QRectF( i * sDraw_->beatPxLength()-10, 0, 20, ch-10 );
			painter->setPen( QColor( 50, 50, 50 ) );
			painter->drawText( textRect, Qt::AlignHCenter | Qt::AlignBottom, beatLabel );
		}
		else {
			if ( sDraw_->beatPxLength() > 3 ) {
				painter->setPen( QColor( 220, 220, 220 ) );
				painter->drawLine( i * sDraw_->beatPxLength(), ch-10, i*sDraw_->beatPxLength(), ch-5);
			}
		}
	}
	painter->setPen( QColor( 220, 220, 220 ) );
	painter->drawLine( 0, ch - 10 , cw, ch - 10 );
}

int BeatRuler::preferredHeight( ) const
{
	return 30;
}

