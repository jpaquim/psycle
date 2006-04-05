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
#include <napp.h>
#include <inttypes.h>

const std::string PSYCLE__VERSION="X";


ChildView::ChildView()
 : NTabBook()
{
  _pSong = Global::pSong();
  Global::pSong()->New();
  Global::pSong()->seqBus=0;

  setTabBarAlign(nAlBottom);
  setAlign(nAlClient);

  machineView_ = new MachineView();
  patternView_ = new PatternView();


  patternView_->setBackground(Global::pConfig()->pvc_row);
  patternView_->setForeground(Global::pConfig()->pvc_background);
  patternView_->setSeparatorColor(Global::pConfig()->pvc_separator);

  addPage(machineView_,"Machine View");
  addPage(patternView_,"Pattern View");

  setActivePage(machineView_);

  getOpenFileName_ = new NFileDialog();
    getOpenFileName_->addFilter("*.psy [psy3 song format]","!S*.psy");
  add(getOpenFileName_);

  getSaveFileName_ = new NFileDialog();
    getSaveFileName_->addFilter("*.psy [psy3 song format]","!S*.psy");
  add(getSaveFileName_);

  //enableSound();
  machineView_->createGUIMachines();

  timer.setIntervalTime(10);
  timer.timerEvent.connect(this,&ChildView::onTimer);
  timer.enableTimer();
}


ChildView::~ChildView()
{
}


void ChildView::onFileLoadSong( NObject * sender )
{
  if (getOpenFileName_->execute()) {
     NApp::flushEventQueue();
     OnFileLoadSongNamed(getOpenFileName_->fileName(),1);
  }
}

void ChildView::onFileSaveSong( NObject * sender )
{
  if (getSaveFileName_->execute()) {
     NApp::flushEventQueue();
     FileSaveSongNamed(getSaveFileName_->fileName());
  }
}

void ChildView::FileSaveSongNamed(std::string fName) {
  _pSong->save(fName);
}

void ChildView::OnFileLoadSongNamed( std::string fName, int fType )
{
  if( fType == 2 )
  {
     //FILE* hFile=fopen(fName.c_str(),"rb");
     //LoadBlock(hFile);
     //fclose(hFile);
  } else
  {
    //if (CheckUnsavedSong("Load Song"))
     {
       FileLoadSongNamed(fName);
     }
  }
}

void ChildView::FileLoadSongNamed( std::string fName )
{
  // stop player
  Global::pPlayer()->Stop();
  // delete machine gui ..
   machineView_->removeMachines();
  // load Song
  _pSong->load(fName);
  // set window title to new song name
  setTitleBarText();
  // create loaded machines
  machineView_->update();
  // emit a signal that song is loaded
  newSongLoaded.emit();
}


void ChildView::setTitleBarText( )
{
  std::string titlename = "[";

  titlename+=Global::pSong()->fileName;
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

void ChildView::playFromStart()
{
  if (Global::pConfig()->_followSong)
  {
    //bScrollDetatch=false;
  }
  patternView_->setPrevEditPosition(patternView_->editPosition());
  Global::pPlayer()->Start(0,0);
  //pParentMain->StatusBarIdle();
}

void ChildView::enableSound( )
{
  AudioDriver* pOut = Global::pConfig()->_pOutputDriver;
  if (!pOut->Initialized())
  {
     pOut->Initialize(Global::pPlayer()->Work, Global::pPlayer());
  }
  if (!pOut->Configured())
  {
     pOut->Configure();
     Global::pPlayer()->SampleRate(pOut->_samplesPerSec);
  //   _outputActive = true;
  }
  if (pOut->Enable(true))
  {
  //   _outputActive = true;
  }
}

MachineView * ChildView::machineView( )
{
  return machineView_;
}

void ChildView::onTimer( )
{
  patternView_->updatePlayBar();
}



