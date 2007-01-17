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
#ifndef NTABBAR_H
#define NTABBAR_H

#include "panel.h"
#include "tab.h"
#include "notebook.h"
#include "togglepanel.h"
#include "flowlayout.h"

/**
@author  Stefan
*/

namespace ngrs {

  class NNotebook;
  class FlowLayout;

  class TabBar : public TogglePanel
  {
  public:
    TabBar();

    ~TabBar();

    void addTab(NTab* tab, VisualComponent* page);
    void setNoteBook(NoteBook* noteBook);
    CustomButton* tab(VisualComponent* page);

    void setOrientation(int orientation);
    void setActiveTab(VisualComponent* page);
    void setActiveTab(unsigned int index);

    virtual void removeChild( VisualComponent * child );


  private:

    FlowLayout fl;

    int orientation_;

    std::map< Object*, VisualComponent* > pageMap_;
    NoteBook* noteBook_;

    void onTabClick(ButtonEvent* sender);

  };

}

#endif
