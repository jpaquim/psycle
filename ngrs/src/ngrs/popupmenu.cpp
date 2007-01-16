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
#include "popupmenu.h"
#include "app.h"
#include "config.h"
#include "frameborder.h"
#include "listlayout.h"
#include <iostream>
#include <algorithm>

namespace ngrs {

  NPopupMenu::NPopupMenu()
    : PopupWindow(), lastOverItem(0)
  {
    setName("name");
    pane()->setLayout(ListLayout());
    pane()->skin_ = App::config()->skin("popup_menu_bg");
  }


  NPopupMenu::~NPopupMenu()
  {
  }

  void NPopupMenu::setVisible( bool on )
  {
    if (on) pack();
    Window::setVisible(on);
    setGrabEvents(on);
    if (on) {
      std::vector<CustomItem*>::iterator it =items.begin();
      lastOverItem = 0;
      for ( ; it < items.end() ; it++ ) {
        CustomItem* item = *it;
        item->onMouseExit();
      }
    }

  }

  void NPopupMenu::add( CustomMenuItem * item )
  {
    pane()->add(item, nAlCenter);
    items.push_back(item);
    item->addMessageListener(this);
    addMessageListener(item);
  }

  void NPopupMenu::onMessage( Event * ev )
  {
    PopupWindow::onMessage(ev);

    if (ev->text() == "ngrs_menu_item_click") {
      Event ev1(this,"ngrs_menu_item_click");
      sendMessage(&ev1);
      checkForRemove(0);
      setVisible(false);
    } else
      if (ev->text() == "ngrs_global_hide") {
        Event ev1(this,"ngrs_menu_item_click");
        sendMessage(&ev1);
        checkForRemove(0);
        setVisible(false);
      } else
        if (ev->text() == "ngrs_menu_item_enter") {
          lastOverItem = ev->sender();
        } else
          if (ev->text() == "ngrs_menu_item_exit") {
            lastOverItem = 0;
          }
  }

  void NPopupMenu::onKeyPress( const KeyEvent & event )
  {
    switch (event.scancode()) {
      case NK_Return : {
        if ( lastOverItem ) {
          Event ev1(this,"ngrs_menu_item_click");
          sendMessage(&ev1);
          setVisible(false);
        }
                       }
                       break;
      case NK_Left : {
        Event ev(this, "ngrs_menu_key_left");
        sendMessage(&ev);
                     }
                     break;
      case NK_Right : {
        Event ev(this, "ngrs_menu_key_right");
        sendMessage(&ev);
                      }
                      break;
      case NK_Up : {
        if ( !lastOverItem && items.size() > 0 ) {
          lastOverItem = items.back();
          Event ev1(this, "ngrs_menu_item_do_enter");
          lastOverItem->onMessage(&ev1);
        } else 
          if (items.size() > 0) {
            if (lastOverItem == *items.begin() ) {
              Event ev1(this, "ngrs_menu_item_do_exit");
              lastOverItem->onMessage(&ev1);
              lastOverItem = items.back();
              Event ev2(this, "ngrs_menu_item_do_enter");
              lastOverItem->onMessage(&ev2);
            } else {
              std::vector<CustomItem*>::iterator it = find(items.begin(),items.end(),lastOverItem);
              it--;
              Event ev1(this, "ngrs_menu_item_do_exit");
              lastOverItem->onMessage(&ev1);
              lastOverItem = *it;
              Event ev2(this, "ngrs_menu_item_do_enter");
              lastOverItem->onMessage(&ev2);
            }
          }
                   }
                   break;
      case NK_Down : {
        if ( !lastOverItem && items.size() > 0 ) {
          lastOverItem = *items.begin();
          Event ev1(this, "ngrs_menu_item_do_enter");
          lastOverItem->onMessage(&ev1);
        } else 
          if (items.size() > 0) {
            if (lastOverItem == items.back() ) {
              Event ev1(this, "ngrs_menu_item_do_exit");
              lastOverItem->onMessage(&ev1);
              lastOverItem = *items.begin();
              Event ev2(this, "ngrs_menu_item_do_enter");
              lastOverItem->onMessage(&ev2);
            } else {
              std::vector<CustomItem*>::iterator it = find(items.begin(),items.end(),lastOverItem);
              it++;
              Event ev1(this, "ngrs_menu_item_do_exit");
              lastOverItem->onMessage(&ev1);
              lastOverItem = *it;
              Event ev2(this, "ngrs_menu_item_do_enter");
              lastOverItem->onMessage(&ev2);
            }
          }
                     }
                     break;
    }
  }

  void NPopupMenu::removeChilds( )
  {
    items.clear();
    lastOverItem = 0;
    pane()->removeChilds();
  }

}
