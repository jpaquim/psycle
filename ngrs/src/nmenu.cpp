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
#include "nmenu.h"
#include "napp.h"
#include "nconfig.h"
#include "nmenuitem.h"
#include "ncheckmenuitem.h"
#include "nmenuseperator.h"
#include "nbevelborder.h"

NMenu::NMenu()
 : NCustomButton()
{
  init();
}

NMenu::NMenu( std::string text )
{
  init();
  setText(text);
}

NMenu::NMenu( std::string text, char mnemonic )
{
  init();
  setText(text);
  setMnemonic(mnemonic);
}

NMenu::NMenu( std::string text, char mnemonic, std::string create )
{
  init();
  setText(text);
  setMnemonic(mnemonic);
  add(create);
}

void NMenu::init( )
{
  popupMenu_ = new NPopupMenu();
  add(popupMenu_);
  popupMenu_->hideRequest.connect(this,&NMenu::onHideRequest);
  popupMenu_->leftPress.connect(this,&NMenu::onLeftPress);
  popupMenu_->rightPress.connect(this,&NMenu::onRightPress);
  setTransparent(true);
  selectedItem_ = 0;
  del = ',';

  NApp::config()->setSkin(&btnOver_,"mbtnover");
  NApp::config()->setSkin(&btnNone_,"mbtnnone");

  setSkin(btnNone_);
}


NMenu::~NMenu()
{
  delete border_;
}

NPopupMenu * NMenu::popupMenu( )
{
  return popupMenu_;
}

void NMenu::addSeperator( )
{
  popupMenu_->addSeperator();
}

void NMenu::add( NCustomMenuItem * item )
{
  item->click.connect(this,&NMenu::onItemSelected);
  popupMenu_->add(item);
}

void NMenu::add( NRuntime * component )
{
  NCustomButton::add(component);
}

void NMenu::onKeyAcceleratorNotify(NKeyAccelerator accelerator) {
   NButtonEvent ev(this,0,0,0); // fake a button press on this menu
   click.emit(&ev);
}

void NMenu::onHideRequest( NObject * sender )
{
  menuHideRequest.emit(this);
}

void NMenu::onLeftPress( NObject * sender )
{
  leftPress.emit(this);
}

void NMenu::onRightPress( NObject * sender )
{
  rightPress.emit(this);
}

void NMenu::onMouseEnter( )
{
  setSkin(btnOver_);
  repaint();
  menuEntered.emit(this);
}

void NMenu::onMouseExit( )
{
  if (!popupMenu()->mapped()) {
    NCustomButton::onMouseExit();
    setSkin(btnNone_);
    repaint();
  }
}

void NMenu::onItemSelected( NButtonEvent * ev )
{
  selectedItem_ = ev->sender();
  NEvent ev_menu(this,text());
  itemClicked.emit(&ev_menu,ev);
}

NObject * NMenu::selectedItem( )
{
  return selectedItem_;
}

void NMenu::add( std::string create )
{
  unsigned int i = 0;
  int start = 0;
  std::string substr;
  do {
    i = create.find(del, i);
    if (i != std::string::npos) {
       substr = create.substr(start,i-start);
       i+=1;
       start = i;
    } else substr = create.substr(start);
    if (substr=="|") {
      add(new NMenuSeperator());
    } else {
      if (substr.substr(0,2)=="&&") {
        substr.erase(0,2);
        NCheckMenuItem *item = new NCheckMenuItem(substr);
        add(item);
        item->setName(substr);
      } else {
        NMenuItem *item = new NMenuItem(substr);
        add(item);
        item->setName(substr);
      }
    }
  } while (i != std::string::npos);
}

NCustomMenuItem * NMenu::itemByName( const std::string & name )
{
  return popupMenu_->itemByName(name);
}

