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
#ifndef NCOLORITEM_H
#define NCOLORITEM_H

#include "ncustomitem.h"

/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class NColorItem : public NCustomItem
  {
  public:
    NColorItem();
    NColorItem( const NColor & color, const std::string & text);

    ~NColorItem();

    virtual void paint( Graphics& g);

    virtual void setColor(const NColor & color);
    const NColor & color() const;

    virtual int preferredHeight() const;

  private:

    NColor color_;
    std::string text_;

  };

}

#endif
