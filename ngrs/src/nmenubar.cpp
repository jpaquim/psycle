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
#include "nmenubar.h"
#include "napp.h"
#include "nconfig.h"
#include "nflowlayout.h"
#include <nmenu.h>


NMenuBar::NMenuBar()
 : NPanel(), isMenuMapped_(0)
{
  setLayout(new NFlowLayout(nAlLeft,5,5),true);
  setAlign(nAlTop);

  NApp::config()->setSkin(&skin_,"mbar");
}


NMenuBar::~NMenuBar()
{
}

void NMenuBar::add( NMenu * menu )
{
  NVisualComponent::add(menu);
  menu->addMessageListener(this);
  menus.push_back(menu);
}

void NMenuBar::onMessage( NEvent * ev )
{
  if (ev->text() == "ngrs_menu_enter") {
    if (isMenuMapped_) {
      NEvent ev(ev->sender(),"ngrs_menu_expose");
      NPanel::onMessage(&ev);
    }
  } else
  if (ev->text() == "ngrs_menu_press") {
    isMenuMapped_ = !isMenuMapped_;
    if (isMenuMapped_) {
      NEvent ev1(ev->sender(),"ngrs_menu_expose");
      NPanel::onMessage(&ev1);
    } else {
      NEvent ev1(ev->sender(),"ngrs_menu_hide");
      NPanel::onMessage(&ev1);
    }
  } else
  if (ev->text() == "ngrs_menu_key_right") {
    std::vector<NObject*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
    NEvent ev1(ev->sender(),"ngrs_menu_hide");
    NPanel::onMessage(&ev1);
    if (it < menus.end()) {
      it++;
      if (it != menus.end()) {
        NEvent ev1(*it,"ngrs_menu_expose");
        NPanel::onMessage(&ev1);
      } else {
        if (menus.size() > 0) {
          NEvent ev1(*menus.begin(),"ngrs_menu_expose");
          NPanel::onMessage(&ev1);
        }
      }
    }
  } else
  if (ev->text() == "ngrs_menu_key_left") {
     std::vector<NObject*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
     NEvent ev1(ev->sender(),"ngrs_menu_hide");
     NPanel::onMessage(&ev1);
     if (it > menus.begin()) {
        it--;
        NEvent ev(*it,"ngrs_menu_expose");
        NPanel::onMessage(&ev1);
     } else {
       if (menus.size() > 0) {
         NEvent ev1(menus.back(),"ngrs_menu_expose");
         NPanel::onMessage(&ev1);
       }
     }
  }
}

