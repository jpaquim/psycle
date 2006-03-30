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

NListBox::NListBox()
 : NScrollBox()
{
  listBoxPane_ = new NPanel();
  listBoxPane_->setBackground(NColor(255,255,255));
  listBoxPane_->setTransparent(false);
  setScrollPane(listBoxPane_);

  listBoxPane_->setLayout(new NListLayout());
  listBoxPane_->setClientSizePolicy(nVertical + nHorizontal);

  itemBg.setRGB(0,0,255);
  multiSelect_ = false;

  setHScrollBarPolicy(nNoneVisible);
}


NListBox::~NListBox()
{
}

void NListBox::add( NCustomItem * component , bool align)
{
  listBoxPane_->add(component);
  component->mousePress.connect(this,&NListBox::onItemPress);
  component->setTransparent(true);
  component->setBackground(itemBg);
  if (align) listBoxPane_->resize();
}

void NListBox::add( NCustomItem * component )
{
  add(component,true);
}

void NListBox::onItemPress( NButtonEvent * ev)
{
  NVisualComponent* item = static_cast<NVisualComponent*>(ev->sender());
  if (!multiSelect_) deSelectItems();

  item->setTransparent(false);
  item->repaint();

  onItemSelected((NCustomItem*) (ev->sender()));
}

void NListBox::onItemSelected( NCustomItem * item )
{
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
    item->setTransparent(true);
    item->repaint();
  }
  selItems_.clear();
}

int NListBox::itemCount( )
{
  return listBoxPane_->componentSize();
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
}

std::vector< int > NListBox::selIndexList( )
{
  std::vector<int> indexList;
  int c = 0;
  for (std::vector<NCustomItem*>::iterator it = selItems_.begin(); it < selItems_.end(); it++) {
     NCustomItem* item = *it;
     indexList.push_back(item->zOrder());
  }
  return indexList;
}

void NListBox::setIndex( int i )
{
  deSelectItems();
  if (i>=0 && i < listBoxPane_->visualComponents().size()) {
     NVisualComponent* item = listBoxPane_->componentByZOrder(i);
     item->setTransparent(false);
     onItemSelected((NCustomItem*) item);
  }
}




