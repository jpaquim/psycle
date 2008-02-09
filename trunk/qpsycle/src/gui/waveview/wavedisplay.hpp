/* -*- mode:c++, indent-tabs-mode:t -*- */
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
#ifndef WAVEDISPLAY_H
#define WAVEDISPLAY_H

#include <QGraphicsView>
#include <QGraphicsRectItem>

namespace psy {
	namespace core {
		class Instrument;
	}
}

class QGraphicsScene;
class QGraphicsTextItem;

namespace qpsycle {

class InstrumentsModel;
class WaveItem;
	
class WaveDisplay : public QGraphicsView
{
Q_OBJECT
public:
	WaveDisplay(QWidget *parent, InstrumentsModel* instModel);

public:
	void resizeEvent(QResizeEvent *ev);

private:
	WaveItem *wave_;
	QGraphicsScene *scene_;

};

class WaveItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
	WaveItem(WaveDisplay *disp, InstrumentsModel *instModel, QGraphicsScene *scene);

	void paint(
		QPainter * painter,
		const QStyleOptionGraphicsItem * option,
		QWidget * widget = 0
	);

	void resize();

public slots:
	void resetInstrument();

private:
	InstrumentsModel *instrumentsModel_;
	psy::core::Instrument *inst_;
	WaveDisplay *disp_;

	QGraphicsTextItem *nodata_;
};

} // namespace qpsycle

#endif
