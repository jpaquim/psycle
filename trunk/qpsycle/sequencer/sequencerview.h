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
 #ifndef SEQUENCERVIEW_H
 #define SEQUENCERVIEW_H

 #include <QWidget>
 #include <QtGui/QGraphicsView>

#include "sequencerline.h"
#include "sequencerarea.h"

#include "psycore/song.h"

class SequencerArea;
class SequencerLine;
class SequencerItem;

 class SequencerView : public QGraphicsView
 {
     Q_OBJECT

 public:
     SequencerView( psy::core::Song *song );

     psy::core::Song* song() { return song_; }
     int beatPxLength() const;
     SequencerLine *selectedLine();

    void addPattern( psy::core::SinglePattern *pattern );

 protected:

 private:
    psy::core::Song *song_;
    QGraphicsScene *scene_;

    SequencerArea *seqArea_;

    SequencerLine *lastLine_;
    SequencerLine *selectedLine_;

    int beatPxLength_;

 };

 #endif
