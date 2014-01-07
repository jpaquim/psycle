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

namespace psycle { namespace core {
	class SequenceLine;
	class SequenceEntry; 
	class Pattern;
}}

#include <QGraphicsItem>


namespace qpsycle {

	class SequencerDraw;
	class SequencerItem;

    class SequencerLine : public QObject, public QGraphicsItem {
		Q_OBJECT
		public:
		SequencerLine( SequencerDraw *sDraw );
		~SequencerLine();

		QRectF boundingRect() const;
		void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0 );

		// don't call setSequenceLine until you have added
		// this SequencerLine to a scene
		void setSequenceLine( psycle::core::SequenceLine * line );
		psycle::core::SequenceLine *sequenceLine() const; 

		void addItem( psycle::core::Pattern* pattern );
		void insertItem( SequencerItem *item );
		void moveItemToNewLine( SequencerItem *item, SequencerLine *newLine );

		// To resolve ambiguity between QObject::children() and QGraphicsItem::children().
        QList<QGraphicsItem*> children() const { return QGraphicsItem::childItems(); }

		SequencerDraw *sDraw_; ///\todo Hrm, why is this public?

		enum { Type = UserType + 6 };
		// Enable the use of qgraphicsitem_cast with this item.
		int type() const { return Type; } 

	protected:
		void mousePressEvent( QGraphicsSceneMouseEvent *event );

    Q_SIGNALS:
		void clicked( SequencerLine* );

    private Q_SLOTS:
		void onItemClicked(SequencerItem*);

	private:
		// does not take ownership of entry.
		void addEntry( psycle::core::SequenceEntry* entry );

		// does not delete entry.
		void removeEntry(psycle::core::SequenceEntry* entry);

		psycle::core::SequenceLine *seqLine_;
		std::list<SequencerItem*> items_;
		typedef std::list<SequencerItem*>::iterator items_iterator;
	};

} // namespace qpsycle

#endif
