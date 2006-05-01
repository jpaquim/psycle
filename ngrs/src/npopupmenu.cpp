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
#include "npopupmenu.h"
#include "napp.h"
#include "nconfig.h"
#include "nframeborder.h"

NPopupMenu::NPopupMenu()
 : NWindow()
{
  setDecoration(false);
  pane()->setLayout(new NListLayout(),true);

  NApp::config()->setSkin(&pane()->skin_,"popmnubg");
}


NPopupMenu::~NPopupMenu()
{
}

void NPopupMenu::setVisible( bool on )
{
  if (on) {
     pack();
  }

  NWindow::setVisible(on);
}

void NPopupMenu::add( NCustomMenuItem * item )
{
  pane()->add(item, nAlClient);
}

void NPopupMenu::onMessage( NEvent * ev )
{
  if (ev->text() == "ngrs_menu_item_click") {
     NEvent ev(this,"ngrs_menu_item_click");
     sendMessage(&ev);
  }
}



