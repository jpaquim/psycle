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

namespace psycle { namespace core {
	class Song;
	class SequenceLine;
	class Pattern;
}}

#include <psycle/core/signalslib.h>

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsRectItem>

class QGraphicsSceneMouseEvent;

namespace qpsycle {

class SequencerView;
class SequencerArea;
class SequencerLine;
class SequencerItem;
class PlayLine;
class BeatRuler;


class SequencerDraw : public QGraphicsView, public boost::signalslib::trackable
{
	Q_OBJECT

public:
	SequencerDraw( SequencerView *seqView );

	void addPattern( psycle::core::Pattern *pattern );

	SequencerView *sequencerView() const { return seqView_; }
	PlayLine *pLine() const { return pLine_; }

	int beatPxLength() const;
	void setBeatPxLength( int beatPxLength );

	int lineHeight() const { return lineHeight_; }
	bool gridSnap() const;
	SequencerLine *selectedLine() const;
	void setSelectedLine( SequencerLine *line );
	std::vector<SequencerLine*> lines() const;

public slots:
	void insertTrack();
	void deleteTrack();
	void moveUpTrack();
	void moveDownTrack();
	void onCollapseButtonCliked();
	void onExpandButtonCliked();
	void onSequencerLineClick( SequencerLine *line );
	void onSequencerItemDeleteRequest( SequencerItem *item );
	void onPlayLineMoved( double newXPos );
	void onItemMoved( SequencerItem* item, QPointF diff );
	void onItemChangedLine( SequencerItem *item, int direction );
	void onNewPatternCreated( psycle::core::Pattern *newPattern );

signals:
	void newPatternCreated( psycle::core::Pattern * );

protected:
	virtual void keyPressEvent( QKeyEvent *event );
	void drawBackground( QPainter * painter, const QRectF & rect );
	void scrollContentsBy ( int dx, int dy );

private:
	SequencerView *seqView_;
	QGraphicsScene *scene_;
	SequencerArea *seqArea_;
	PlayLine *pLine_;

	std::vector<SequencerLine*> lines_;
	typedef std::vector<SequencerLine*>::iterator lines_iterator;

	BeatRuler *beatRuler_;
	SequencerLine *selectedLine_;

	int beatPxLength_;
	double newBeatPos_;
	int lineHeight_;

	SequencerLine* makeSequencerLine(psycle::core::SequenceLine* seqLine);

	void onNewLineCreated(psycle::core::SequenceLine* seqLine);
	void onNewLineInserted(psycle::core::SequenceLine* newSeqLine, psycle::core::SequenceLine* position);
	void onLineRemoved(psycle::core::SequenceLine* seqLine);
	void onLinesSwapped(psycle::core::SequenceLine* a, psycle::core::SequenceLine* b);

	QList<SequencerItem*> copyBuffer_;
};



class PlayLine : public QObject, public QGraphicsItem {
Q_OBJECT
public:
	PlayLine( QGraphicsView *seqDraw ); 
	QRectF boundingRect() const;

protected:
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

signals:
	void playLineMoved( double );

private:
	QGraphicsView *m_seqDraw;
	

};

} // namespace qpsycle

#endif
