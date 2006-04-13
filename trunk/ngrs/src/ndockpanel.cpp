/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "ndockpanel.h"
#include "nalignlayout.h"
#include "nflowlayout.h"
#include "nimage.h"
#include "nbutton.h"
#include "nwindow.h"
#include "napp.h"
#include "nconfig.h"
#include "npoint.h"

/* XPM */
const char * winundock_xpm[] = {
"16 13 2 1",
" 	c None",
".	c #003063",
"................",
".              .",
".              .",
"................",
".              .",
".              .",
".              .",
".          ... .",
".           .. .",
".          . . .",
".         .    .",
".              .",
"................"};


const char * windock_xpm[] = {
"16 13 2 1",
" 	c None",
".	c #003063",
"................",
".              .",
".              .",
"................",
".              .",
".              .",
".              .",
".            . .",
".         . .  .",
".         ..   .",
".         ...  .",
".              .",
"................"};


NDockPanel::NDockPanel()
 : NPanel()
{
  init();
}

NDockPanel::NDockPanel( NVisualComponent * clientComponent )
{
  init();
  pane()->add(clientComponent, nAlClient);
}

void NDockPanel::init( )
{
  // an alignLayout for top : dochBar and client the pane
  setLayout( alignLayout = new NAlignLayout() );

  // create header with the button to dock and undock the pane

  dockBar_ = new NPanel();
    dockBar_->setLayout( flowLayout = new NFlowLayout(nAlRight,0,0));
    // create the dockBar icon bitmaps
    dockBmp.createFromXpmData(windock_xpm);
    undockBmp.createFromXpmData(winundock_xpm);
    dockImg = new NImage();
      dockImg->setSharedBitmap(&undockBmp);
      dockImg->setPreferredSize(20,17);
    NButton* unCoupleBtn = new NButton(dockImg);
      unCoupleBtn->clicked.connect(this,&NDockPanel::onUndockWindow);
    dockBar_->add(unCoupleBtn);
    dockBar_->setBackground(NColor(210,210,210));
    dockBar_->setTransparent(false);
  add(dockBar_,nAlTop);

  // create the pane

  area_ = new NPanel();
    // set as area btw pane default NAlignLayout
    area_->setLayout( alignLayoutArea = new NAlignLayout());
  add(area_,nAlClient);

  // start state = docked
  undockedWindow = 0;
}

NDockPanel::~NDockPanel()
{
  delete alignLayout;
  delete alignLayoutArea;
  delete flowLayout;
}


NPanel * NDockPanel::pane( )
{
  return area_;
}



void NDockPanel::onUndockWindow( NButtonEvent * ev )
{
  if (undockedWindow!=0) {
    dockWindow();
    NApp::addRemovePipe(undockedWindow);
    undockedWindow=0;
  } else {

    add ( undockedWindow = new NWindow() );
    undockedWindow->setDock(this);
    NPoint newWinSize(area_->width(),area_->height() + dockBar_->height());

    erase(area_);
    erase(dockBar_);

    undockedWindow->pane()->add(dockBar_,nAlTop);
    undockedWindow->pane()->add(area_,nAlClient);

    dockImg->setSharedBitmap(&dockBmp);

    undockedWindow->setPosition(0,0,newWinSize.x(),newWinSize.y());
    undockedWindow->setVisible(true);

    window()->resize();
    window()->repaint(window()->pane(),NRect(0,0,window()->width(),window()->height()));
    window()->checkForRemove(0);
  }
}

void NDockPanel::onDockWindow( )
{
  dockWindow();
  delete undockedWindow;
  undockedWindow = 0;
}


void NDockPanel::dockWindow( )
{
  undockedWindow->setVisible(false);
  NApp::lastOverWin_ = 0;

  undockedWindow->pane()->erase(dockBar_);
  undockedWindow->pane()->erase(area_);

  NPanel::add(dockBar_,nAlTop);
  NPanel::add(area_,nAlClient);

  dockImg->setSharedBitmap(&undockBmp);

  window()->resize();
  window()->repaint(window()->pane(),NRect(0,0,window()->width(),window()->height()));
  window()->checkForRemove(0);
}








