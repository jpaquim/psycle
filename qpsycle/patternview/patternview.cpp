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

#include "psycore/song.h"
#include "psycore/player.h"

#include "patternview.h"
#include "patterndraw.h"

#include <QtGui>

int d2i(double d)
{
		return (int) ( d<0?d-.5:d+.5);
}

PatternView::PatternView( psy::core::Song *song )
{
    song_ = song;
    pattern_ = 0;
    patternStep_ = 1;
    setNumberOfTracks( 6 );
    patDraw_ = new PatternDraw( this );
    setPattern( new psy::core::SinglePattern() );

    playPos_ = 0;

    setSelectedMachineIndex( 255 ); // FIXME: why 255?
    layout = new QVBoxLayout();
    setLayout( layout );
    // Create the toolbar.
    createToolBar();
    layout->addWidget( toolBar_ );
    layout->addWidget( patDraw_ );
}

void PatternView::createToolBar()
{
    toolBar_ = new QToolBar();

    patStepCbx_ = new QComboBox();
    for ( int i = 0; i < 17; i++ ) {
        patStepCbx_->addItem( QString::number( i ) );
    }
    patStepCbx_->setCurrentIndex( 1 );
    connect( patStepCbx_, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( onPatternStepComboBoxIndexChanged( int ) ) );

    delBarAct_ = new QAction( "Delete Bar", this );
    delBarAct_->setStatusTip( "Delete a bar" );

    toolBar_->addWidget( new QLabel( "Step: " ) );
    toolBar_->addWidget( patStepCbx_ );
    toolBar_->addSeparator();
    toolBar_->addAction( delBarAct_ );

}

void PatternView::enterNote( const PatCursor & cursor, int note ) 
{
    if ( pattern() ) {
        psy::core::PatternEvent event = pattern()->event( cursor.line(), cursor.track() );
        psy::core::Machine* tmac = song_->_pMachine[ song_->seqBus ];
        event.setNote( octave() * 12 + note );
        event.setSharp( false/*drawArea->sharpMode()*/ );
        if (tmac) event.setMachine( tmac->_macIndex );
        if (tmac && tmac->_type == psy::core::MACH_SAMPLER ) {
            event.setInstrument( song_->instSelected );
        }
        pattern()->setEvent( cursor.line(), cursor.track(), event );
//        if (tmac) PlayNote( octave() * 12 + note, 127, false, tmac);   
    }
}

void PatternView::clearNote( const PatCursor & cursor) {
    if ( pattern() ) {
        psy::core::PatternEvent event = pattern()->event( cursor.line(), cursor.track() );
        psy::core::Machine* tmac = song_->_pMachine[ song_->seqBus ];
        event.setNote(255);
        event.setSharp( false/*drawArea->sharpMode()*/ );
        pattern()->setEvent( cursor.line(), cursor.track(), event );
    }
}

void PatternView::onTick( double sequenceStart ) {
    if ( pattern() ) {
        int liney = d2i ( ( psy::core::Player::Instance()->playPos() - sequenceStart ) * beatZoom() );
        if ( liney != playPos_ ) {			
            int oldPlayPos = playPos_;
            playPos_ = liney;
            int startTrack = 0;//drawArea->findTrackByScreenX( drawArea->dx() );
            int endTrack = numberOfTracks();//drawArea->findTrackByScreenX( drawArea->dx() + drawArea->clientWidth() );
            patternGrid()->update( patternGrid()->repaintTrackArea( oldPlayPos, oldPlayPos, startTrack, endTrack ) );
            patternGrid()->update( patternGrid()->repaintTrackArea( liney, liney, startTrack, endTrack ) );
        }
    }
}


// Getters.
int PatternView::rowHeight( ) const
{
    return 13;
}

int PatternView::numberOfLines() const
{
    return ( pattern() ) ? static_cast<int> ( pattern()->beatZoom() * pattern()->beats() ) : 0;  
}

int PatternView::numberOfTracks() const
{
    return numberOfTracks_;
}

int PatternView::trackWidth() const
{
    return 130;
}

int PatternView::selectedMachineIndex( ) const
{
    return selectedMacIdx_;
}

int PatternView::beatZoom( ) const
{
    if ( pattern() )
        return pattern()->beatZoom();
    else
        return 4;
}

PatternGrid* PatternView::patternGrid() 
{ 
    return patDraw()->patternGrid(); 
}

int PatternView::patternStep( ) const
{
    return patternStep_;
}

int PatternView::octave( ) const
{
    return octave_;
}




// Setters.
void PatternView::setNumberOfTracks( int numTracks )
{
    numberOfTracks_ = numTracks;
}

void PatternView::setPattern( psy::core::SinglePattern *pattern )
{
    pattern_ = pattern;
    patternGrid()->update();
}

void PatternView::setSelectedMachineIndex( int idx )
{
    selectedMacIdx_ = idx;
}

void PatternView::setPatternStep( int newStep )
{
    patternStep_ = newStep;
}

void PatternView::setOctave( int newOctave )
{
    octave_ = newOctave;
}

// GUI events.
void PatternView::onPatternStepComboBoxIndexChanged( int newIndex )
{
    setPatternStep( newIndex );
}

void PatternView::keyPressEvent( QKeyEvent *event )
{
    int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers(), event->key() ) );
    switch ( command ) {
/*        case Qt::Key_A:
        {
            float position = patternGrid()->cursor().line() / (float) beatZoom();
            pattern()->removeBar(position);
            patternGrid()->update();
            break;
        }*/
        case psy::core::cdefPatternstepDec:
            patStepCbx_->setCurrentIndex( std::max( 0, patternStep() - 1 ) );
        break;
        case psy::core::cdefPatternstepInc:
            patStepCbx_->setCurrentIndex( std::min( 16, patternStep() + 1 ) );
        break;
        default:
            event->ignore();
    }
}

