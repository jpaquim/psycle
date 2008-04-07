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

#include <psycle/core/patternsequence.h>

#include "sequencerarea.hpp"
#include "sequencerview.hpp"
#include "sequencerdraw.hpp"
#include "sequencerline.hpp"
#include "sequenceritem.hpp"


#include <vector>

namespace qpsycle {

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
	Q_UNUSED( painter ); Q_UNUSED( option ); Q_UNUSED( widget );
	// This stopped having any paint operations when the timeline drawing
	// moved to drawBackground of SeqDraw.  It's still acting as a container
	// for other items at the moment.
}

} // namespace qpsycle
