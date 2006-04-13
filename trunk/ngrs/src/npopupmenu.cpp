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
#include "npopupmenu.h"
#include "napp.h"
#include "nconfig.h"
#include "nframeborder.h"

NPopupMenu::NPopupMenu()
 : NWindow()
{
  setDecoration(false);
  NFont fnt = pane()->font();
  fnt.setSize(10);
  pane()->setFont(fnt);
  NVisual::setVisible(false);
  nl = new NListLayout();
  pane()->setLayout(nl);
  NApp::config()->setSkin(&pane()->skin_,"popmnubg");
  selItemIndex_ = -1;
  subMenuItem_ = 0;
  NApp::addPopupWindow(this);
}


NPopupMenu::~NPopupMenu()
{
  delete nl;
}

void NPopupMenu::setVisible( bool on )
{
  if (visible()==on) return;

   if (on) {
      updateAlign();
      pack();
      selItemIndex_ = -1;
      NWindow::setVisible(on);
      setGrabEvents(true);
    } else  {
      NVisualComponent* item = pane()->componentByZOrder(selItemIndex_);
         if (item!=0) {
           item->onMouseExit();
         }
      NWindow::setVisible(on);
      setGrabEvents(false);
      hideRequest.emit(this);
    }
}

void NPopupMenu::addSeperator( )
{
  pane()->add(new NMenuSeperator());
}

void NPopupMenu::updateAlign( )
{
  int maxIcon     = 0;
  int maxCaption  = 0;
  int maxShortCut = 0;
  int maxSubHint  = 0;
  std::vector<NRuntime*>::iterator itr = pane()->components.begin();
  for (;itr < pane()->components.end(); itr++) {
      NRuntime* child = *itr;
      if (child->visit(NVisualComponent::isVisualComponent)) {
         NCustomMenuItem* item = (NCustomMenuItem*) child;
         if (maxIcon     < item->maxIconSize()) maxIcon = item->maxIconSize();
         if (maxCaption  < item->maxCaptionSize()) maxCaption = item->maxCaptionSize();
         if (maxShortCut < item->maxShortCutSize()) maxShortCut = item->maxShortCutSize();
         if (maxSubHint  < item->maxHintSize()) maxSubHint = item->maxHintSize();
      }
  }
  itr = pane()->components.begin();
  for (;itr < pane()->components.end(); itr++) {
      NRuntime* child = *itr;
      if (child->visit(NVisualComponent::isVisualComponent)) {
         NCustomMenuItem* item = (NCustomMenuItem*) child;
         item->setIconSize(maxIcon);
         item->setCaptionSize(maxCaption);
         item->setShortCutSize(maxShortCut);
         item->setHintSize(maxSubHint);
         item->setHeight(item->preferredHeight());
      }
  }
}



void NPopupMenu::onKeyPress( const NKeyEvent & keyEvent )
{
   if (NApp::system().keyState() & Mod1Mask) {
      std::vector<NRuntime*>::iterator itr = pane()->components.begin();
      for (;itr < pane()->components.end(); itr++) {
      NRuntime* child = *itr;
      if (child->visit(NVisualComponent::isVisualComponent)) {
         NCustomMenuItem* item = (NCustomMenuItem*) child;
         if (item->mnemonic()==keyEvent.scancode()) {
            NButtonEvent ev(item,0,0,0);
            item->click.emit(&ev);
            return;
         }
      }
     }
  }

   switch (keyEvent.scancode()) {
     case XK_Return: {
        NVisualComponent* item = pane()->componentByZOrder(selItemIndex_);
          if (item!=0) {
             NCustomMenuItem* menuItem = (NCustomMenuItem*) item;
             NButtonEvent ev(menuItem,0,0,0);
             menuItem->click.emit(&ev);
          }
        }
     break;
     case XK_Alt_L :
        hideRequest.emit(this);
     break;
     case XK_Left:
        leftPress.emit(this);
     break;
     case XK_Right:
        rightPress.emit(this);
     break;
     case XK_Down: {
         NVisualComponent* item = pane()->componentByZOrder(selItemIndex_);
         if (item!=0) {
           item->onMouseExit();
         }
         if (pane()->componentZOrderSize()-1>selItemIndex_) selItemIndex_++;
         setSelItem(selItemIndex_);
        }
     break;
     case XK_Up: {
        NVisualComponent* item = pane()->componentByZOrder(selItemIndex_);
         if (item!=0) { 
             item->onMouseExit(); 
             if (selItemIndex_>0) selItemIndex_--;
         } else selItemIndex_=0;
         setSelItem(selItemIndex_);
        }
     break;
   }
}

void NPopupMenu::setSelItem( int index )
{
  NVisualComponent* item = pane()->componentByZOrder(index);
  if (item!=0) {
    item->onMouseEnter();
  }
}

void NPopupMenu::add( NCustomMenuItem * item )
{
  pane()->add(item);
  item->mouseEnter.connect(this,&NPopupMenu::onItemEnter);
  item->subMenuMapped.connect(this, &NPopupMenu::onSubMenuMapped);

  items.push_back(item);
}

void NPopupMenu::onItemEnter( NEvent * ev )
{
  selItemIndex_ = ((NVisualComponent*) ev->sender())->zOrder();
  if (subMenuItem_ != 0 && ev->sender() != subMenuItem_) subMenuItem_->onMouseExit();
}

void NPopupMenu::onSubMenuMapped( NObject * sender)
{
  subMenuItem_ = static_cast<NCustomMenuItem*>(sender);
}

NCustomMenuItem * NPopupMenu::itemByName( const std::string & name )
{
  for (std::vector<NCustomMenuItem*>::iterator it = items.begin(); it < items.end(); it++) {
     NCustomMenuItem* item = *it;
     if (item->text() == name) return item;
  }

  return 0;
}

