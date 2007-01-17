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
#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "togglepanel.h"

/**
@author  Stefan Nattkemper
*/

namespace ngrs {

  class Button;
  class PopupWindow;

  class ToolBar : public TogglePanel
  {
  public:

    ToolBar();

    ~ToolBar();

    virtual void paint( Graphics& g );
    virtual void add( Runtime* component );
    virtual Button* add( Button*  button );
    virtual void add( VisualComponent* comp );

    virtual void resize();

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;


  protected:

    virtual void drawChildren( Graphics& g, const ngrs::Region & repaintArea, VisualComponent* sender );
    virtual VisualComponent* checkChildrenEvent( Graphics& g, int absX, int absY );

  private:

    int ident_;

    Button* moreBtn_;
    PopupWindow* popup_;

    void doAlign();
    void onMoreBtnClicked( ButtonEvent* ev );

  };

}

#endif
