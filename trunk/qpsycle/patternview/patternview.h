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
#ifndef PATTERNVIEW_H
#define PATTERNVIEW_H

#include <QVBoxLayout>

#include "linenumbercolumn.h"
#include "patterndraw.h"
#include "patterngrid.h"
#include "psycore/song.h"
#include "psycore/singlepattern.h"

class QToolBar;
class QComboBox;
class QAction;

class LineNumberColumn;
class PatternDraw;
class PatternGrid;
class PatCursor;

class PatternView : public QWidget
{
    Q_OBJECT

public:
    PatternView( psy::core::Song *song );

    void enterNote( const PatCursor & cursor, int note );
    void onTick( double sequenceStart );


    // Getters.
    psy::core::Song *song() { return song_; }
    psy::core::SinglePattern *pattern() const { return pattern_; }
    PatternDraw* patDraw() { return patDraw_; }
    PatternGrid* patternGrid(); 
    int rowHeight() const;
    int numberOfLines() const;
    int numberOfTracks() const;
    int trackWidth() const;
    int selectedMachineIndex() const;
    int playPos() { return playPos_; }
    int beatZoom() const;

    // Setters.
    void setSelectedMachineIndex( int idx );
    void setPattern( psy::core::SinglePattern *pattern );
    void setNumberOfTracks( int numTracks );

    // GUI events.
    void keyPressEvent( QKeyEvent *event );

private:
    void createToolBar();

    psy::core::Song *song_;
    psy::core::SinglePattern* pattern_;

    int numberOfTracks_;
    int selectedMacIdx_;
    int playPos_;

    PatternDraw *patDraw_;
    QVBoxLayout *layout;
    QToolBar *toolBar_;
    QComboBox *meterCbx_;
    QComboBox *patternCbx_;
    QAction *delBarAct_;

};

#endif
