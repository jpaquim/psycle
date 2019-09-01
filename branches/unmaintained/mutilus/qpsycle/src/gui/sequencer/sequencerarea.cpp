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

#include <psycle/core/patternsequence.h>

#include "sequencerarea.h"
#include "sequencerview.h"
#include "sequencerdraw.h"
#include "sequencerline.h"
#include "sequenceritem.h"


#include <vector>

SequencerArea::SequencerArea( SequencerDraw *seqDrawIn )
	: seqDraw_( seqDrawIn )
{
	beatPxLength_ = sequencerDraw()->beatPxLength();
}

QRectF SequencerArea::boundingRect() const 
{
	int width = std::max( seqDraw_->width(), (int)childrenBoundingRect().width() );
	int height = std::max( seqDraw_->height(), (int)childrenBoundingRect().height() );
	return QRectF( 0, 0, width, height );
}


void SequencerArea::paint( QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	drawTimegrid( painter );
}

void SequencerArea::drawTimegrid( QPainter *painter )
{
	QRectF br = boundingRect();
	int start = 0;
	int end   = (int)br.width();

	painter->setPen( QColor( 30, 30, 30 ) );
	for (int i = start ; i <= end ; i++) {
		if ( beatPxLength_ > 3 || (beatPxLength_ <= 3 && (!( i %16)))  ) {
			painter->drawLine( i*beatPxLength_, 0,
								i*beatPxLength_, br.height() );
		}
	}
}
