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

#include <QtGui>

#include "patternview.h"
#include "patterndraw.h"
#include "patterngrid.h"
#include "header.h"

#include "psycore/song.h"

 PatternDraw::PatternDraw( PatternView *patView )
 {
     patView_ = patView; 
     setAlignment( Qt::AlignLeft | Qt::AlignTop );
     scene_ = new QGraphicsScene(this);
     scene_->setBackgroundBrush( QColor( 30, 30, 30 ) );
     setScene(scene_);
     
     lineNumCol_ = new LineNumberColumn( this );
     Header *trackHeader = new Header( this );
     patGrid_ = new PatternGrid( this );

     scene_->addItem( lineNumCol_ );
     scene_->addItem( trackHeader );
     scene_->addItem( patGrid_ );

     trackHeader->setPos( 50, 0 );
     lineNumCol_->setPos( 0, 20 );
     patGrid_->setPos( 50, 20 );
 }
