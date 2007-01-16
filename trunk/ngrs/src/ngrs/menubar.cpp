/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "menubar.h"
#include "app.h"
#include "config.h"
#include "flowlayout.h"
#include "menu.h"
#include <algorithm>

namespace ngrs {

  MenuBar::MenuBar()
    : Panel(), isMenuMapped_(0), lastMenuOver(0)
  {
    setLayout(FlowLayout(nAlLeft,5,2));
    setAlign(nAlTop);

    skin_ = App::config()->skin("mbar");
  }


  MenuBar::~MenuBar()
  {
  }

  void MenuBar::add( Menu * menu )
  {
    VisualComponent::add(menu);
    menu->addMessageListener(this);
    menus.push_back(menu);
  }

  void MenuBar::onMessage( Event * ev )
  {
    if (ev->text() == "ngrs_menu_hide_all") {
    } else
      if (ev->text() == "ngrs_menu_enter") {
        if (isMenuMapped_) {
          Event ev1(ev->sender(),"ngrs_menu_expose");
          Panel::onMessage(&ev1);

          if (lastMenuOver!=0 && lastMenuOver != ev->sender()) {
            lastMenuOver->setSkin(lastMenuOver->btnNone_);
            lastMenuOver->repaint();
          }
        }
        std::vector<Object*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
        if (it != menus.end()) {
          lastMenuOver = reinterpret_cast<Menu*> (*it);
        }
      } else
        if (ev->text() == "ngrs_menu_exit") {
          if (!isMenuMapped_) {
            std::vector<Object*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
            if (it != menus.end()) {
              Menu* menu = reinterpret_cast<Menu*> (*it);
              menu->setSkin(menu->btnNone_);
              menu->repaint();
            }
          }
        } else
          if (ev->text() == "ngrs_menu_press") {
            isMenuMapped_ = !isMenuMapped_;
            if (isMenuMapped_) {
              Event ev1(ev->sender(),"ngrs_menu_expose");
              Panel::onMessage(&ev1);
            } else {
              Event ev1(ev->sender(),"ngrs_menu_hide");
              Panel::onMessage(&ev1);
            }
            std::vector<Object*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
            if (it != menus.end()) {
              lastMenuOver = reinterpret_cast<Menu*> (*it);
            }
          } else
            if (ev->text() == "ngrs_menu_key_right") {
              std::vector<Object*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
              Event ev1(ev->sender(),"ngrs_menu_hide");
              Panel::onMessage(&ev1);

              if (it < menus.end()) {
                if (lastMenuOver!=0) {
                  lastMenuOver->setSkin(lastMenuOver->btnNone_);
                  lastMenuOver->repaint();
                }
                it++;
                if (it != menus.end()) {
                  Event ev1(*it,"ngrs_menu_expose");
                  Panel::onMessage(&ev1);
                  lastMenuOver = reinterpret_cast<Menu*> (*it);
                } else {
                  if (menus.size() > 0) {
                    Event ev1(*menus.begin(),"ngrs_menu_expose");
                    Panel::onMessage(&ev1);
                    lastMenuOver = reinterpret_cast<Menu*> (*menus.begin());
                  }
                }
              }
            } else
              if (ev->text() == "ngrs_menu_key_left") {
                std::vector<Object*>::iterator it = find(menus.begin(),menus.end(),ev->sender());
                Event ev1(ev->sender(),"ngrs_menu_hide");
                Panel::onMessage(&ev1);
                if (it > menus.begin()) {
                  it--;
                  if (lastMenuOver!=0 && lastMenuOver != ev->sender()) {
                    lastMenuOver->setSkin(lastMenuOver->btnNone_);
                    lastMenuOver->repaint();
                  }
                  Event ev1(*it,"ngrs_menu_expose");
                  Panel::onMessage(&ev1);
                  lastMenuOver = reinterpret_cast<Menu*> (*it);
                } else {
                  if ( menus.size() > 0 ) {
                    Event ev1(menus.back(),"ngrs_menu_expose");
                    Panel::onMessage(&ev1);
                    lastMenuOver = reinterpret_cast<Menu*> (menus.back());
                  }
                }
              }
  }

  void MenuBar::removeChilds( )
  {
    menus.clear();
    Panel::removeChilds();
  }

}
