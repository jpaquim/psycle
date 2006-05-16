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
#include "nmenu.h"


NMenuBar::NMenuBar()
 : NPanel(), isMenuMapped_(0), lastMenuOver(0)
{
  setLayout(NFlowLayout(nAlLeft,5,2));
  setAlign(nAlTop);

  skin_ = NApp::config()->skin("mbar");
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
  if (ev->text() == "ngrs_menu_hide_all") {
      std::cout << "here" << std::endl;
  } else
  if (ev->text() == "ngrs_menu_enter") {
    if (isMenuMapped_) {
      NEvent ev1(ev->sender(),"ngrs_menu_expose");
      NPanel::onMessage(&ev1);

      if (lastMenuOver!=0 && lastMenuOver != ev->sender()) {
        lastMenuOver->setSkin(lastMenuOver->btnNone_);
        lastMenuOver->repaint();
      }
    }
    std::vector<NObject*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
      if (it != menus.end()) {
        lastMenuOver = reinterpret_cast<NMenu*> (*it);
      }
  } else
  if (ev->text() == "ngrs_menu_exit") {
    if (!isMenuMapped_) {
      std::vector<NObject*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
      if (it != menus.end()) {
        NMenu* menu = reinterpret_cast<NMenu*> (*it);
        menu->setSkin(menu->btnNone_);
        menu->repaint();
      }
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
    std::vector<NObject*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
      if (it != menus.end()) {
        lastMenuOver = reinterpret_cast<NMenu*> (*it);
      }
  } else
  if (ev->text() == "ngrs_menu_key_right") {
    std::vector<NObject*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
    NEvent ev1(ev->sender(),"ngrs_menu_hide");
    NPanel::onMessage(&ev1);

    if (it < menus.end()) {
      if (lastMenuOver!=0) {
        lastMenuOver->setSkin(lastMenuOver->btnNone_);
        lastMenuOver->repaint();
      }
      it++;
      if (it != menus.end()) {
        NEvent ev1(*it,"ngrs_menu_expose");
        NPanel::onMessage(&ev1);
        lastMenuOver = reinterpret_cast<NMenu*> (*it);
      } else {
        if (menus.size() > 0) {
          NEvent ev1(*menus.begin(),"ngrs_menu_expose");
          NPanel::onMessage(&ev1);
          lastMenuOver = reinterpret_cast<NMenu*> (*menus.begin());
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

