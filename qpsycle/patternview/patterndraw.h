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
 #ifndef PATTERNDRAW_H
 #define PATTERNDRAW_H

#include "psycore/song.h"
#include "psycore/singlepattern.h"

#include <QGraphicsView>
#include <QGraphicsScene>

class QToolBar;
class QComboBox;
class QGridLayout;
class QAction;

class LineNumberColumn;
class PatternGrid;
class PatCursor;
class PatternView;
class PatternDraw;
class TrackHeader;

/**
 * TrackGeometry.
 */
class TrackGeometry {
public:			
    TrackGeometry();

    TrackGeometry( PatternDraw & patternDraw );

    ~TrackGeometry();

    void setLeft( int left );
    int left() const;

    void setWidth( int width );
    int width() const;			

    void setVisibleColumns( int cols );
    int visibleColumns() const;

    void setVisible( bool on);
    bool visible() const;

private:

    PatternDraw *pDraw;
    int left_;
    int width_;
    int visibleColumns_;
    bool visible_;

};


class PatternDraw : public QGraphicsView
{
   Q_OBJECT

public:
    PatternDraw( PatternView *patView );

    PatternView *patternView() { return patView_; }
    PatternGrid *patternGrid() { return patGrid_; }

    const std::map<int, TrackGeometry> & trackGeometrics() const;

    void setupTrackGeometrics( int numberOfTracks );
    void alignTracks();

    int gridWidthByTrack( int track ) const;
    int findTrackByXPos( int x ) const;
    int xOffByTrack( int track ) const;
    int xEndByTrack( int track ) const;
    int trackWidthByTrack( int track ) const; 
    TrackGeometry findTrackGeomByTrackNum( int trackNum );
    int trackPaddingLeft() const { return 5; }
    int trackPaddingRight() const { return 5; }

protected:
    void scrollContentsBy ( int dx, int dy );

private:
   std::map<int, TrackGeometry> trackGeometryMap;

   QGraphicsScene *scene_;
   PatternView *patView_;

   LineNumberColumn *lineNumCol_;
   TrackHeader *trackHeader_;
   PatternGrid *patGrid_;
   psy::core::SinglePattern *pattern_;
};

#endif
