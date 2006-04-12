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
const char * winbtn_xpm[] = {
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


NDockPanel::NDockPanel()
 : NPanel()
{
  alignLayout = new NAlignLayout();
  NPanel::setLayout(alignLayout);

  dockBar_ = new NPanel();
    dockBar_->setLayout( flowLayout = new NFlowLayout(nAlRight,0,0));
  NPanel::add(dockBar_,nAlTop);

    NImage* img = new NImage();
      img->createFromXpmData(winbtn_xpm);
      img->setPreferredSize(20,17);
    NButton* unCoupleBtn = new NButton(img);
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
  undockedWindow = new NWindow();
  NPanel::add(undockedWindow);

  undockedWindow->setDock(this);

  undockedWindow->erase(undockedWindow->pane_);
  delete undockedWindow->pane_;

  erase(area_);

  oldAreaLayout_ = area_->layout();
  area_->setLayout(0);
  undockedWindow->pane_ = area_;
  undockedWindow->pane_->setLayout(new NAlignLayout());

  NApp::config()->setSkin(&undockedWindow->pane_->skin_,"pane");
  undockedWindow->add(undockedWindow->pane_);

  window()->resize();
  window()->repaint(window()->pane(),NRect(0,0,window()->width(),window()->height()));

  undockedWindow->setPosition(0,0,area_->width(),area_->height());
  area_->setPosition(0,0,area_->width(),area_->height());
  undockedWindow->pane()->resize();
  undockedWindow->setVisible(true);
}

void NDockPanel::onDockWindow( )
{
  undockedWindow->erase(undockedWindow->pane_);
  area_ = undockedWindow->pane_;
  area_->setLayout(oldAreaLayout_);
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


