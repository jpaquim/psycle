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

#include <psycle/core/song.h>
#include <psycle/core/singlepattern.h>
#include <psycle/core/player.h>

#include "../global.h"
#include "../configuration.h"
#include "patternview.h"
#include "patterndraw.h"
#include "patterngrid.h"
#include "linenumbercolumn.h"

#include <QVBoxLayout>
#include <QtGui>

int d2i(double d)
{
	return (int) ( d<0?d-.5:d+.5);
}

PatternView::PatternView( psy::core::Song *song )
{
	song_ = song;
	pattern_ = NULL;
	patternStep_ = 1;
	setNumberOfTracks( 6 );
	patDraw_ = new PatternDraw( this );

	playPos_ = 0;

	setSelectedMachineIndex( 255 ); // FIXME: why 255?
	layout = new QVBoxLayout();
	setLayout( layout );
	// Create the toolbar.
	createToolBar();
	layout->addWidget( toolBar_ );
	layout->addWidget( patDraw_ );

	patDraw_->patternGrid()->setFt2HomeEndBehaviour( Global::configuration().ft2HomeEndBehaviour() );
	patDraw_->patternGrid()->setShiftArrowForSelect( Global::configuration().shiftArrowForSelect() );
	patDraw_->patternGrid()->setWrapAround( Global::configuration().wrapAround() );
	patDraw_->patternGrid()->setCenterCursor( Global::configuration().centerCursor() );
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

	addBarAct_ = new QAction(QIcon(":images/pat_addbar.png"), "Add Bar", this );
	addBarAct_->setStatusTip( "Add a bar" );	 
		 
	delBarAct_ = new QAction(QIcon(":images/pat_delbar.png"), "Delete Bar", this );
	delBarAct_->setStatusTip( "Delete a bar" );
	
	recordCb_ = new QAction(QIcon(":/images/recordnotes.png") ,tr("Record"), this);
	recordCb_->setCheckable(true);
	recordCb_->setStatusTip( "Enable/Disable Recording");
	recordCb_->setChecked(true);
	
	tracksCbx_ = new QComboBox();
	for ( int e = 1; e < 65; e++ ) {
		tracksCbx_->addItem( QString::number( e ) );
	}
	tracksCbx_->setCurrentIndex( numberOfTracks()-1 );
	connect( tracksCbx_, SIGNAL( currentIndexChanged( int ) ),
		 this, SLOT( onTracksComboBoxIndexChanged( int ) ) );

	toolBar_->addWidget( new QLabel( "# of Tracks: ") );
	toolBar_->addWidget ( tracksCbx_ );	 
	toolBar_->addSeparator();
	toolBar_->addWidget( new QLabel( "Step: " ) );
	toolBar_->addWidget( patStepCbx_ );
	toolBar_->addSeparator();
	toolBar_->addAction( addBarAct_ );	
	toolBar_->addAction( delBarAct_ );
	toolBar_->addSeparator();
	toolBar_->addAction( recordCb_ );


	toolBar_->setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Fixed );

}

// Returns true if a note was successfully added.
bool PatternView::enterNote( const PatCursor & cursor, int note ) 
{
	if ( recordCb_->isChecked() == true)
	{
		if ( pattern() ) {
			psy::core::PatternEvent event = pattern()->event( cursor.line(), cursor.track() );
			psy::core::Machine* tmac = song_->machine( song_->seqBus );
			event.setNote( octave() * 12 + note );
			event.setSharp( false/*drawArea->sharpMode()*/ );
			if (tmac) event.setMachine( tmac->id() );
			if (tmac && tmac->type() == psy::core::MACH_SAMPLER ) {
				event.setInstrument( song_->instSelected );
			}
			pattern()->setEvent( cursor.line(), cursor.track(), event );
//        if (tmac) PlayNote( octave() * 12 + note, 127, false, tmac);   
			return true;
		}
	}

	return false;
}

void PatternView::clearNote( const PatCursor & cursor) {
	if ( pattern() ) {
		psy::core::PatternEvent event = pattern()->event( cursor.line(), cursor.track() );
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
	printf("PatternView::setPattern(%p)\n",pattern);
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

void PatternView::onPatternStepComboBoxIndexChanged( int newIndex )
{
	setPatternStep( newIndex );
}

void PatternView::onTracksComboBoxIndexChanged( int index )
{
	setNumberOfTracks( index+1 ); // +1 as combo index begins at 0.
	patDraw_->scene()->update();
}

void PatternView::keyPressEvent( QKeyEvent *event )
{
	int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );
	switch ( command ) {
		/*case Qt::Key_A:
		  {
		  float position = patternGrid()->cursor().line() / (float) beatZoom();
		  pattern()->removeBar(position);
		  patternGrid()->update();
		  break;
		  }*/ //why is this commented?
        case commands::pattern_step_dec:
		patStepCbx_->setCurrentIndex( std::max( 0, patternStep() - 1 ) );
		break;
        case commands::pattern_step_inc:
		patStepCbx_->setCurrentIndex( std::min( 16, patternStep() + 1 ) );
		break;
        default:
		event->ignore();
	}
}

void PatternView::showEvent( QShowEvent * event ) 
{
	Q_UNUSED( event );
	patDraw()->setFocus();
	patDraw()->scene()->setFocusItem( patDraw()->patternGrid() );
}
