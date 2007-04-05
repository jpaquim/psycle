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

#include "psycore/machine.h"

#include "machineview.h"
#include "wiregui.h"
#include "machinetweakdlg.h"

#include <QGraphicsItem>
#include <QAction>
#include <QObject>
#include <QPointF>

class MachineView;
class WireGui;

class MachineGui : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    MachineGui(int left, int top, psy::core::Machine *mac, MachineView *macView);
    void addWireGui(WireGui *wireGui);
    void setName(const QString &name);
    QPointF centrePointInSceneCoords();
    QList<WireGui *> wireGuiList();
    enum { Type = UserType + 1 };

    int type() const { return Type; }

    psy::core::Machine* mac();

    void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

public slots:
    void showMacTwkDlg();
    void onDeleteMachineActionTriggered();
    void onRenameMachineActionTriggered();
    void onToggleMuteActionTriggered();
    void onToggleSoloActionTriggered();
    void onToggleBypassActionTriggered(); // FIXME: this should be in EffectGui,
                                          // but Qt fails to find it there.

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );

    MachineView *machineView;
    psy::core::Machine *mac_;

    MachineTweakDlg *macTwkDlg_;

    QAction *showMacTwkDlgAct_;
    QAction *deleteMachineAct_;
    QAction *renameMachineAct_;
    QAction *toggleMuteAct_;
    QAction *toggleSoloAct_;

    QGraphicsTextItem *nameItem;
    int left_;
    int top_;
    QList<WireGui *> wireGuiList_;

    QColor backgroundColor_;

signals:
    void startNewConnection( MachineGui *macGui, QGraphicsSceneMouseEvent *event );
    void closeNewConnection( MachineGui *macGui, QGraphicsSceneMouseEvent *event );
    void chosen( MachineGui *macGui );
    void deleteRequest( MachineGui *macGui );
    void renamed();

private:

};

class GeneratorGui : public MachineGui {
public:
    GeneratorGui( int left, int top, psy::core::Machine *mac, MachineView *macView );
    void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

protected:
    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event );
    void keyPressEvent( QKeyEvent * event );
};

#endif
