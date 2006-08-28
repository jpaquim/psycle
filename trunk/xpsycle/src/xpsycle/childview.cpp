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
#include "childview.h"
#include "configuration.h"
#include "player.h"
#include "newmachine.h"
#include "waveedframe.h"
#include "sequencergui.h"
#include "sequencerbar.h"
#include <ngrs/napp.h>
#include <inttypes.h>
#include <ngrs/ndockpanel.h>

namespace psycle {
	namespace host {

const std::string PSYCLE__VERSION="X";


ChildView::ChildView()
  : NPanel()
{
  _pSong = new Song();

  setLayout( NAlignLayout() );

  add(sequencerBar_ = new SequencerBar(), nAlLeft);

  tabBook_ = new NTabBook();
  add(tabBook_, nAlClient);

  tabBook_->setTabBarAlign(nAlBottom);
  setAlign(nAlClient);

  machineView_ = new MachineView( _pSong );
    machineView_->scrollArea()->mouseDoublePress.connect(this,&ChildView::onMachineViewDblClick);
    machineView_->selected.connect(this,&ChildView::onMachineSelected);
    machineView_->patternTweakSlide.connect(this, &ChildView::onTweakSlide);

  patternView_ = new PatternView( _pSong );
    sequencerBar_->setPatternView( patternView() );
    patternView_->setForeground(Global::pConfig()->pvc_background);
    patternView_->setSeparatorColor(Global::pConfig()->pvc_separator);

  sequencerView_ = new SequencerGUI();
  sequencerBar_->setSequenceGUI( sequencerView() ) ;
  sequencerView_->setPatternSequence( _pSong->patternSequence());
  sequencerView_->addSequencerLine();

  
  NDockPanel* macDock = new NDockPanel(machineView_);
  tabBook_->addPage(macDock,"Machine View");
  NDockPanel* patDock = new NDockPanel(patternView_);
  tabBook_->addPage(patDock,"Pattern View");
  waveEd_ = new WaveEdFrame( song() );
  tabBook_->addPage(waveEd_,"WaveEditor");
  NDockPanel* seqDock = new NDockPanel(sequencerView_);
  tabBook_->addPage(seqDock,"Sequencer View");

  tabBook_->setActivePage(macDock);

  machineView_->createGUIMachines();

  //timer.setIntervalTime(80);
  //timer.enableTimer();

}


ChildView::~ChildView()
{
  delete _pSong;
}
  
void ChildView::setTitleBarText( )
{
  std::string titlename = "[";

  titlename += _pSong->fileName;
  /*if(pUndoList) {
      if (UndoSaved != pUndoList->counter) titlename+=" *"; else
      if (UndoMacSaved != UndoMacCounter)  titlename+=" *"; else
      if (UndoSaved != 0) titlename+=" *";
  }*/
  titlename += "] Psycle Modular Music Creation Studio (" + PSYCLE__VERSION + ")";
  window()->setTitle(titlename);
}

void ChildView::onPatternView( NObject * sender )
{

}

PatternView * ChildView::patternView( )
{
  return patternView_;
}


void ChildView::play( )
{
  patternView_->setPrevEditPosition(patternView_->editPosition());
  Player::Instance()->Start(0.0);
}


void ChildView::playFromStart()
{
  if (Global::pConfig()->_followSong)
  {
    //bScrollDetatch=false;
  }
  patternView_->setPrevEditPosition(patternView_->editPosition());
  Player::Instance()->Start(0.0);
  //pParentMain->StatusBarIdle();
}


MachineView * ChildView::machineView( )
{
  return machineView_;
}

void ChildView::onMachineSelected( Machine* mac ) {
   machineSelected.emit( mac );
}

void ChildView::onMachineViewDblClick( NButtonEvent * ev )
{
  machineViewDblClick.emit( ev );
}

WaveEdFrame * ChildView::waveEditor( )
{
  return waveEd_;
}

void psycle::host::ChildView::onTweakSlide( int machine, int command, int value)
{
  _pSong->patternTweakSlide( machine,command,value, patternView_->editPosition(), patternView_->cursor().x(),patternView_->cursor().y());
}

SequencerGUI * ChildView::sequencerView( )
{
  return sequencerView_;
}

Song * ChildView::song( )
{
  return _pSong;
}

SequencerBar * ChildView::sequencerBar( )
{
  return sequencerBar_;
}


}
}

void psycle::host::ChildView::showMachineView( )
{
  tabBook_->setActivePage(0);
  repaint();
}

void psycle::host::ChildView::showPatternView( )
{
  tabBook_->setActivePage(1);
  repaint();
}

void psycle::host::ChildView::update( )
{
  waveEditor()->Notify();
  sequencerBar_->update();
  sequencerView()->update();
  machineView()->update();
}

void psycle::host::ChildView::showSequencerView( )
{
  tabBook_->setActivePage(3);
  repaint();
}
