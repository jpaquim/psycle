// -*- mode:c++; indent-tabs-mode:t -*-
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
#include <qpsyclePch.hpp>

#include "beatruler.hpp"
#include "sequencerdraw.hpp"

#include <QGraphicsScene>
#include <QVarLengthArray>

namespace qpsycle {

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
	///\ todo: this paint method uses a lot of processor power when the play line
	// is being updated over the top of it -- need a way to only draw the part which
	// has been invalided.  Possibly. do this by moving to SeqDraw::drawBackground.
	Q_UNUSED( option ); Q_UNUSED( widget );

	int cw = boundingRect().width();
	int ch = boundingRect().height();

	painter->fillRect( boundingRect(), QColor( 230, 230, 230 ) );

	int start = 0;
	int end   = sDraw_->width();
	QVarLengthArray<QLineF, 100> lines1;
	QVarLengthArray<QLineF, 100> lines2;
 	QString beatLabel;
 	QRectF textRect;

	for (int i = start ; i < end ; i++) 
	{
		if (! (i % 16)) 
		{
			painter->setPen( QColor( 50, 50, 50 ) );
			lines1.append( QLineF( i * sDraw_->beatPxLength(), ch-10, i * sDraw_->beatPxLength(), ch-1 ) );
			beatLabel = QString::number(i/4);
			textRect.setRect( i * sDraw_->beatPxLength()-10, 0, 20, ch-10 );
			painter->drawText( textRect, Qt::AlignHCenter | Qt::AlignBottom, beatLabel );
		}
		else {
			if ( sDraw_->beatPxLength() > 3 ) {
				lines2.append( QLineF( i * sDraw_->beatPxLength(), ch-10, i*sDraw_->beatPxLength(), ch-5) );
			}
		}
	}
	painter->setPen( QColor( 180, 180, 180 ) );
	painter->drawLines( lines1.data(), lines1.size() );

 	painter->setPen( QColor( 220, 220, 220 ) );
 	painter->drawLines( lines2.data(), lines2.size() );
	painter->drawLine( 0, ch - 10 , cw, ch - 10 );
}

int BeatRuler::preferredHeight( ) const
{
	return 30;
}

} // namespace qpsycle
