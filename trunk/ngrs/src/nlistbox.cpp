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
#include "nlistbox.h"
#include "nbuttonevent.h"
#include "napp.h"
#include "nconfig.h"

NListBox::NListBox()
 : NScrollBox()
{
  listBoxPane_ = new NPanel();
  listBoxPane_->setBackground(NColor(255,255,255));
  listBoxPane_->setTransparent(false);
  setScrollPane(listBoxPane_);

  listBoxPane_->setLayout(new NListLayout(),true);
  listBoxPane_->setClientSizePolicy(nVertical + nHorizontal);

  multiSelect_ = false;

  setHScrollBarPolicy(nNoneVisible);

  NApp::config()->setSkin(&itemBg,"lbitemsel");
  NApp::config()->setSkin(&itemFg,"lbitemnone");
}


NListBox::~NListBox()
{
}

void NListBox::add( NCustomItem * component , bool align)
{
  listBoxPane_->add(component);
  component->mousePress.connect(this,&NListBox::onItemPress);
  component->setTransparent(true);
  if (align) listBoxPane_->resize();
}

void NListBox::insert( NCustomItem * component, int index , bool align )
{
  listBoxPane_->insert(component,index);
  component->mousePress.connect(this,&NListBox::onItemPress);
  component->setTransparent(true);
  if (align) listBoxPane_->resize();
}


void NListBox::add( NCustomItem * component )
{
  add(component,true);
}

void NListBox::onItemPress( NButtonEvent * ev)
{
  NVisualComponent* item = static_cast<NVisualComponent*>(ev->sender());

  if (!multiSelect_ || !(NApp::system().keyState() & ControlMask)) deSelectItems();

  item->setSkin(itemBg);
  item->repaint();

  onItemSelected((NCustomItem*) (ev->sender()));
}

void NListBox::onItemSelected( NCustomItem * item )
{
  if (!multiSelect_) deSelectItems();
  selItems_.push_back(item);
  NItemEvent ev(item,item->text());
  itemSelected.emit(&ev);
}

void NListBox::removeChilds( )
{
  selItems_.clear();
  listBoxPane_->removeChilds();
  listBoxPane_->setScrollDx(0);
  listBoxPane_->setScrollDy(0);
}

void NListBox::deSelectItems( )
{
  for (std::vector<NCustomItem*>::iterator it = selItems_.begin(); it < selItems_.end(); it++) {
    NCustomItem* item = *it;
    item->setSkin(itemFg);
    item->repaint();
  }
  selItems_.clear();
}

int NListBox::itemCount( )
{
  return listBoxPane_->visualComponents().size();
}

int NListBox::selIndex( )
{
  int c = 0;
  if (selItems_.size()==0) return -1;
  for (std::vector<NRuntime*>::iterator it = listBoxPane_->components.begin(); it < listBoxPane_->components.end(); it++) {
     if (*it == selItems_.at(0)) {
        return c;
     }
     c++;
  }
  return -1;
}

std::vector< int > NListBox::selIndexList( )
{
  std::vector<int> indexList;
  for (std::vector<NCustomItem*>::iterator it = selItems_.begin(); it < selItems_.end(); it++) {
     NCustomItem* item = *it;
     indexList.push_back(item->zOrder());
  }
  return indexList;
}

void NListBox::setIndex( unsigned int i )
{
  if (!multiSelect_) deSelectItems();
  if (i>=0 && i < listBoxPane_->visualComponents().size()) {
     NCustomItem* item = (NCustomItem*) listBoxPane_->componentByZOrder(i); ///todo avoid cast
     selItems_.push_back(item);
     item->setSkin(itemBg);
     item->repaint();
  }
}

void NListBox::setMultiSelect( bool on )
{
  multiSelect_ = on;
}

void NListBox::selClear( )
{
  deSelectItems();
}

NCustomItem * NListBox::itemAt( unsigned int index )
{
  if (index >= 0 && index < listBoxPane_->visualComponents().size())
    return (NCustomItem*) listBoxPane_->visualComponents().at(index);
  else
    return 0;
}






