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
#include <ngrs/nautoscrolllayout.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/napp.h>

namespace psycle { namespace host {

MachineView::MachineView( Song* song )
  : NPanel()
{
  _pSong = song;

  setLayout(NAlignLayout());

  scrollBox_ = new NScrollBox();
    scrollArea_ = new NPanel();
      scrollArea_->setLayout(NAutoScrollLayout());
      scrollArea_->setClientSizePolicy(nVertical + nHorizontal);
    scrollBox_->setScrollPane(scrollArea_);
  add(scrollBox_, nAlClient);

  init();
}


MachineView::~MachineView()
{
}

void MachineView::init( )
{
  scrollArea_->setTransparent(false);
  selectedMachine_ = 0;
	updateSkin();
}


void MachineView::onCreateMachine( Machine * mac )
{
  switch (mac->mode())
  {		
      case MACHMODE_GENERATOR: {
        MachineGUI* macGui = new GeneratorGUI(mac);
          macGui->moved.connect(this,&MachineView::onMoveMachine);
          macGui->newConnection.connect(this,&MachineView::onNewConnection);
          macGui->patternTweakSlide.connect(this,&MachineView::onTweakSlide);
          macGui->selected.connect(this,&MachineView::onMachineSelected);
        macGui->propsDlg()->updateMachineProperties.connect(this,&MachineView::onUpdateMachinePropertiesSignal);
        macGui->deleteRequest.connect(this,&MachineView::onMachineDeleteRequest);
        scrollArea_->add(macGui);
        machineGUIs.push_back(macGui);
        }
      break;
      case MACHMODE_FX: {
        MachineGUI* macGui = new EffektGUI(mac);
          macGui->moved.connect(this,&MachineView::onMoveMachine);
          macGui->newConnection.connect(this,&MachineView::onNewConnection);
          macGui->patternTweakSlide.connect(this,&MachineView::onTweakSlide);
          macGui->selected.connect(this,&MachineView::onMachineSelected);
          macGui->deleteRequest.connect(this,&MachineView::onMachineDeleteRequest);
        macGui->propsDlg()->updateMachineProperties.connect(this,&MachineView::onUpdateMachinePropertiesSignal);
        scrollArea_->add(macGui);
        machineGUIs.push_back(macGui);
      }
      break;
      case MACHMODE_MASTER: {
        MachineGUI* macGui = new MasterGUI(mac);
          macGui->newConnection.connect(this,&MachineView::onNewConnection);
          macGui->moved.connect(this,&MachineView::onMoveMachine);
          macGui->selected.connect(this,&MachineView::onMachineSelected);
          macGui->deleteRequest.connect(this,&MachineView::onMachineDeleteRequest);
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
    Machine* mac = _pSong->_pMachine[c];
    if (mac) { onCreateMachine(mac); }
  }


  // add Wires
  for(int c=0;c<MAX_MACHINES;c++)
  {
      Machine* tmac = _pSong->_pMachine[c];
      if (tmac) for (int w=0; w<MAX_CONNECTIONS; w++)
      {
        if (tmac->_connection[w]) {
              MachineGUI* from = findByMachine(tmac);
              if (from!=0) {
                Machine* pout = _pSong->_pMachine[tmac->_outputMachines[w]];
                MachineGUI* to = findByMachine(pout);
                if (to != 0) {
                  MachineWireGUI* line = new MachineWireGUI();
									wireGUIs.push_back( line );
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
	wireGUIs.clear();
  machineGUIs.clear();
  scrollArea_->removeChilds();
  createGUIMachines();
  scrollArea_->resize(); // update scroll client area
  repaint();
}

MachineGUI * MachineView::findByMachine( Machine * mac )
{
  for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
      MachineGUI* machineGUI = *it;
      if (machineGUI->pMac() == mac) return machineGUI;
  }
  return 0;
}

/**
 * Called when starting a new connection from a machine.
 */
void MachineView::onNewConnection( MachineGUI * sender )
{
  startGUI = sender;

  int midW = sender->clientWidth()  / 2;
  int midH = sender->clientHeight() / 2;

  MachineWireGUI* newline = new MachineWireGUI();
  newline->setPoints(NPoint(sender->left()+midW,sender->top()+midH),NPoint(sender->left()+midW,sender->top()+midH));
  scrollArea_->insert(newline,0);
  newline->setMoveable(NMoveable(nMvVertical | nMvHorizontal | nMvPolygonPicker));
  newline->setName("line");
  repaint();
  newline->setMoveFocus(0); 
  newline->wireMoveEnd.connect(this,&MachineView::onLineMoveEnd);
} 

/**
 * Triggered when the user starts rewiring a connection.
 */
void MachineView::onLineRewireBeginSignal(MachineWireGUI *theline, int rewireType)
{  
  startGUI = this->findByMachine(theline->dialog()->pSrcMachine());
  MachineGUI *dstGui = this->findByMachine(theline->dialog()->pDstMachine());
  
  theline->setMoveable(NMoveable(nMvVertical | nMvHorizontal | nMvPolygonPicker));
  theline->setMoveFocus(0); 
  if (rewireType & nsCtrl) {
          int midW = dstGui->clientWidth()  / 2;
          int midH = dstGui->clientHeight() / 2;
        theline->setPoints(NPoint(dstGui->left()+midW,dstGui->top()+midH),NPoint(dstGui->left()+midW,dstGui->top()+midH));
        theline->setWireState(WIRESTATE_REWIRING_SRC);
  } else {
          int midW = startGUI->clientWidth()  / 2;
          int midH = startGUI->clientHeight() / 2;
        theline->setPoints(NPoint(startGUI->left()+midW,startGUI->top()+midH),NPoint(startGUI->left()+midW,startGUI->top()+midH));
        theline->setWireState(WIRESTATE_REWIRING_DST); 
  }

  repaint();

}

/**
 * Triggered when the user stops dragging a machine connection
 * (be it a new connection or a rewiring.)
 */
void MachineView::onLineMoveEnd(MachineWireGUI *theline, const NMoveEvent & ev)
{	
  switch (theline->wireState())
  {
        case WIRESTATE_NEWCONNECTION:
        {
          bool found = false;
          for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
            MachineGUI* machineGUI = *it;
            if (machineGUI->clipBox().intersects(theline->left()+ev.x(),theline->top()+ev.y())
                && machineGUI->pMac()->AcceptsConnections()) {
                      found = true;
                      _pSong->InsertConnection(startGUI->pMac()->_macIndex , machineGUI->pMac()->_macIndex, 1.0f);
                      startGUI->attachLine(theline,0);
                      machineGUI->attachLine(theline,1);
                      theline->setMoveable(NMoveable());
                      theline->dialog()->setMachines(startGUI->pMac(),machineGUI->pMac());
                      theline->dialog()->deleteMe.connect(this,&MachineView::onWireDelete);
                      theline->rewireBegin.connect(this,&MachineView::onLineRewireBeginSignal);
                      wireGUIs.push_back(theline);
                      repaint(); 
                      break;
            } 
          }
          if (!found) {
                theline->dialog()->setName("wiredlg");
            scrollArea_->removeChild(theline);
            repaint();
          } else {
                theline->setMoveable( NMoveable() ); 
          }
        }
        break;
        case WIRESTATE_REWIRING_DST: 
        {
          MachineGUI* oldDestGUI = this->findByMachine(theline->dialog()->pDstMachine());
          bool found = false;
          for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
            MachineGUI* newDestGui = *it;
            if (newDestGui->clipBox().intersects(theline->left()+ev.x(),theline->top()+ev.y())
                && newDestGui->pMac()->AcceptsConnections()) {
              found = true;
              Machine* srcMac = startGUI->pMac();
              Machine* dstMac = newDestGui->pMac();

              // We need to remove line from wireGuis and add it again when updated.
              std::vector<MachineWireGUI*>::iterator it = wireGUIs.begin();
              it = find( wireGUIs.begin(), wireGUIs.end(), theline );
              if ( it != wireGUIs.end() ) wireGUIs.erase(it);	

              // Update the GUI.
              oldDestGUI->detachLine(theline);
              newDestGui->attachLine(theline,1);
              theline->setMoveable(NMoveable());
              theline->dialog()->setMachines(srcMac,dstMac);
              theline->rewireBegin.connect(this,&MachineView::onLineRewireBeginSignal);
              theline->setWireState(WIRESTATE_WIRED);
              wireGUIs.push_back(theline);

              // Update the connections in the song.
              int wireIndex = srcMac->FindOutputWire(oldDestGUI->pMac()->_macIndex);
              _pSong->ChangeWireDestMac(srcMac->_macIndex, dstMac->_macIndex, wireIndex);
                 
              repaint(); 
              break;
            } 
          }
          if (!found) {
              // reattach end of wire to where it was previously.
              oldDestGUI->attachLine(theline,1);
              repaint();
          } 
        }
        break;
        case WIRESTATE_REWIRING_SRC:
        {
          MachineGUI* oldSrcGui = this->findByMachine(theline->dialog()->pSrcMachine());
          MachineGUI* dstGui = this->findByMachine(theline->dialog()->pDstMachine());
          bool found = false;
          for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
            MachineGUI* newSrcGui = *it;
            if (newSrcGui->clipBox().intersects(theline->left()+ev.x(),theline->top()+ev.y())
                && newSrcGui->pMac()->EmitsConnections()) {
              found = true;
              Machine* srcMac = newSrcGui->pMac();
              Machine* dstMac = dstGui->pMac();

              // We need to remove line from wireGuis and add it again when updated.
              std::vector<MachineWireGUI*>::iterator it = wireGUIs.begin();
              it = find( wireGUIs.begin(), wireGUIs.end(), theline );
              if ( it != wireGUIs.end() ) wireGUIs.erase(it);	

              // Update the GUI.
              oldSrcGui->detachLine(theline);
              newSrcGui->attachLine(theline,0);
              dstGui->attachLine(theline,1);
              theline->setMoveable(NMoveable());
              theline->dialog()->setMachines(srcMac,dstMac);
              theline->rewireBegin.connect(this,&MachineView::onLineRewireBeginSignal);
              theline->setWireState(WIRESTATE_WIRED);
              wireGUIs.push_back(theline);

              // Update the connections in the song.
              int wireIndex = dstMac->FindInputWire(oldSrcGui->pMac()->_macIndex);
              _pSong->ChangeWireSourceMac(srcMac->_macIndex, dstMac->_macIndex, wireIndex);
                 
              repaint(); 
              break;
            } 
          }
          if (!found) {
              // reattach end of wire to where it was previously.
              oldSrcGui->attachLine(theline,1);
              repaint();
          } 
        }
        break;
  }
}

void MachineView::onWireDelete( WireDlg * dlg )
{
	Player::Instance()->lock();

  dlg->pSrcMachine()->Disconnect(*dlg->pDstMachine());
  Machine* _pSrcMachine = dlg->pSrcMachine();
  Machine* _pDstMachine = dlg->pDstMachine();

  int wireIndex = dlg->pSrcMachine()->FindOutputWire(_pDstMachine->_macIndex);
  int dstWireIndex = _pDstMachine->FindInputWire(_pSrcMachine->_macIndex);

  _pSrcMachine->_connection[wireIndex] = false;
  _pSrcMachine->_outputMachines[wireIndex] = -1;
  _pSrcMachine->_connectedOutputs--;

  _pDstMachine->_inputCon[dstWireIndex] = false;
  _pDstMachine->_inputMachines[dstWireIndex]=-1;
  _pDstMachine->_connectedInputs--;


  MachineGUI* from = this->findByMachine(dlg->pSrcMachine());
  MachineGUI* to   = this->findByMachine(dlg->pDstMachine());

  from->detachLine(dlg->line());
  to->detachLine(dlg->line());
 
  /*dlg->setVisible(false);
  dlg->line()->setVisible(false);
  NApp::flushEventQueue();
  if (window()!=0) window()->checkForRemove(0);*/
	std::vector<MachineWireGUI*>::iterator it = wireGUIs.begin();
	it = find( wireGUIs.begin(), wireGUIs.end(), dlg->line() );
	if ( it != wireGUIs.end() ) wireGUIs.erase(it);	

  scrollArea_->removeChild ( dlg->line() );

	line = 0;
        repaint();
	Player::Instance()->unlock();
}

void MachineView::removeMachines( )
{
  wireGUIs.clear();
  scrollArea_->removeChilds();
}

NPanel * MachineView::scrollArea( )
{
  return scrollArea_;
}

void MachineView::updateVUs( )
{
  for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
    MachineGUI* machineGUI = *it;
    machineGUI->repaintVUMeter();
  }
}

void MachineView::onMoveMachine( Machine * mac, int x, int y )
{
  machineMoved.emit(mac,x,y);
}

Machine * MachineView::selMachine( )
{
  if (selectedMachine_)
    return selectedMachine_->pMac();
  else
    return 0;
}

void MachineView::onTweakSlide( int machine, int command, int value )
{
  patternTweakSlide.emit(machine,command,value);
}

void MachineView::onMachineSelected( MachineGUI * gui )
{
  if (selectedMachine_ && gui != selectedMachine_) {
     selectedMachine_->setSelected(false);
     selectedMachine_->repaint();
  }
  if (gui != selectedMachine_) {
    selectedMachine_ = gui;
    gui->setSelected(true);
    gui->repaint();
    selected.emit(gui->pMac()); 
  } 
}

void MachineView::setSelectedMachine( Machine* mac)
{
  for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
    MachineGUI* machineGUI = *it;
    if ( machineGUI->pMac() == mac) {
       onMachineSelected(machineGUI);
       break;
    }
  }
}

void MachineView::onUpdateMachinePropertiesSignal(Machine *machine)
{
  int index = machine->_macIndex;
  machineNameChanged.emit(index); 
}

void MachineView::onMachineDeleteRequest( MachineGUI * machineGUI )
{
		// todo remove machine
  int index = machineGUI->pMac()->_macIndex;
	_pSong->DestroyMachine( index );
  selectedMachine_ = 0;
  update();		 
  machineDeleted.emit(index); 
}

void MachineView::updateSkin( )
{
  setColorInfo( SkinReader::Instance()->machineview_color_info() );

  for (std::vector<MachineGUI*>::iterator it = machineGUIs.begin() ; it < machineGUIs.end(); it++) {
    MachineGUI* machineGUI = *it;
    machineGUI->updateSkin();
  }

	repaint();
}

void MachineView::setColorInfo( const MachineViewColorInfo & info ) {
	colorInfo_ = info;

	scrollArea_->setBackground ( colorInfo_.pane_bg_color );
}

const MachineViewColorInfo & MachineView::colorInfo() const {
	return colorInfo_;
}
 
}
}
