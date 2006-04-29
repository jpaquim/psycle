/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "ntogglepanel.h"
#include "ncustombutton.h"


NTogglePanel::NTogglePanel()
 : NPanel()
{
}


NTogglePanel::~NTogglePanel()
{
}

void NTogglePanel::add( NCustomButton * toggleComponent )
{
  if (toggleComponent->toggle()) {
    toggleComponent->click.connect(this,&NTogglePanel::onClick);
    if (visualComponents().size()==0) toggleComponent->setDown(true); else toggleComponent->setDown(false);
  }
  NPanel::add(toggleComponent);
}

void NTogglePanel::onClick( NButtonEvent * ev )
{
  for (std::vector<NRuntime*>::iterator it=components.begin(); it<components.end(); it++) {
    NRuntime* msgClient = *it;
    if (msgClient != ev->sender()) {
      NEvent toggleEvent(ev->sender(),"toggle:'up'");
      msgClient->onCustomMessage(&toggleEvent);
    }
  }
  resize();
  repaint();
}

void NTogglePanel::setDown( NCustomButton * btn )
{
  for (std::vector<NRuntime*>::iterator it=components.begin(); it<components.end(); it++) {
    NRuntime* msgClient = *it;
    if (msgClient != btn) {
      NEvent toggleEvent(btn,"toggle:'up'");
      msgClient->onCustomMessage(&toggleEvent);
    } else {
       btn->setDown(true);
    }
  }
  resize();
  repaint();
}

void NTogglePanel::add( NVisualComponent * component, int align )
{
   NPanel::add(component,align);
}


