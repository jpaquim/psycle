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

#include "psycore/player.h"

#include "sequencerview.h"
#include "sequencerdraw.h"
#include "sequencerline.h"

#include <iostream>
#include <vector>

#include <QAction>
#include <QGraphicsScene>

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
    connect( delTrkAct, SIGNAL( triggered() ), seqDraw_, SLOT( deleteTrack() ) );
    QAction *upTrkAct = toolBar_->addAction( "Move Track Up" );
    toolBar_->addAction( upTrkAct );
    connect( upTrkAct, SIGNAL( triggered() ), seqDraw_, SLOT( moveUpTrack() ) );
    QAction *dnTrkAct = toolBar_->addAction( "Move Track Down" );
    toolBar_->addAction( dnTrkAct );
    connect( dnTrkAct, SIGNAL( triggered() ), seqDraw_, SLOT( moveDownTrack() ) );

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
            seqDraw_->pLine()->setPos( xPos, 0 );
            seqDraw_->pLine()->update( seqDraw_->pLine()->boundingRect() );
        }
        oldPlayPos_ = psy::core::Player::Instance()->playPos();
    }
}

