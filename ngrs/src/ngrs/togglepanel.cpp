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
#include "togglepanel.h"
#include "custombutton.h"

namespace ngrs {

  TogglePanel::TogglePanel()
    : Panel()
  {
    selectedComp_ = 0;
  }


  TogglePanel::~TogglePanel()
  {
  }

  void TogglePanel::add( CustomButton * toggleComponent )
  {
    if (toggleComponent->toggle()) {
      toggleComponent->click.connect(this,&TogglePanel::onClick);
      if (visualComponents().size()==0) {
        toggleComponent->setDown(true);
        selectedComp_ = toggleComponent;
      }
      else 
        toggleComponent->setDown(false);
    }
    Panel::add(toggleComponent);
  }

  void TogglePanel::add( VisualComponent * component, int align )
  {
    Panel::add(component,align);
  }

  void TogglePanel::add( CustomButton * component, int align )
  {
    component->setAlign(align);
    add(component);
  }

  void TogglePanel::add( CustomButton * component, const AlignConstraint & align )
  {
    component->setAlignConstraint(align);
    add(component);
  }

  void TogglePanel::add( VisualComponent * component, const AlignConstraint & align )
  {
    Panel::add(component,align,true);
  }

  void TogglePanel::onClick( ButtonEvent * ev )
  {
    for (std::vector<VisualComponent*>::const_iterator it=visualComponents().begin(); it<visualComponents().end(); it++) {
      VisualComponent* msgClient = *it;
      if (msgClient != ev->sender()) {
        Event toggleEvent(ev->sender(),"toggle:'up'");
        msgClient->onMessage(&toggleEvent);
      } else {
        selectedComp_ = *it;
        click.emit(ev);
      }
    }
    resize();
    repaint();
  }

  void TogglePanel::setDown( CustomButton * btn )
  {
    for (std::vector<Runtime*>::iterator it=components.begin(); it<components.end(); it++) {
      Runtime* msgClient = *it;
      if (msgClient != btn) {
        Event toggleEvent(btn,"toggle:'up'");
        msgClient->onMessage(&toggleEvent);
      } else {
        btn->setDown(true);
      }
    }
    resize();
    repaint();
  }


  VisualComponent * TogglePanel::selectedComponent( )
  {
    return selectedComp_;
  }

  void TogglePanel::removeChild( VisualComponent * child )
  {
    if (child == selectedComp_) {
      selectedComp_ = 0;
    }

    Panel::removeChild( child );
  }

  void TogglePanel::removeChilds( )
  {
    selectedComp_ = 0;
    Panel::removeChilds();
  }

}


// the class factories
extern "C" ngrs::Object* createTogglePanel() {
  return new ngrs::TogglePanel();
}

extern "C" void destroyTogglePanel( ngrs::Object* p) {
  delete p;
}
