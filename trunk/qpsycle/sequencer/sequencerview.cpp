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

#include <iostream>
#include <vector>

 #include "sequencerview.h"
 #include "sequencerline.h"

SequencerView::SequencerView( psy::core::Song *asong )
{
    song_ = asong;

    layout_ = new QVBoxLayout();

    seqDraw_ = new SequencerDraw( this );
    toolBar_ = new QToolBar();
    toolBar_->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    toolBar_->addAction( "Insert Track" );
    toolBar_->addAction( "Delete Track" );
    toolBar_->addAction( "Move Track Down" );
    toolBar_->addAction( "Move Track Up" );

    layout_->addWidget( toolBar_ );
    layout_->addWidget( seqDraw_ );

    setLayout( layout_ );
}

SequencerLine* SequencerView::selectedLine() 
{
    return selectedLine_;
}

void SequencerView::setSelectedLine( SequencerLine *line ) 
{
    selectedLine_ = line;
}

void SequencerView::addPattern( psy::core::SinglePattern *pattern )
{
    if ( selectedLine() ) {
        selectedLine()->addItem( pattern );
    }
}
