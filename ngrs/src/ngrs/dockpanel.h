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
#ifndef DOCKPANEL_H
#define DOCKPANEL_H

#include "panel.h"

namespace ngrs {
  class Image;
  class Label;

  /**
  @author  Stefan Nattkemper
  */
  class DockPanel : public Panel
  {
  public:
    DockPanel();
    DockPanel(VisualComponent* clientComponent);

    ~DockPanel();

    Panel* pane();

    void setAutoHideOnDockOut( bool on );

    void onDockWindow();

  private:

    Bitmap undockBmp;
    Bitmap dockBmp;
    Image* dockImg;

    Panel* dockBar_;
    Panel* area_;
    Label* dockBarLabel_;

    Window* undockedWindow;
    
    bool autoHideDock_;

    void init();

    void onUndockWindow(ButtonEvent* ev);
    void dockWindow();
  };
}

#endif
