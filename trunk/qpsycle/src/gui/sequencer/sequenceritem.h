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
#ifndef SEQUENCERITEM_H
#define SEQUENCERITEM_H

namespace psy { namespace core {
class SequenceEntry;
}}

#include <QGraphicsItem>
#include <QObject>
#include <QAction>

class QKeyEvent;
class QGraphicsSceneMouseEvent;
class SequencerView;
class SequencerLine;

class SequencerItem : public QObject, public QGraphicsItem
{
	Q_OBJECT

public:
	SequencerItem();
	~SequencerItem();

	QRectF boundingRect() const;
	void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

	void setSequenceEntry( psy::core::SequenceEntry *sequenceEntry );
	psy::core::SequenceEntry *sequenceEntry(); 
	void constrainToParent();

	enum { Type = UserType + 5 };
	int type() const
	{
		// Enable the use of qgraphicsitem_cast with this item.
		return Type;
	}

protected: 
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void contextMenuEvent( QGraphicsSceneContextMenuEvent *event );
	void keyPressEvent( QKeyEvent *event );
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private slots:
	void onLoopEntryActionTriggered();
	void onDeleteEntryActionTriggered();

signals:
	void clicked(SequencerItem*);
	void deleteRequest( SequencerItem* );
	void moved( SequencerItem*, QPointF diff );
	void changedLine( SequencerItem*, int );


	private:
	psy::core::SequenceEntry *sequenceEntry_;

	QAction *deleteEntryAction_;
	QAction *loopEntryAction_;
	const QColor & QColorFromLongColor( long longCol );

	int beatPxLength_;
	};

#endif
