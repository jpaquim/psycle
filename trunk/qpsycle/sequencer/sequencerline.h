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
#ifndef SEQUENCERLINE_H
#define SEQUENCERLINE_H

#include "psycore/patternsequence.h"
#include "sequenceritem.h"
#include "sequencerdraw.h"

#include <QGraphicsItem>
#include <QObject>

class SequencerDraw;
class SequencerItem;

class SequencerLine : public QObject, public QGraphicsRectItem, public boost::signalslib::trackable
{
    Q_OBJECT
public:
    SequencerLine( SequencerDraw *sDraw );
    ~SequencerLine();

    // don't call setSequenceLine until you have added
    // this SequencerLine to a scene
    void setSequenceLine( psy::core::SequenceLine * line );
    psy::core::SequenceLine *sequenceLine(); 

    void addItem( psy::core::SinglePattern* pattern );

    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    SequencerDraw *sDraw_;

    enum { Type = UserType + 6 };
    int type() const  // Enable the use of qgraphicsitem_cast with this item.
    {

        return Type;
    }

signals:
    void clicked( SequencerLine* );

private slots:
    void onItemClicked(SequencerItem*);

private:
    // does not take ownership of entry.
    void addEntry( psy::core::SequenceEntry* entry );

    // does not delete entry.
    void removeEntry(psy::core::SequenceEntry* entry);

    psy::core::SequenceLine *seqLine_;
    std::list<SequencerItem*> items_;
    typedef std::list<SequencerItem*>::iterator items_iterator;
};

#endif
