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

#include <QtGui>

#include "patternview.h"
#include "patterngrid.h"
#include "header.h"

#include "psycore/song.h"



 PatternView::PatternView( psy::core::Song *song_ )
 {
     setAlignment( Qt::AlignLeft | Qt::AlignTop );
     scene_ = new QGraphicsScene(this);
     scene_->setBackgroundBrush( QColor( 30, 30, 30 ) );
     setSceneRect(0,0,width(),height());
     setScene(scene_);
     
    psy::core::PatternCategory* category0 = song_->patternSequence()->patternData()->createNewCategory("Category0");
    psy::core::SinglePattern* pattern0 = category0->createNewPattern("Pattern0");
    pattern_ = pattern0;

    psy::core::PatternEvent event0 = pattern_->event( 0, 0 );
    psy::core::Machine* tmac = song_->_pMachine[ song_->seqBus ];
    event0.setNote( 4 * 12 + 0);
    event0.setSharp( false );
    if (tmac) event0.setMachine( tmac->_macIndex );
    if (tmac && tmac->_type == psy::core::MACH_SAMPLER ) {
        event0.setInstrument( song_->instSelected );
    }
    pattern_->setEvent( 0, 0, event0 );

    psy::core::PatternEvent event1 = pattern_->event( 2, 0 );
    event1.setNote( 4 * 12 + 0);
    event1.setSharp( false );
    if (tmac) event1.setMachine( tmac->_macIndex );
    if (tmac && tmac->_type == psy::core::MACH_SAMPLER ) {
        event1.setInstrument( song_->instSelected );
    }
    pattern_->setEvent( 2, 0, event1 );


     lineNumCol_ = new LineNumberColumn( this );
     Header *trackHeader = new Header( this );
     patGrid_ = new PatternGrid( this );
     trackHeader->setPos( 50, 0 );
     scene_->addItem( lineNumCol_ );
     scene_->addItem( trackHeader );
     scene_->addItem( patGrid_ );
     patGrid_->setPos( 50, 20 );

    // Create the toolbar.
    //     createToolBar();
 }

 void PatternView::createToolBar()
 {
      toolBar_ = new QToolBar();
      meterCbx_ = new QComboBox();
      meterCbx_->addItem("4/4");
      meterCbx_->addItem("3/4");

      delBarAct_ = new QAction(tr("Delete Bar"), this);
      delBarAct_->setStatusTip(tr("Delete a bar"));

      toolBar_->addWidget(meterCbx_);
      toolBar_->addAction(delBarAct_);

      layout->addWidget(toolBar_);
  }

int PatternView::rowHeight( ) const
{
    return 13;
}

int PatternView::numberOfLines() const
{
    return 4;
}

int PatternView::numberOfTracks() const
{
    return 2;
}

int PatternView::trackWidth() const
{
    return 100;
}

psy::core::SinglePattern * PatternView::pattern( )
{
    return pattern_;
}


