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
#include "patterndraw.h"

#include "psycore/song.h"



PatternView::PatternView( psy::core::Song *song )
{
    song_ = song;
    pattern_ = 0;
    setNumberOfTracks( 6 );
    patDraw_ = new PatternDraw( this );
    setPattern( new psy::core::SinglePattern() );

    setSelectedMachineIndex( 255 ); // FIXME: why 255?
    layout = new QVBoxLayout();
    setLayout( layout );
    // Create the toolbar.
    createToolBar();
    layout->addWidget(toolBar_);
    layout->addWidget( patDraw_ );
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

  }

int PatternView::rowHeight( ) const
{
    return 13;
}

int PatternView::numberOfLines() const
{
    return ( pattern_ ) ? static_cast<int> ( pattern_->beatZoom() * pattern_->beats() ) : 0;  
}

int PatternView::numberOfTracks() const
{
    return numberOfTracks_;
}

void PatternView::setNumberOfTracks( int numTracks )
{
    numberOfTracks_ = numTracks; 
}

int PatternView::trackWidth() const
{
    return 130;
}

psy::core::SinglePattern * PatternView::pattern( )
{
    return pattern_;
}

void PatternView::setPattern( psy::core::SinglePattern *pattern )
{
    pattern_ = pattern;
    patDraw_->patternGrid()->update();
}


void PatternView::enterNote( const PatCursor & cursor, int note ) 
{
    if ( pattern() ) {
        psy::core::PatternEvent event = pattern()->event( cursor.line(), cursor.track() );
        psy::core::Machine* tmac = song_->_pMachine[ song_->seqBus ];
        event.setNote( 4/*editOctave()*/ * 12 + note );
        event.setSharp( false/*drawArea->sharpMode()*/ );
        if (tmac) event.setMachine( tmac->_macIndex );
        if (tmac && tmac->_type == psy::core::MACH_SAMPLER ) {
            event.setInstrument( song_->instSelected );
        }
        pattern()->setEvent( cursor.line(), cursor.track(), event );
//        if (tmac) PlayNote( editOctave() * 12 + note, 127, false, tmac);   
    }
}

void PatternView::setSelectedMachineIndex( int idx )
{
    selectedMacIdx_ = idx;
}

int PatternView::selectedMachineIndex( ) const
{
    return selectedMacIdx_;
}

void PatternView::keyPressEvent( QKeyEvent *event )
{
    switch ( event->key() ) {
        case Qt::Key_A:
        {
            float position = patDraw()->patternGrid()->cursor().line() / (float) beatZoom();
            pattern()->removeBar(position);
            patDraw()->patternGrid()->update();
        }
        break;
        default:
            event->ignore();
    }
}

int PatternView::beatZoom( ) const
{
    if ( pattern_ )
        return pattern_->beatZoom();
    else
        return 4;
}

