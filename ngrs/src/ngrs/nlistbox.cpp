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
#include "ntablelayout.h"

NListBox::NListBox()
 : NScrollBox()
{
  listBoxPane_ = new NPanel();
  listBoxPane_->setBackground(NColor(255,255,255));
  listBoxPane_->setTransparent(false);
  setScrollPane(listBoxPane_);

  listBoxPane_->setLayout(NListLayout());
  listBoxPane_->setClientSizePolicy(nVertical + nHorizontal);

  multiSelect_ = false;

  setHScrollBarPolicy(nNoneVisible);

  itemBg = NApp::config()->skin("lbitemsel");
  itemFg = NApp::config()->skin("lbitemnone");
}


NListBox::~NListBox()
{
}

// the class factories

extern "C" NObject* createListBox() {
    return new NListBox();
}

extern "C" void destroyListBox(NObject* p) {
    delete p;
}


void NListBox::add( NCustomItem * component , bool align)
{
  listBoxPane_->add(component,nAlCenter);
  component->mousePress.connect(this,&NListBox::onItemPress);
  component->setTransparent(true);
  items_.push_back(component);
  if (align) listBoxPane_->resize();
}

void NListBox::insert( NCustomItem * component, int index , bool align )
{
  listBoxPane_->insert(component,index);

  if (index <= items_.size())
    items_.insert(items_.begin()+index,component);
  else
    items_.push_back(component);

  component->mousePress.connect(this,&NListBox::onItemPress);
  component->setTransparent(true);
  if (align) listBoxPane_->resize();
}


void NListBox::add( NCustomItem * component )
{
  add(component,true);
  component->mousePress.connect(this,&NListBox::onItemPress);
  component->setTransparent(true);
}

void NListBox::onItemPress( NButtonEvent * ev)
{
  if (ev->button() == 1) {
    NVisualComponent* item = static_cast<NVisualComponent*>(ev->sender());

    if (!multiSelect_ || !(NApp::system().keyState() & ControlMask)) deSelectItems();

    item->setSkin(itemBg);
    item->repaint();

    onItemSelected((NCustomItem*) (ev->sender()));
  } else {
      // todo add mousewheel code
  }
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
  items_.clear();
  selItems_.clear();
  listBoxPane_->removeChilds();
  listBoxPane_->setScrollDx(0);
  listBoxPane_->setScrollDy(0);
}

void NListBox::removeChild( NCustomItem * item )
{
  std::vector<NCustomItem*>::iterator it = find( items_.begin(), items_.end(), item );
  if ( it != items_.end() ) items_.erase(it);

  it = find( selItems_.begin(), selItems_.end(), item );
  if ( it != selItems_.end() ) selItems_.erase(it);

  listBoxPane_->removeChild(item);
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

int NListBox::selIndex( ) const
{
  int c = 0;
  if (selItems_.size()==0) return -1;
  for (std::vector<NVisualComponent*>::const_iterator it = listBoxPane_->visualComponents().begin(); it < listBoxPane_->visualComponents().end(); it++) {
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

void NListBox::setOrientation( int orientation )
{
  if (orientation == nHorizontal) {
    setHScrollBarPolicy(nAlwaysVisible);
  }
}

std::vector< NCustomItem * > & NListBox::items( )
{
  return items_;
}








