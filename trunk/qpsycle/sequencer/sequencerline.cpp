/***************************************************************************
*   Copyright (C) 2007 by Neil Mather   *
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

#include "sequencerline.h"
#include "sequencerview.h"
#include "sequenceritem.h"

#include "psycore/patternsequence.h"

 SequencerLine::SequencerLine()
 {
     setRect(QRectF(0, 0, 500, 30));
     setPen(QPen(Qt::white,1));
     setBrush(QBrush(Qt::transparent));
 }

void SequencerLine::setSequenceLine( psy::core::SequenceLine * line )
{
    seqLine_ = line;
}

