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
#ifndef PATTERNVIEW_H
#define PATTERNVIEW_H

class QVBoxLayout;
#include <QWidget>
#include <QCheckBox>


namespace psy { namespace core {
class Song;
class SinglePattern;
}}

class QToolBar;
class QComboBox;
class QAction;

namespace qpsycle {

class LineNumberColumn;
class PatternDraw;
class PatternGrid;
class PatCursor;
/*!
 * \brief
 * Tab object for the Pattern View.
 * 
 * PatternView is the object that holds the whole pattern view (the tab).
 */


class PatternView : public QWidget {
Q_OBJECT

public:
	PatternView( psy::core::Song *song );
	~PatternView();

	//FIXME: These three functions should be placed inside patternGrid, and have a signal call for view update
	bool enterNote( const PatCursor & cursor, int note );
	bool enterNoteOff( const PatCursor & cursor );
	void clearNote( const PatCursor & cursor);
	// Called from mainwindow's periodical update to refresh playback position
	void onTick( double offsetPos );


	// Getters.
	psy::core::Song *song() { return song_; }
	psy::core::SinglePattern *pattern() const { return pattern_; }
	PatternDraw* patDraw() { return patDraw_; }
	PatternGrid* patternGrid(); 

	int numberOfLines() const;
	int numberOfTracks() const;
	int selectedMachineIndex() const;
	int playPos() { return playPos_; }
	int beatZoom() const;
	int patternStep() const;
	int octave() const;

	// Setters.
	void setSelectedMachineIndex( int idx );
	void setPattern( psy::core::SinglePattern *pattern );
	void setNumberOfTracks( int numTracks );
	void setPatternStep( int newStep );
	void setOctave( int newOctave );

	// GUI events.
	void keyPressEvent( QKeyEvent *event );

public slots:
	void onPatternStepComboBoxIndexChanged( int newIndex );
	void onTracksComboBoxIndexChanged( int index );
	void onZoomComboBoxIndexChanged( int );
protected:
	void showEvent( QShowEvent * event );

private:
	void createToolBar();

	psy::core::Song *song_;
	psy::core::SinglePattern* pattern_;

	// Settings.
	int patternStep_;
	int numberOfTracks_; 
	int selectedMacIdx_;
	int playPos_;
	int octave_;
	// GUI items.
	PatternDraw *patDraw_;

	QVBoxLayout *layout_;
	QToolBar *toolBar_;
	QComboBox *patStepCbx_;
	QAction *addBarAct_;
	QAction *delBarAct_;
	QAction *recordCb_;
	QComboBox *tracksCbx_;
	QComboBox *zoomCbx_;
};

} // namespace qpsycle

#endif
