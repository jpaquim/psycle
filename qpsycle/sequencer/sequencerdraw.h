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

 #include <QtGui/QGraphicsView>

#include "sequencerview.h"
#include "sequencerline.h"
#include "sequencerarea.h"

#include "psycore/song.h"

class SequencerArea;
class SequencerLine;
class SequencerItem;
class SequencerView;
class PlayLine;

 class SequencerDraw : public QGraphicsView
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
    void onSequencerLineClick( SequencerLine *line );
    void onSequencerItemDeleteRequest( SequencerItem *item );
    void onPlayLineMoved( double newXPos );

 protected:

 private:
    SequencerView *seqView_;
    QGraphicsScene *scene_;

    SequencerArea *seqArea_;

    std::vector<SequencerLine*> lines_;
    SequencerLine *lastLine_;
    SequencerLine *selectedLine_;

    int beatPxLength_;
    double newBeatPos_;
    int lineHeight_;

    PlayLine *pLine_;

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
