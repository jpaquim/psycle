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

#include "linenumbercolumn.h"

#include <QPen>
#include <QRectF>
#include <QPaintEvent>
#include <QScrollBar>

LineNumberColumn::LineNumberColumn( PatternDraw *patDraw, QWidget *parent ) 
    : QWidget( parent )
{
    patternDraw_ = patDraw;
}

LineNumberColumn::~ LineNumberColumn( )
{
}

void LineNumberColumn::paintEvent( QPaintEvent *event ) 
{
    QPainter painter( this );
    int columnWidth = 50;
    int ch      = height();//clientHeight();
    QColor lineColor( Qt::black );
    QColor textColor( QColor(200,200,200) );
//    TimeSignature signature;
    int rowHeight = patternDraw()->patternView()->rowHeight();
    painter.setPen( QPen ( Qt::black ) );
    painter.setBrush( QBrush ( QColor(30,30,30 ) ) );
    painter.drawRect(0, 0, columnWidth, ch );

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

			for (int i = startLine; i <= endLine; i++)
				painter.drawLine(0,(i+1)*rowHeight - dy_ -1, width(),(i+1)*rowHeight -1 - dy_ );

//			g.setForeground(pView->separatorColor());
//			g.drawLine(0,0,0,clientHeight() );

			for (int i = startLine; i <= endLine; i++) 
            {
                QString text = QString::number( i );
                QRectF textBound( 0, i*rowHeight - dy_, columnWidth, rowHeight );
                painter.setPen( QPen ( textColor ) );
                painter.drawText( textBound, text, QTextOption( Qt::AlignRight ) );
/*				//if (i+startLine == pView->cursor().line()) {
				//  g.setForeground(Global::pConfig()->pvc_cursor);
				//  g.fillRect(0,i*pView->rowHeight()+pView->headerHeight(),clientWidth()-1,pView->rowHeight());
				//}
				std::string text = QString::number( i ).toStdString();
				if ( patternDraw()->patternView()->pattern() ) {
					float position = i  / (float) pView->pattern()->beatZoom();
					SinglePattern::iterator it = pView->pattern()->find_nearest(i);
					if (it != pView->pattern()->end()) {
						// check out how many hidden lines there are
						int lastLine = d2i (it->first * pView->pattern_->beatZoom());
						int y = lastLine;
						SinglePattern::iterator it2 = it;
						int count = 0;
						do {
							y = d2i (it2->first * pView->pattern_->beatZoom());
							if ( y != lastLine) break;
							it2++;
							count++;
						} while (it2 != pView->pattern()->end() && y == lastLine);

						if ( count > 1 ) {
							text+= std::string(":") + stringify( count );
						}
						// check if line is on beatzoom raster else draw arrow up or down hint
						if ( std::abs(it->first - position) > 0.001) {
							int xOff = clientWidth()-g.textWidth(text)- 10 ;
							int yOff = i*pView->rowHeight()+pView->rowHeight() -1 - 3 -dy();
							g.drawLine( xOff , yOff+1, xOff, yOff - pView->rowHeight() + 5);
							if (it->first < position) {
								g.drawLine( xOff , yOff - pView->rowHeight() + 5, xOff-3, yOff - pView->rowHeight() + 8);
								g.drawLine( xOff , yOff - pView->rowHeight() + 5, xOff+4, yOff - pView->rowHeight() + 9);
							} else {
								g.drawLine( xOff , yOff +1, xOff-3, yOff - 2);
								g.drawLine(  xOff , yOff +1, xOff+4, yOff - 3);
							}
						}
					}
					if ( pView->pattern()->barStart(position, signature) ) {
						std::string caption = stringify(signature.numerator())+"/"+stringify(signature.denominator());
						// vcenter text
						int yp = (pView->rowHeight() - g.textHeight()) / 2  + g.textAscent();
						g.drawText(0,i*pView->rowHeight()+ yp - dy(),caption, textColor() );
					}
				}
				int yp = (pView->rowHeight() - g.textHeight()) / 2  + g.textAscent();
				g.drawText(clientWidth()-g.textWidth(text)-3,i*pView->rowHeight()+yp-dy(),text, textColor() );*/
			}

    
/*    // the start for whole repaint
    int start    = patternDraw()->verticalScrollBar()->value();
    // the endline for whole repaint
    int end     = patternDraw()->patternView()->numberOfLines()-1;

    int startLine = 4;//start * rowHeight;
    int endLine   = end;
    
    for (int i = startLine; i <= endLine; i++)
    {
        painter.setPen( QPen ( lineColor ) );
        painter.drawLine( 0, i*rowHeight,
                           columnWidth, i*rowHeight );

        QString text = QString::number( i );
        QRectF textBound( 0, i*rowHeight, columnWidth-2, rowHeight );
        painter.setPen( QPen ( textColor ) );
        painter.drawText( textBound, text, QTextOption( Qt::AlignRight ) );
   }

    for (int i = startLine; i <= endLine; i++) {
        //if (i+startLine == pView->cursor().line()) {
        //  g.setForeground(Global::pConfig()->pvc_cursor);
        //  g.fillRect(0,i*pView->rowHeight()+pView->headerHeight(),clientWidth()-1,pView->rowHeight());
        //}
        //std::string text = stringify( i );
        /*if ( pView->pattern() ) {
            float position = i  / (float) pView->pattern()->beatZoom();
            SinglePattern::iterator it = pView->pattern()->find_nearest(i);
            if (it != pView->pattern()->end()) {
                // check out how many hidden lines there are
                int lastLine = d2i (it->first * pView->pattern_->beatZoom());
                int y = lastLine;
                SinglePattern::iterator it2 = it;
                int count = 0;
                do {
                    y = d2i (it2->first * pView->pattern_->beatZoom());
                    if ( y != lastLine) break;
                    it2++;
                    count++;
                } while (it2 != pView->pattern()->end() && y == lastLine);

                if ( count > 1 ) {
                    text+= std::string(":") + stringify( count );
                }
                // check if line is on beatzoom raster else draw arrow up or down hint
                if ( std::abs(it->first - position) > 0.001) {
                    int xOff = clientWidth()-g.textWidth(text)- 10 ;
                    int yOff = i*pView->rowHeight()+pView->rowHeight() -1 - 3 -dy();
                    g.drawLine( xOff , yOff+1, xOff, yOff - pView->rowHeight() + 5);
                    if (it->first < position) {
                        g.drawLine( xOff , yOff - pView->rowHeight() + 5, xOff-3, yOff - pView->rowHeight() + 8);
                        g.drawLine( xOff , yOff - pView->rowHeight() + 5, xOff+4, yOff - pView->rowHeight() + 9);
                    } else {
                        g.drawLine( xOff , yOff +1, xOff-3, yOff - 2);
                        g.drawLine(  xOff , yOff +1, xOff+4, yOff - 3);
                    }
                }
            }
            if ( pView->pattern()->barStart(position, signature) ) {
                std::string caption = stringify(signature.numerator())+"/"+stringify(signature.denominator());
                // vcenter text
                int yp = (pView->rowHeight() - g.textHeight()) / 2  + g.textAscent();
                g.drawText(0,i*pView->rowHeight()+ yp - dy(),caption, textColor() );
            }
        }*/
//        int yp = (patternView->rowHeight() - g.textHeight()) / 2  + g.textAscent();
    ///}
}
