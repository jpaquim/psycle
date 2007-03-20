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

#include <QAction>
#include <QGraphicsScene>

 #include "sequencerview.h"
 #include "sequencerline.h"

 #include "psycore/player.h"

/*int d2i(double d)
{
		return (int) ( d<0?d-.5:d+.5);
}*/

SequencerView::SequencerView( psy::core::Song *asong )
{
    song_ = asong;

    layout_ = new QVBoxLayout();

    seqDraw_ = new SequencerDraw( this );
    toolBar_ = new QToolBar();
    toolBar_->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    QAction *insTrkAct = toolBar_->addAction( "Insert Track" );
    connect( insTrkAct, SIGNAL( triggered() ), seqDraw_, SLOT( insertTrack() ) );
    QAction *delTrkAct = toolBar_->addAction( "Delete Track" );
    toolBar_->addAction( delTrkAct );
    QAction *dnTrkAct = toolBar_->addAction( "Move Track Down" );
    toolBar_->addAction( dnTrkAct );
    QAction *upTrkAct = toolBar_->addAction( "Move Track Up" );
    toolBar_->addAction( upTrkAct );

    layout_->addWidget( toolBar_ );
    layout_->addWidget( seqDraw_ );

    setLayout( layout_ );
}


SequencerLine* SequencerView::selectedLine() 
{
    return selectedLine_;
}

void SequencerView::addPattern( psy::core::SinglePattern *pattern )
{
    if ( seqDraw_->selectedLine() ) {
        seqDraw_->selectedLine()->addItem( pattern );
    }
}

void SequencerView::updatePlayPos() {
    if ( song()->patternSequence() /*&& scrollArea() && !scrollArea()->lockPlayLine()*/ ) {
        int beatPxLength = seqDraw_->beatPxLength();
        int xPos =  std::min(song()->patternSequence()->tickLength()* beatPxLength, psy::core::Player::Instance()->playPos() * beatPxLength);
        int oxPos = std::min(song()->patternSequence()->tickLength()* beatPxLength, oldPlayPos_ * beatPxLength);
        if (oxPos != xPos) {
            seqDraw_->pLine()->setLine( xPos, 0, xPos, seqDraw_->height() );
            seqDraw_->pLine()->update( seqDraw_->pLine()->boundingRect() );
        }
        oldPlayPos_ = psy::core::Player::Instance()->playPos();
    }
}

