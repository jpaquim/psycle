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
#include "beatruler.hpp"
#include "sequencerdraw.hpp"

#include <QGraphicsScene>
#include <QVarLengthArray>
#include <QScrollBar>

namespace qpsycle {

BeatRuler::BeatRuler( SequencerDraw* seqDraw )
	: QWidget( seqDraw )
{
	sDraw_ = seqDraw;
}

BeatRuler::~BeatRuler( )
{
}

///\ todo Update this so it makes use of event->region(), which gives the geometry
// of the invalidated region that needs repainting.  At the moment it repaints
// everything regardless of which bit got invalidated.
///\ todo Need to draw the playline.
void BeatRuler::paintEvent( QPaintEvent *event )
{
	///\todo Seems incorrect (and wasteful) to call setGeometry in here, but if we don't
	// the beatruler isn't the right size after a window resize.  Should be able to
	// put this somewhere else, but not sure where yet.
	int borderWidth = 2;  ///\todo Not sure how to get this programatically.
	setGeometry( borderWidth, borderWidth, sDraw_->viewport()->width(), 30 );

	QPainter painter(this);

	int myWidth = width();
	int myHeight = height();

	painter.fillRect( 0, 0, myWidth, 30, QColor( 230, 230, 230 ) );
	int scrollDx = sDraw_->horizontalScrollBar()->value();

	int start = 0;
	int end   = myWidth;
	QVarLengthArray<QLineF, 100> lines1;
	QVarLengthArray<QLineF, 100> lines2;
	QString beatLabel;
	QRectF textRect;

	for (int i = start; i < end ; i++) 
	{
		int currentX = i *sDraw_->beatPxLength()-scrollDx;
		if (! (i % 16)) 
		{
			painter.setPen( QColor( 50, 50, 50 ) );
			lines1.append( QLineF( currentX, myHeight-10, currentX, myHeight-1 ) );
			beatLabel = QString::number(i/4);
			textRect.setRect( currentX - 10, 0, 20, myHeight-10 );
			painter.drawText( textRect, Qt::AlignHCenter | Qt::AlignBottom, beatLabel );
		}
		else {
			if ( sDraw_->beatPxLength() > 3 ) {
				lines2.append( QLineF( currentX, myHeight-10, currentX, myHeight-5) );
			}
		}
	}
	painter.setPen( QColor( 180, 180, 180 ) );
	painter.drawLines( lines1.data(), lines1.size() );

	painter.setPen( QColor( 220, 220, 220 ) );
	painter.drawLines( lines2.data(), lines2.size() );
	painter.drawLine( 0, myHeight - 10 , myWidth, myHeight - 10 );
}

int BeatRuler::preferredHeight( ) const
{
	return 30;
}

} // namespace qpsycle
