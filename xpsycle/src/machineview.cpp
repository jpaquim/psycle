/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
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
#include "machineview.h"
#include "song.h"
#include "machine.h"
#include "machinegui.h"
#include "global.h"
#include "plugin.h"
#include <napp.h>



MachineView::MachineView()
 : NPage()
{
  init();

  scrollBox_ = new NScrollBox();
  scrollBox_->setTransparent(true);

  scrollArea_ = new NPanel();
  scrollArea_->setTransparent(true);
  scrollArea_->setLayout(new NAutoScrollLayout);
  scrollBox_->setScrollPane(scrollArea_);
  add(scrollBox_);
  scrollArea_->setClientSizePolicy(nVertical + nHorizontal);
}


MachineView::~MachineView()
{
}

void MachineView::init( )
{
  setBackground(NColor(150,150,180));
  setTransparent(false);
}

void MachineView::resize( )
{
  scrollBox_->setPosition(0,0,clientWidth(), clientHeight());
}

void MachineView::onCreateMachine( Machine * mac )
{
  switch (mac->_mode)
  {
     case MACHMODE_GENERATOR: {
        MachineGUI* macGui = new GeneratorGUI(mac);
          macGui->newConnection.connect(this,&MachineView::onNewConnection);
        scrollArea_->add(macGui);
        machineGUIs.push_back(macGui);
        }
     break;
     case MACHMODE_FX: {
        MachineGUI* macGui = new EffektGUI(mac);
          macGui->newConnection.connect(this,&MachineView::onNewConnection);
        scrollArea_->add(macGui);
        machineGUIs.push_back(macGui);
      }
     break;
     case MACHMODE_MASTER: {
        MachineGUI* macGui = new MasterGUI(mac);
          macGui->newConnection.connect(this,&MachineView::onNewConnection);
          scrollArea_->add(macGui);
         machineGUIs.push_back(macGui);
      }
     break;
  }
}

void MachineView::addMachine( Machine * mac )
{
  onCreateMachine(mac);
}


void MachineView::onDestroyMachine( Machine * mac )
{
}

void MachineView::createGUIMachines( )
{
  // add Gui to Machine
  for(int c=0;c<MAX_MACHINES;c++)
  {
    Machine* mac = Global::pSong()->_pMachine[c];
    if (mac) { onCreateMachine(mac); }
  }


  // add Wires
  for(int c=0;c<MAX_MACHINES;c++)
  {
     Machine* tmac = Global::pSong()->_pMachine[c];
     if (tmac) for (int w=0; w<MAX_CONNECTIONS; w++)
     {
        if (tmac->_connection[w]) {
             MachineGUI* from = findByMachine(tmac);
             if (from!=0) {
               Machine* pout = Global::pSong()->_pMachine[tmac->_outputMachines[w]];
               MachineGUI* to = findByMachine(pout);
               if (to != 0) {
                 Wire* line = new Wire();
                 line->setPoints(NPoint(10,10),NPoint(100,100));
                 scrollArea_->insert(line,0);
                 from->attachLine(line,0);
                 to->attachLine(line,1);
                 line->dialog()->setMachines(tmac,pout);
                 line->dialog()->deleteMe.connect(this,&MachineView::onWireDelete);
               }
             }
        }
     }
  }
}

void MachineView::update( )
{
  machineGUIs.clear();
  scrollArea_->removeChilds();
  createGUIMachines();
}

MachineGUI * MachineView::findByMachine( Machine * mac )
{
  for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
      MachineGUI* machineGUI = *it;
      if (machineGUI->pMac() == mac) return machineGUI;
  }
  return 0;
}


void MachineView::onNewConnection( MachineGUI * sender )
{
  startGUI = sender;

  int midW = sender->clientWidth()  / 2;
  int midH = sender->clientHeight() / 2;

  line = new Wire();
  line->setPoints(NPoint(sender->left()+midW,sender->top()+midH),NPoint(sender->left()+midW,sender->top()+midH));
  scrollArea_->insert(line,0);
  line->setMoveable(NMoveable(nMvVertical | nMvHorizontal | nMvPolygonPicker));
  repaint();
  line->setMoveFocus(0);
  line->mousePressed.connect(this,&MachineView::onLineMousePressed);
}

void MachineView::onLineMousePressed( NButtonEvent * ev )
{
  bool found = false;
  for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
    MachineGUI* machineGUI = *it;
    if (machineGUI->clipBox().intersects(line->left()+ev->x(),line->top()+ev->y())) {
      Global::pSong()->InsertConnection(startGUI->pMac()->_macIndex , machineGUI->pMac()->_macIndex, 1.0f);
      startGUI->attachLine(line,0);
      machineGUI->attachLine(line,1);
      line->setMoveable(NMoveable());
      line->dialog()->setMachines(startGUI->pMac(),machineGUI->pMac());
      line->dialog()->deleteMe.connect(this,&MachineView::onWireDelete);
      found = true;
      repaint();
      break;
    }
  }
  if (!found) {
    scrollArea_->removeChild(line);
    repaint();
  }
}

void MachineView::onWireDelete( WireDlg * dlg )
{
  Machine* _pSrcMachine = dlg->pSrcMachine();
  Machine* _pDstMachine = dlg->pDstMachine();

  _pSrcMachine->_connection[dlg->pDstMachine()->_macIndex] = false;
  _pSrcMachine->_outputMachines[dlg->pDstMachine()->_macIndex] = -1;
  _pSrcMachine->_numOutputs--;

  _pDstMachine->_inputCon[dlg->pSrcMachine()->_macIndex] = false;
  _pDstMachine->_inputMachines[dlg->pSrcMachine()->_macIndex]=-1;
  _pDstMachine->_numInputs--;

  MachineGUI* from = this->findByMachine(dlg->pSrcMachine());
  MachineGUI* to   = this->findByMachine(dlg->pDstMachine());

  from->detachLine(dlg->line());
  to->detachLine(dlg->line());

  dlg->setVisible(false);
  dlg->line()->setVisible(false);
  NApp::flushEventQueue();
  if (window()!=0) window()->checkForRemove(0);
  scrollArea_->removeChild(dlg->line());
  repaint();
}

void MachineView::removeMachines( )
{
  scrollArea_->removeChilds();
}

NPanel * MachineView::scrollArea( )
{
  return scrollArea_;
}







