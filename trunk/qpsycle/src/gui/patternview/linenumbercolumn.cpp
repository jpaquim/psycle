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
#include "linenumbercolumn.hpp"
#include "patterndraw.hpp"
#include "patternview.hpp"
#include "patterngrid.hpp"

#include <QPen>
#include <QRectF>
#include <QPaintEvent>
#include <QScrollBar>

namespace qpsycle {

const int LineNumberColumn::width_ = 50;

LineNumberColumn::LineNumberColumn( PatternDraw *patDraw ) 
	: QWidget( patDraw )
{
	patternDraw_ = patDraw;
}

LineNumberColumn::~ LineNumberColumn( )
{
}

void LineNumberColumn::paintEvent( QPaintEvent *event ) 
{
	Q_UNUSED( event );
	QPainter painter( this );
	setGeometry(0, patternDraw()->trackHeaderHeight()+2, width(), patternDraw()->viewport()->height() );
	
	int ch = patternDraw()->viewport()->height();
	QColor lineColor( Qt::black );
	QColor textColor( QColor(200,200,200) );
//    TimeSignature signature;
	int rowHeight = patternDraw()->rowHeight();
	painter.setPen( QPen ( Qt::black ) );
	painter.setBrush( QBrush ( QColor(30,30,30 ) ) );
	painter.drawRect(0, 0, width(), ch );

	int dy_ = patternDraw()->verticalScrollBar()->value();
	int absTop  = 0;//absoluteTop();
			// the start for whole repaint
	int start    = dy_ / rowHeight;
	// the offset for the repaint expose request
	int startOff = std::max( ( x() - absTop) / rowHeight , 0 );
	// the start
	start        = std::min(start + startOff, patternDraw()->patternGrid()->endLineNumber() );
	// the endline for whole repaint
	int end     = (dy_ + ch) / rowHeight;
	// the offset for the repaint expose request
	int endOff  = std::max( (ch-(x()-absTop + height())) / rowHeight, 0);
	// the end
	end         = std::min(end - endOff, patternDraw()->patternGrid()->endLineNumber() );

	int startLine = start;
	int endLine   = end;

	for (int i = startLine; i <= endLine; i++) {
		painter.drawLine(0,(i+1)*rowHeight - dy_, width(),(i+1)*rowHeight - dy_ );
	}

	for (int i = startLine; i <= endLine; i++) 
	{
				QString text = QString::number( i );
				QRectF textBound( 0, i*rowHeight - dy_, width(), rowHeight );
				painter.setPen( QPen ( textColor ) );
				painter.drawText( textBound, text, QTextOption( Qt::AlignRight ) );
	}
	
}

const int LineNumberColumn::width()
{
	return width_;
}
} // namespace qpsycle
