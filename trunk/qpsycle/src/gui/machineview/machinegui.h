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

#ifndef MACHINEGUI_H
#define MACHINEGUI_H

namespace psy { namespace core {
class Machine;
}}


#include <QGraphicsItem>
#include <QAction>
#include <QObject>
#include <QPointF>
#include <QList>

class MachineView;
class WireGui;
class MachineTweakDlg;

class MachineGui : public QObject, public QGraphicsRectItem {
Q_OBJECT
public:
	MachineGui(int left, int top, psy::core::Machine *mac, MachineView *macView);
	~MachineGui();

	void addWireGui(WireGui *wireGui);
	void setName(const QString &name);
	QPointF centrePointInSceneCoords();
	std::vector<WireGui *> wireGuiList();
	enum { Type = UserType + 1 };

	int type() const { return Type; }

	psy::core::Machine* mac();
	int noteFromCommand( int command );

	void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

	MachineView *macView() { return m_macView; }

	std::vector<WireGui *> wireGuiList_;

public slots:
	void onDeleteMachineActionTriggered();
	void onRenameMachineActionTriggered();
	void onToggleMuteActionTriggered();
	void onToggleSoloActionTriggered();
	void onCloneMachineActionTriggered();

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );

	MachineView *m_macView;
	psy::core::Machine *m_mac;

	QAction *showMacTweakDlgAct_;
	QAction *deleteMachineAct_;
	QAction *renameMachineAct_;
	QAction *cloneMachineAct_;
	QAction *toggleMuteAct_;
	QAction *toggleSoloAct_;

	QGraphicsTextItem *nameItem;
	int left_;
	int top_;


	QColor backgroundColor_;

signals:
	void startNewConnection( MachineGui *macGui, QGraphicsSceneMouseEvent *event );
	void closeNewConnection( MachineGui *macGui, QGraphicsSceneMouseEvent *event );
	void chosen( MachineGui *macGui );
	void deleteRequest( MachineGui *macGui );
	void renamed();
	void cloneRequest( MachineGui *macGui );

private:
	void initGraphics();
	void initActions();
	void initSignals();

};


#endif
