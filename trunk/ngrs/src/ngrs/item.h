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
#ifndef NITEM_H
#define NITEM_H

#include "customitem.h"
#include "image.h"

namespace ngrs {

  class Label;

  /**
  @author Stefan
  */

  class Item : public CustomItem
  {
  public:
    Item();
    Item(const std::string & text);

    ~Item();

    void setText(const std::string & text);

    virtual std::string text() const;

    virtual void resize();

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual void add(Image* icon);

  private:

    int iconWidth_;

    Label* label_;
    Image* icon_;

    void init();
  };

}

#endif
