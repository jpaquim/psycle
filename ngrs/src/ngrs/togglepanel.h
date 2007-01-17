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
#ifndef NTOGGLEPANEL_H
#define NTOGGLEPANEL_H

#include "panel.h"

/**
@author  Stefan
*/

namespace ngrs {

  class CustomButton;

  class TogglePanel : public Panel
  {
  public:
    TogglePanel();

    ~TogglePanel();

    virtual void add(VisualComponent* component, int align);
    virtual void add(CustomButton* component, int align);
    virtual void add(CustomButton* toogleComponent);
    virtual void add(CustomButton* component, const AlignConstraint & align );
    virtual void add(VisualComponent* component, const AlignConstraint & align );

    virtual void removeChild( VisualComponent* child);
    virtual void removeChilds();

    void setDown(CustomButton* btn);

    VisualComponent* selectedComponent();

    signal1<ButtonEvent*> click;

  private:

    void onClick(ButtonEvent* ev);

    VisualComponent* selectedComp_;
  };

}

#endif
