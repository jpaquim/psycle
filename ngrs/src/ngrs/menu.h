/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#ifndef MENU_H
#define MENU_H

#include "custombutton.h"

/**
@author  Stefan
*/

namespace ngrs {

  class PopupMenu;
  class CustomMenuItem;

  class Menu : public CustomButton
  {
  public:
    Menu();
    Menu(const std::string & text);

    ~Menu();

    bool mapped() const;

    PopupMenu* popupMenu();

    virtual CustomMenuItem* add(CustomMenuItem* item);

    virtual void onMouseEnter();
    virtual void onMouseExit();
    virtual void onMousePress( int x, int y, int button );

    virtual void onMessage(Event* ev);

    virtual void removeChilds();

  private:

    friend class MenuBar;

    Skin btnOver_;
    Skin btnNone_;

    PopupMenu* popupMenu_;
    bool hide_;

    void init();

  };
}

#endif
