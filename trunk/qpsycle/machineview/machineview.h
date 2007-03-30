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

 #include <vector>

 #include "psycore/song.h"
 #include "psycore/machine.h"
 #include "psycore/patternevent.h"
 #include "psycore/pluginfinder.h"

 #include "newmachinedlg.h"
 #include "machinegui.h"
 #include "wiregui.h"

 #include <QtGui/QGraphicsView>
 #include <QtGui/QGraphicsScene>
 #include <QtGui/QGraphicsSceneMouseEvent>
 #include <QtGui/QKeyEvent>
 #include <QtGui/QMouseEvent>

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


 protected:
    void keyPressEvent(QKeyEvent *event);

    void scaleView(qreal scaleFactor);

 public slots:
    void startNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
    void closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
    void deleteConnection( WireGui *wireGui );
    void onMachineGuiChosen( MachineGui *macGui );
    MachineGui* findMachineGuiByMachineIndex( int index );
    void onDeleteMachineRequest( MachineGui *macGui );

signals:
    void machineGuiChosen( MachineGui *macGui );

private:
    MachineGui* findByMachine( psy::core::Machine *mac );
    MachineGui *theChosenOne_;

    psy::core::Song *song_;
    std::vector<MachineGui*> machineGuis;

    int octave_;
    
    QGraphicsScene *scene_;
    QGraphicsLineItem *tempLine_;

};

class MachineScene : public QGraphicsScene {
    Q_OBJECT
public:
    MachineScene( MachineView *macView );
protected:
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
signals:
    void newMachineCreated( int bus );

private:
    MachineView *macView_;
    NewMachineDlg *newMachineDlg;
    psy::core::PluginFinder pluginFinder_;
};

 #endif
