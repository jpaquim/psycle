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
#include "ngrs/npopupmenu.h"
#include "ngrs/napp.h"
#include "ngrs/nconfig.h"
#include "ngrs/nframeborder.h"
#include "ngrs/nlistlayout.h"
#include <iostream>

NPopupMenu::NPopupMenu()
 : NPopupWindow()
{
  setName("name");
  pane()->setLayout(NListLayout());
  pane()->skin_ = NApp::config()->skin("popup_menu_bg");
}


NPopupMenu::~NPopupMenu()
{
}

void NPopupMenu::setVisible( bool on )
{
  if (on) pack();
  NWindow::setVisible(on);
  setGrabEvents(on);
}

void NPopupMenu::add( NCustomMenuItem * item )
{
  pane()->add(item, nAlClient);
  item->addMessageListener(this);
}

void NPopupMenu::onMessage( NEvent * ev )
{
  if (ev->text() == "ngrs_menu_item_click") {
     NEvent ev1(this,"ngrs_menu_item_click");
     sendMessage(&ev1);
     setVisible(false);
  } else
  if (ev->text() == "ngrs_global_hide") {
     NEvent ev1(this,"ngrs_menu_item_click");
     sendMessage(&ev1);
     setVisible(false);
  }
}

void NPopupMenu::onKeyPress( const NKeyEvent & event )
{
    switch (event.scancode()) {
    case XK_Left : {
      NEvent ev(this, "ngrs_menu_key_left");
      sendMessage(&ev);
    }
    break;
    case XK_Right : {
      NEvent ev(this, "ngrs_menu_key_right");
      sendMessage(&ev);
    }
    break;
  }
}





