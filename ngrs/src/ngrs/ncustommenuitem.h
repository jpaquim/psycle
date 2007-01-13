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
#ifndef NCUSTOMMENUITEM_H
#define NCUSTOMMENUITEM_H

#include "ncustomitem.h"
#include "nalignlayout.h"

/**
@author Stefan
*/

namespace ngrs {

  class NCustomMenuItem : public NCustomItem
  {
  public:
    NCustomMenuItem();

    ~NCustomMenuItem();

    signal1<NButtonEvent*> click;

    virtual void add(class NMenu* menu);
    virtual void add(NRuntime* comp);
    virtual void add(NVisualComponent* comp, int align);

    virtual void onMouseEnter();
    virtual void onMouseExit();

    virtual void onMousePress(int x, int y, int button);

    virtual void onMessage(NEvent* ev);

    virtual void setFont(const NFont & font);

  private:

    NSkin itemNone;
    NSkin itemOver;


  };

}

#endif
