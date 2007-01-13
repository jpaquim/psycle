/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#ifndef NTOOLBAR_H
#define NTOOLBAR_H

#include "ntogglepanel.h"

/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class NButton;
  class NPopupWindow;

  class NToolBar : public NTogglePanel
  {
  public:

    NToolBar();

    ~NToolBar();

    virtual void paint( NGraphics* g );
    virtual void add( NRuntime* component );
    virtual NButton* add( NButton*  button );
    virtual void add( NVisualComponent* comp );

    virtual void resize();

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;


  protected:

    virtual void drawChildren( NGraphics* g, const NRegion & repaintArea, NVisualComponent* sender );
    virtual NVisualComponent* checkChildrenEvent( NGraphics* g, int absX, int absY );

  private:

    int ident_;

    NButton* moreBtn_;
    NPopupWindow* popup_;

    void doAlign();
    void onMoreBtnClicked( NButtonEvent* ev );

  };

}

#endif
