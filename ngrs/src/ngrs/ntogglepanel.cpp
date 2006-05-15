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
  selectedComp_ = 0;
}


NTogglePanel::~NTogglePanel()
{
}

// the class factories

extern "C" NObject* createTogglePanel() {
    return new NTogglePanel();
}

extern "C" void destroyTogglePanel(NObject* p) {
    delete p;
}


void NTogglePanel::add( NCustomButton * toggleComponent )
{
  if (toggleComponent->toggle()) {
    toggleComponent->click.connect(this,&NTogglePanel::onClick);
    if (visualComponents().size()==0) {
      toggleComponent->setDown(true);
      selectedComp_ = toggleComponent;
    }
    else 
      toggleComponent->setDown(false);
  }
  NPanel::add(toggleComponent);
}

void NTogglePanel::onClick( NButtonEvent * ev )
{
  for (std::vector<NVisualComponent*>::const_iterator it=visualComponents().begin(); it<visualComponents().end(); it++) {
    NVisualComponent* msgClient = *it;
    if (msgClient != ev->sender()) {
      NEvent toggleEvent(ev->sender(),"toggle:'up'");
      msgClient->onMessage(&toggleEvent);
    } else {
       selectedComp_ = *it;
       click.emit(ev);
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
      msgClient->onMessage(&toggleEvent);
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

NVisualComponent * NTogglePanel::selectedComponent( )
{
  return selectedComp_;
}


