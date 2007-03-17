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

#include "beatruler.h"

BeatRuler::BeatRuler( SequencerDraw* seqDraw )
{
    sDraw_ = seqDraw;
}

BeatRuler::~BeatRuler( )
{
}

QRectF BeatRuler::boundingRect() const
{
    return QRectF( 0, 0, sDraw_->width(), 20 );
}

void BeatRuler::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    painter->drawRect( boundingRect() );
/*    ngrs::Rect area = g.repaintArea().rectClipBox();

    int cw = clientWidth();
    int ch = clientHeight();

    g.setForeground( ngrs::Color(220,220,220) );

    g.drawLine(scrollDx(), ch - 10 , cw + scrollDx(), ch - 10);

    int start = (area.left() - absoluteLeft() + scrollDx()) / sView->beatPxLength();
    ///\ todo end seems not valid 
    int end   = (area.left() + area.width() - absoluteLeft() + scrollDx() ) / sView->beatPxLength();

    for (int i = start ; i < end ; i++) {
        if (! (i % 16)) {
            g.setForeground( ngrs::Color( 180, 180, 180) );
            g.drawLine(i* sView->beatPxLength(),ch-10,d2i(i*sView->beatPxLength()), ch);
            std::string beatLabel = stringify(i/4);
            int textWidth = g.textWidth(beatLabel);
            g.drawText(i* sView->beatPxLength() - textWidth / 2, g.textAscent(), beatLabel);
        }
        else {
            if (sView->beatPxLength() > 3) {
                g.setForeground( ngrs::Color( 220, 220, 220) );
                g.drawLine(i* sView->beatPxLength(),ch-10,d2i(i*sView->beatPxLength()), ch-5);
            }
        }
    }*/
}

int BeatRuler::preferredHeight( ) const
{
//    ngrs::FontMetrics metrics(font());

//    return metrics.textHeight() + 10;
    return 20;
}

// end of beat ruler

