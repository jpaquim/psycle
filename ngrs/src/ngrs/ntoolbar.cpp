/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "ngrs/ntoolbar.h"
#include "ngrs/nbevelborder.h"
#include "ngrs/nflowlayout.h"
#include "ngrs/napp.h"
#include "ngrs/nconfig.h"
#include "ngrs/nbutton.h"

NToolBar::NToolBar()
 : NTogglePanel()
{
  NFlowLayout fl(nAlLeft,3,2);
   fl.setLineBreak(false);
  setLayout(fl);

  spacer_ = new NPanel();
  spacer_->setWidth(5);
  spacer_->setTransparent(true);
  add(spacer_);

  skin_ = NApp::config()->skin("toolbar");
}


NToolBar::~NToolBar()
{
}

void NToolBar::paint( NGraphics * g )
{
  g->fillGradient(0,0,5,clientHeight(),NColor(background().red() + 50,background().green() + 50, background().blue()+50),NColor(background().red() + 20,background().green() + 20, background().blue()+20),nVertical );
  g->fillGradient(5,0,4,clientHeight(),NColor(background().red() + 20,background().green() + 20, background().blue()+20),NColor(background().red() - 40,background().green() - 40, background().blue()-40),nVertical);
}

void NToolBar::add( NRuntime * component )
{
  NPanel::add(component);
}

NButton* NToolBar::add( NButton * button )
{
  NTogglePanel::add(button);
  return button;
}

void NToolBar::add( NVisualComponent * comp )
{
  NPanel::add(comp);
}



