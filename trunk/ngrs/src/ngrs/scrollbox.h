/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#ifndef SCROLLBOX_H
#define SCROLLBOX_H

#include "panel.h"

/**
@author  Stefan
*/

namespace ngrs {

  class ScrollBar;

  const int nAlwaysVisible = 1;
  const int nNoneVisible   = 2;

  class ScrollBox : public Panel
  {
  public:
    ScrollBox();

    ~ScrollBox();

    virtual void resize();
    void setScrollPane(VisualComponent* scrollPane);
    VisualComponent* scrollPane();

    void setHScrollBarPolicy(int policy);

    ScrollBar* horBar();
    ScrollBar* verBar();

  private:

    ScrollBar* horBar_;
    ScrollBar* verBar_;
    VisualComponent* scrollPane_;

    void init();
    void onVPosChange( ScrollBar* sender );
    void onHPosChange( ScrollBar* sender );

  };

}

#endif
