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
#include "nmenubar.h"
#include "napp.h"
#include "nconfig.h"

NMenuBar::NMenuBar()
 : NPanel()
{
  NFlowLayout* flow = new NFlowLayout(nAlLeft);
  flow->setVgap(2);
  setLayout(flow,true);
  setAlign(nAlTop);
  aMenuMapped_ = false;
  NFont fnt = font();
  fnt.setSize(10);
  setFont(fnt);
  lastMenu_ = 0;
  lastUnmapMenu_ = 0;

  NApp::config()->setSkin(&skin_,"mbar");
}

void NMenuBar::setSkin( )
{

}

NMenuBar::~NMenuBar()
{
}

void NMenuBar::add( NMenu * menu )
{
  NVisualComponent::add(menu);
  menu->menuEntered.connect(this,&NMenuBar::onMenuEnter);
  menu->click.connect(this,&NMenuBar::onMenuClick);
  menu->menuHideRequest.connect(this,&NMenuBar::unmapMenu);
  menu->leftPress.connect(this,&NMenuBar::mapLeft);
  menu->rightPress.connect(this,&NMenuBar::mapRight);
  NApp::addKeyAccelerator(NKeyAccelerator(Mod1Mask,menu->mnemonic()),menu);
}

void NMenuBar::onMenuEnter( NObject * sender )
{
  if (aMenuMapped_) mapMenu(sender);
}

void NMenuBar::onMenuClick( NButtonEvent * ev )
{
   if (!NApp::popupUnmapped_) {
     aMenuMapped_ = !aMenuMapped_;
     if (aMenuMapped_) {
        mapMenu(ev->sender());
     } else unmapMenu(ev->sender());
   }
   lastUnmapMenu_ = 0;
}

void NMenuBar::mapRight( NObject * actual )
{
  std::vector<NRuntime*>::iterator it = std::find(components.begin(),components.end(),actual);
  if (it!=components.end()) {
     it++;
     if (it == components.end()) it = components.begin();
  } else it = components.begin();

  mapMenu(*it);
}

void NMenuBar::mapLeft( NObject * actual )
{
  std::vector<NRuntime*>::iterator it = std::find(components.begin(),components.end(),actual);
  if (it!=components.begin()) it--;
  else {
    it = components.end();
    it--;
  }
  mapMenu(*it);
}

void NMenuBar::mapMenu(NObject* sender)
{
  if (lastMenu_ == sender) return;
  NMenu* menu = (NMenu*) sender;
  if (lastMenu_ != 0) unmapMenu(lastMenu_);
  NWindow* win = window();
  menu->popupMenu()->setPosition(win->left()+menu->absoluteLeft(), win->top()+menu->absoluteTop()+menu->height(),menu->popupMenu()->width(),menu->popupMenu()->height());
  menu->popupMenu()->setVisible(true);
  lastMenu_ = menu;
  aMenuMapped_ = true;
}

void NMenuBar::unmapMenu( NObject * sender )
{
  NMenu* menu = (NMenu*) sender;
  lastUnmapMenu_ = menu;
  menu->popupMenu()->setVisible(false);
  menu->onMouseExit();
  aMenuMapped_ = false;
  lastMenu_=0;
}






