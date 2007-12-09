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
#ifndef MACHINEVIEW_H
#define MACHINEVIEW_H

namespace psy { namespace core {
	class Song;
	class Machine;
	class PatternEvent;
}}

#include <psycle/core/constants.h>
#include <psycle/core/pluginfinder.h>

#include <vector>

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

class NewMachineDlg;
class MachineGui;
class WireGui;

class MachineView : public QGraphicsView {
Q_OBJECT

public:
	MachineView( psy::core::Song *song_ );

	void addNewMachineGui( psy::core::Machine *mac );

	MachineGui* findMachineGuiByCoreMachineIndex( int index );
	MachineGui* findMachineGuiByCoreMachine( psy::core::Machine *mac );
	MachineGui *machineGuiAtPoint( QPointF point );

	void playNote( int note, int velocity, bool bTranspose, psy::core::Machine*pMachine);
	void stopNote( int note, bool bTranspose=true, psy::core::Machine* pMachine=NULL);
public slots:
	void onNotePress( int note, psy::core::Machine* mac );
	void onNoteRelease( int note, psy::core::Machine* mac );

public:
	psy::core::Song *song();
	void setSong( psy::core::Song *song ) { song_ = song; }
	void setChosenMachine( MachineGui* macGui );
	MachineGui* chosenMachine() { return chosenMachine_; }
	int octave() const;
	void setOctave( int newOctave );
	bool isCreatingWire() { return creatingWire_; }

	int noteFromCommand( int command );

protected:
	void keyPressEvent(QKeyEvent *event);
	void scaleView(qreal scaleFactor);

public slots:
	void startNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
	void closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
	void deleteConnection( WireGui *wireGui );
	void onMachineChosen( MachineGui *macGui );
	void onDeleteMachineRequest( MachineGui *macGui );
	void onMachineRenamed();
	void cloneMachine( MachineGui *macGui );

signals:
	void machineChosen( MachineGui *macGui );
	void machineDeleted( int macIndex );
	void machineRenamed();
	void newMachineCreated( psy::core::Machine* mac );

private:
	void createMachineGuis();
	void createWireGuis();
	void createTempLine();
	void initKeyjazzSettings();

	void connectMachines(MachineGui *srcMacGui, MachineGui *dstMacGui );
	WireGui* createWireGui( MachineGui *srcMacGui, MachineGui *dstMacGui );
	MachineGui * createMachineGui( psy::core::Machine *mac );

	MachineGui *chosenMachine_;

	psy::core::Song *song_;
	std::vector<MachineGui*> machineGuis;

	int octave_;
	
	QGraphicsScene *scene_;
	QGraphicsLineItem *tempLine_;

	bool creatingWire_;
	
	/// For multi-key playback state.
	int notetrack[psy::core::MAX_TRACKS];
	int outtrack;
};


class MachineScene : public QGraphicsScene {
Q_OBJECT
public:
	MachineScene( MachineView *macView );
protected:
	void keyPressEvent( QKeyEvent *event );
	void keyReleaseEvent( QKeyEvent *event );
	void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
signals:

private:
	MachineView *macView_;
	NewMachineDlg *newMachineDlg;
	psy::core::PluginFinder pluginFinder_;
};

#endif
