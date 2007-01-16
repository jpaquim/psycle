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
#ifndef NCUSTOMSTATUSITEM_H
#define NCUSTOMSTATUSITEM_H

#include "panel.h"

/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class CustomStatusItem : public Panel
  {
  public:

    CustomStatusItem();

    CustomStatusItem( unsigned int modelIndex );

    ~CustomStatusItem();

    virtual void setText( const std::string & text ) = 0;

    void setModelIndex( unsigned int modelIndex );
    unsigned int modelIndex() const;

  private:

    unsigned int modelIndex_;

  };

}

#endif
