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
 #ifndef SEQUENCERDRAW_H
 #define SEQUENCERDRAW_H

#include "psycore/song.h"

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsRectItem>

class SequencerView;
class SequencerArea;
class SequencerLine;
class SequencerItem;
class PlayLine;

class QGraphicsSceneMouseEvent;

class SequencerDraw : public QGraphicsView, public boost::signalslib::trackable
 {
     Q_OBJECT

 public:
     SequencerDraw( SequencerView *seqView );

     SequencerView *sequencerView() { return seqView_; }
     int beatPxLength() const;
     SequencerLine *selectedLine();
    void setSelectedLine( SequencerLine *line );
    std::vector<SequencerLine*> lines();

    void addPattern( psy::core::SinglePattern *pattern );
    PlayLine *pLine() { return pLine_; }

public slots:
    void insertTrack();
    void deleteTrack();
    void moveUpTrack();
    void moveDownTrack();
    void onSequencerLineClick( SequencerLine *line );
    void onSequencerItemDeleteRequest( SequencerItem *item );
    void onPlayLineMoved( double newXPos );

 protected:

 private:
    SequencerView *seqView_;
    QGraphicsScene *scene_;

    SequencerArea *seqArea_;

    std::vector<SequencerLine*> lines_;
    typedef std::vector<SequencerLine*>::iterator lines_iterator;
    SequencerLine *selectedLine_;

    int beatPxLength_;
    double newBeatPos_;
    int lineHeight_;

    PlayLine *pLine_;

    SequencerLine* makeSequencerLine(psy::core::SequenceLine* seqLine);

    void onNewLineCreated(psy::core::SequenceLine* seqLine);
    void onNewLineInserted(psy::core::SequenceLine* newSeqLine, psy::core::SequenceLine* position);
    void onLineRemoved(psy::core::SequenceLine* seqLine);
    void onLinesSwapped(psy::core::SequenceLine* a, psy::core::SequenceLine* b);
 };

class PlayLine : public QObject, public QGraphicsRectItem {
    Q_OBJECT
public:
    PlayLine(); 

protected:
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );

signals:
    void playLineMoved( double );
    

};

 #endif
