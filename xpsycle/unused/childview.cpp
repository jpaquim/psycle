/***************************************************************************
  *   Copyright (C) 2006 by  Stefan   *
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
#include "sequencergui.h"
#include "sequencerbar.h"
#include "virtualpattern.h"
#include <ngrs/app.h>
#include <ngrs/dockpanel.h>
#include <ngrs/splitbar.h>

namespace psycle {
	namespace host {

const std::string PSYCLE__VERSION="X";


ChildView::ChildView()
  : ngrs::Panel()
{
  _pSong = new Song();

  setLayout( ngrs::AlignLayout() );

  add(sequencerBar_ = new SequencerBar(), ngrs::nAlLeft);
  sequencerBar_->setChildView(this);

  tabBook_ = new ngrs::TabBook();
  add(tabBook_, ngrs::nAlClient);

  tabBook_->setTabBarAlign(ngrs::nAlBottom);
  setAlign(ngrs::nAlClient);

  machineView_ = new MachineView( *_pSong );
    machineView_->scrollArea()->mouseDoublePress.connect(this,&ChildView::onMachineViewDblClick);
    machineView_->selected.connect(this,&ChildView::onMachineSelected);
    machineView_->patternTweakSlide.connect(this, &ChildView::onTweakSlide);

  patternView_ = new PatternView( _pSong );
	sequencerBar_->setPatternView( patternView() );

  sequencerView_ = new SequencerGUI();
  sequencerBar_->setSequenceGUI( sequencerView() ) ;
  sequencerView_->setPatternSequence( _pSong->patternSequence());
  sequencerView_->addSequencerLine();

	virtualPattern_ = new VirtualPattern();

  
  macDock = new ngrs::DockPanel(machineView_);
  tabBook_->addPage(macDock,"Machine View");
  ngrs::NTab* tab = tabBook_->tab( macDock );
  tab->click.connect(this,&ChildView::onTabChange);
  patDock = new ngrs::DockPanel(patternView_);
  tabBook_->addPage(patDock,"Pattern View");
  tab = tabBook_->tab( patDock );
  tab->click.connect(this,&ChildView::onTabChange);

  ngrs::Panel* seqGroup = new ngrs::Panel();
		seqGroup->setLayout( ngrs::AlignLayout() );
    seqGroup->add( virtualPattern_, ngrs::nAlBottom);
    ngrs::SplitBar* splitBar = new ngrs::SplitBar();
		splitBar->setOrientation( ngrs::nHorizontal );
    seqGroup->add( splitBar, ngrs::nAlBottom );
    seqGroup->add( sequencerView_, ngrs::nAlClient);
	seqDock = new ngrs::DockPanel(seqGroup);
  tabBook_->addPage(seqDock,"Sequencer View");
  tab = tabBook_->tab( seqDock );
  tab->click.connect(this,&ChildView::onTabChange);

  tabBook_->setActivePage(macDock);

  machineView_->createGUIMachines();

  //timer.setIntervalTime(80);
  //timer.enableTimer();
  _pSong->patternSequence()->patternData()->resetToDefault();
  sequencerBar_->update();
}


ChildView::~ChildView()
{
  delete _pSong;
}
  
void ChildView::setTitleBarText( )
{
}

void ChildView::onPatternView( ngrs::Object * sender )
{
}

PatternView * ChildView::patternView( )
{
  return patternView_;
}

VirtualPattern* ChildView::virtualPattern()
{
  return virtualPattern_;
}

void ChildView::play( )
{
}

void ChildView::playFromStart()
{
}

void ChildView::stop( )
{
}

MachineView * ChildView::machineView( )
{
  return machineView_;
}

void ChildView::onMachineSelected( Machine* mac ) {
   machineSelected.emit( mac );
}

void ChildView::onMachineViewDblClick( ngrs::ButtonEvent * ev )
{
  machineViewDblClick.emit( ev );
}

void psycle::host::ChildView::onTweakSlide( int machine, int command, int value)
{
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
  patternView_->setFocus();
  repaint();
}

void psycle::host::ChildView::update( )
{
  sequencerBar_->update();
  sequencerView()->update();
  machineView()->update();
}


void psycle::host::ChildView::showWaveView( )
{
  tabBook_->setActivePage(2);
  repaint();
}


void psycle::host::ChildView::showSequencerView( )
{
  tabBook_->setActivePage(3);
  repaint();
}

void psycle::host::ChildView::onTabChange( ngrs::ButtonEvent * ev )
{
  if (tabBook_->activePage() == patDock ) {
    patternView()->setFocus();
	}
}
