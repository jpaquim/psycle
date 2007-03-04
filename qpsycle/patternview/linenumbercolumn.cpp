/***************************************************************************
*   Copyright (C) 2007 by  Neil Mather   *
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

#include <QPen>
#include <QRectF>

#include "linenumbercolumn.h"

LineNumberColumn::LineNumberColumn( PatternView *patView ) : dy_(0)
{
    patternView = patView;
    setPen( QPen( Qt::red ) );
}

LineNumberColumn::~ LineNumberColumn( )
{
}

QRectF LineNumberColumn::boundingRect() const 
{
    return QRectF( 0, 0, 60, patternView->height() );
}

void LineNumberColumn::paint( QPainter *painter,
                               const QStyleOptionGraphicsItem *option,
                               QWidget *widget )
{
//    TimeSignature signature;
    painter->setPen( QPen ( Qt::red ) );
    painter->drawRect(0, 0, 60, patternView->height() );

//    ngrs::Rect repaintRect = g.repaintArea().rectClipBox();
			int ch      = patternView->height();
    
    // the start for whole repaint
    int start    = dy_ / patternView->rowHeight();
    // the endline for whole repaint
    int end     = (dy_ + ch) / (patternView->rowHeight());

    int startLine = start;
    int endLine   = end;
    
//    qreal width = boundingRect.width();
    for (int i = startLine; i <= endLine; i++)
        painter->drawLine( 0,(i+1)*patternView->rowHeight() - dy() -1,
        60,(i+1)*patternView->rowHeight() -1 - dy() );

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
        QString text = QString::number( i );
        painter->drawText( 5, i*patternView->rowHeight()-dy(), text );
    }
}

void LineNumberColumn::setDy( int dy ) {
    dy_ = dy;
}

int LineNumberColumn::dy( ) const {
    return dy_;
}

int LineNumberColumn::preferredWidth() const {
    return 70;
}
