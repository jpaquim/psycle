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
#ifndef NTEXTSTATUSITEM_H
#define NTEXTSTATUSITEM_H

#include "customstatusitem.h"

/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class Label;

  class TextStatusItem : public CustomStatusItem
  {
  public:
    TextStatusItem();
    TextStatusItem( unsigned int modelIndex );
    TextStatusItem(const std::string & text);

    ~TextStatusItem();

    void setText(const std::string & text);
    const std::string & text() const;

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual void resize();

  private:

    Label* label_;

    void init();

  };

}

#endif
