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

#include "psycore/pluginfinder.h"

#include <vector>

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

class NewMachineDlg;
class MachineGui;
class WireGui;

 class MachineView : public QGraphicsView
 {
     Q_OBJECT

 public:
     MachineView( psy::core::Song *song_ );

     void connectMachines(MachineGui *srcMacGui, MachineGui *dstMacGui );
    void PlayNote( int note, int velocity, bool bTranspose, psy::core::Machine*pMachine);
    void StopNote( int note, bool bTranspose=true, psy::core::Machine* pMachine=NULL);
    psy::core::Song *song();
    void setSong( psy::core::Song *song ) { song_ = song; }
    void setTheChosenOne( MachineGui* macGui ) { theChosenOne_ = macGui; }
    MachineGui* theChosenOne() { return theChosenOne_; }
    int octave() const;
    void setOctave( int newOctave );
    void createMachineGui( psy::core::Machine *mac );
    WireGui* createWireGui( MachineGui *srcMacGui, MachineGui *dstMacGui );

    bool isCreatingWire() { return creatingWire_; }


protected:
    void keyPressEvent(QKeyEvent *event);

    void scaleView(qreal scaleFactor);

public slots:
    void startNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
    void closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
    void deleteConnection( WireGui *wireGui );
//    void startRewiringDest( WireGui *wireGui );
    void onMachineGuiChosen( MachineGui *macGui );
    MachineGui* findMachineGuiByMachineIndex( int index );
    void onDeleteMachineRequest( MachineGui *macGui );
    void onMachineRenamed();

signals:
    void machineGuiChosen( MachineGui *macGui );
    void machineDeleted( int macIndex );
    void machineRenamed();

private:
    MachineGui* findByMachine( psy::core::Machine *mac );
    MachineGui *theChosenOne_;

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
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
signals:
    void newMachineCreated( psy::core::Machine* mac );

private:
    MachineView *macView_;
    NewMachineDlg *newMachineDlg;
    psy::core::PluginFinder pluginFinder_;
};

 #endif
