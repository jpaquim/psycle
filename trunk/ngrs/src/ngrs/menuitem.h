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
#ifndef NMENUITEM_H
#define NMENUITEM_H

#include "custommenuitem.h"


/**
 @author  Stefan
*/

namespace ngrs {

  class Label;
  class Menu;
  class Image;

  class MenuItem : public CustomMenuItem
  {
  public:
    MenuItem();
    MenuItem(const std::string & text);
    MenuItem(const std::string & text, const Bitmap & icon);

    ~MenuItem();

    virtual void add(Menu* menu);

    virtual void onMouseEnter();
    virtual void onMouseExit();

    Label* captionLbl_;

  private:

    Menu* menu_;


    Image* iconImg_;
    Image* subMenuImg_;

    void init();

  };

}

#endif
