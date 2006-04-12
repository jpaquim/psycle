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
  alignLayout = new NAlignLayout();
  NPanel::setLayout(alignLayout);

  dockBar_ = new NPanel();
    dockBar_->setLayout( flowLayout = new NFlowLayout(nAlRight,0,0));
  NPanel::add(dockBar_,nAlTop);

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


  area_ = new NPanel();

  NPanel::add(area_,nAlClient);
}


NDockPanel::~NDockPanel()
{
  delete alignLayout;
  delete flowLayout;
}

void NDockPanel::add( NVisualComponent * comp )
{
  area_->add(comp);
}

void NDockPanel::onUndockWindow( NButtonEvent * ev )
{
  if (undockedWindow!=0) {
    undockedWindow->setVisible(false);
    NApp::lastOverWin_ = 0;
    onDockWindow();
  } else {
    undockedWindow = new NWindow();
    NPanel::add(undockedWindow);

    undockedWindow->setDock(this);

    int undockWinWidth  = area_->width();
    int undockWinHeight = area_->height() + dockBar_->height();

    erase(area_);
    erase(dockBar_);

    undockedWindow->pane()->add(dockBar_,nAlTop);
    undockedWindow->pane()->add(area_,nAlClient);

    dockImg->setSharedBitmap(&dockBmp);

    window()->resize();
    window()->repaint(window()->pane(),NRect(0,0,window()->width(),window()->height()));

    undockedWindow->setPosition(0,0,undockWinWidth,undockWinHeight);
    undockedWindow->setVisible(true);
  }
}

void NDockPanel::onDockWindow( )
{
  undockedWindow->pane()->erase(dockBar_);
  undockedWindow->pane()->erase(area_);

  NPanel::add(dockBar_,nAlTop);
  NPanel::add(area_,nAlClient);

  delete undockedWindow;
  undockedWindow = 0;

  window()->resize();
  window()->repaint(window()->pane(),NRect(0,0,window()->width(),window()->height()));
  window()->checkForRemove(0);
}

void NDockPanel::setLayout( NLayout * layout )
{
  area_->setLayout(layout);
}

void NDockPanel::setFont( const NFont & font )
{
  area_->setFont(font);
}

void NDockPanel::setBackground( const NColor & background )
{
  area_->setBackground(background);
}

void NDockPanel::setTransparent( bool on )
{
  area_->setTransparent(on);
}


