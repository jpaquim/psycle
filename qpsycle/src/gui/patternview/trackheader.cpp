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

#include <psycle/core/signalslib.h>

#include "trackheader.hpp"
#include "patterndraw.hpp"
#include "patterngrid.hpp"
#include "patternview.hpp"

#include <QScrollBar>
#include <QDebug>
#include <QPainter>
#include <QGraphicsRectItem>

namespace qpsycle {

const int TrackHeader::height_ = 20;

TrackHeader::TrackHeader( PatternDraw * pPatternDraw ) 
	: QWidget(pPatternDraw),
		pDraw(pPatternDraw)
{}

TrackHeader::~ TrackHeader( )
{}

void TrackHeader::paintEvent( QPaintEvent *event ) 
{
	Q_UNUSED( event );

	int viewableWidth = pDraw->width();

	QPainter painter(this);
	painter.setBrush( QBrush( QColor(30,30,30) ) );
	setGeometry( pDraw->lineNumColWidth(), 0, viewableWidth, height() );
	painter.drawRect( 0, 0, viewableWidth, height() );

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
		QRectF textBound( xOff, 0, trackGeometry.width(), height_ );
		painter.drawText( textBound, text, QTextOption( Qt::AlignCenter ) );
		if (it->first!=0) {
			painter.drawLine( xOff, 0, xOff, height() ); // col seperator
		}
	}
}
const int TrackHeader::height()
{
	return height_;
}
} // namespace qpsycle
