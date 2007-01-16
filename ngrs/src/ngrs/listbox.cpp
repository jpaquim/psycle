/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "listbox.h"
#include "buttonevent.h"
#include "app.h"
#include "config.h"
#include "tablelayout.h"
#include "scrollbar.h"
#include <algorithm>

namespace ngrs {

  ListBox::ListBox()
    : ScrollBox()
  {
    listBoxPane_ = new Panel();
    listBoxPane_->setBackground(Color(255,255,255));
    listBoxPane_->setTransparent(false);
    setScrollPane(listBoxPane_);

    listBoxPane_->setLayout(ListLayout());
    listBoxPane_->setClientSizePolicy(nVertical + nHorizontal);

    multiSelect_ = false;

    setHScrollBarPolicy(nNoneVisible);

    itemBg = App::config()->skin("lbitemsel");
    itemFg = App::config()->skin("lbitemnone");
  }


  ListBox::~ListBox()
  {
  }

  void ListBox::add( CustomItem * component , bool align)
  {
    listBoxPane_->add( component,nAlCenter, false);
    component->mousePress.connect(this,&ListBox::onItemPress);
    component->setTransparent(true);
    items_.push_back(component);
    if (align) listBoxPane_->resize();
  }

  void ListBox::insert( CustomItem * component, int index , bool align )
  {
    listBoxPane_->insert(component,index);

    if ( index <= items_.size() )
      items_.insert( items_.begin() + index, component );
    else
      items_.push_back( component );

    component->mousePress.connect(this,&ListBox::onItemPress);
    component->setTransparent(true);
    if ( align ) listBoxPane_->resize();
  }


  void ListBox::add( CustomItem * component )
  {
    add(component,true);
    component->mousePress.connect(this,&ListBox::onItemPress);
    component->setTransparent(true);
  }

  void ListBox::onItemPress( ButtonEvent * ev)
  {
    if (ev->button() == 1) {
      VisualComponent* item = static_cast<VisualComponent*>(ev->sender());

      if ( !multiSelect_ 
#ifdef __unix__
        || !(App::system().keyState() & ControlMask)
#endif   
        ) deSelectItems();

      item->setSkin(itemBg);
      item->repaint();

      onItemSelected((CustomItem*) (ev->sender()));
    } else {
      // todo add mousewheel code
    }
  }

  void ListBox::onItemSelected( CustomItem * item )
  {
    if (!multiSelect_) deSelectItems();
    selItems_.push_back(item);
    ItemEvent ev(item,item->text());
    itemSelected.emit(&ev);
  }

  void ListBox::removeChilds( )
  {
    items_.clear();
    selItems_.clear();
    listBoxPane_->removeChilds();
    listBoxPane_->setScrollDx(0);
    listBoxPane_->setScrollDy(0);
  }

  void ListBox::removeChild( CustomItem * item )
  {
    std::vector<CustomItem*>::iterator it = find( items_.begin(), items_.end(), item );
    if ( it != items_.end() ) items_.erase(it);

    it = find( selItems_.begin(), selItems_.end(), item );
    if ( it != selItems_.end() ) selItems_.erase(it);

    listBoxPane_->removeChild(item);
  }

  void ListBox::deSelectItems( )
  {
    for (std::vector<CustomItem*>::iterator it = selItems_.begin(); it < selItems_.end(); it++) {
      CustomItem* item = *it;
      item->setSkin(itemFg);
      item->repaint();
    }
    selItems_.clear();
  }

  int ListBox::itemCount( )
  {
    return listBoxPane_->visualComponents().size();
  }

  int ListBox::selIndex( ) const
  {
    int c = 0;
    if (selItems_.size()==0) return -1;
    for (std::vector<VisualComponent*>::const_iterator it = listBoxPane_->visualComponents().begin(); it < listBoxPane_->visualComponents().end(); it++) {
      if (*it == selItems_.at(0)) {
        return c;
      }
      c++;
    }
    return -1;
  }

  std::vector< int > ListBox::selIndexList( )
  {
    std::vector<int> indexList;
    for (std::vector<CustomItem*>::iterator it = selItems_.begin(); it < selItems_.end(); it++) {
      CustomItem* item = *it;
      indexList.push_back(item->zOrder());
    }
    return indexList;
  }

  void ListBox::setIndex( unsigned int i )
  {
    if (!multiSelect_) deSelectItems();
    if (i>=0 && i < listBoxPane_->visualComponents().size()) {
      CustomItem* item = (CustomItem*) listBoxPane_->componentByZOrder(i); ///todo avoid cast
      selItems_.push_back(item);
      item->setSkin(itemBg);
      item->repaint();
    }
  }

  void ListBox::setMultiSelect( bool on )
  {
    multiSelect_ = on;
  }

  void ListBox::selClear( )
  {
    deSelectItems();
  }

  CustomItem * ListBox::itemAt( unsigned int index )
  {
    if (index >= 0 && index < listBoxPane_->visualComponents().size())
      return static_cast<CustomItem*> (listBoxPane_->visualComponents().at(index));
    else
      return 0;
  }

  void ListBox::setOrientation( int orientation )
  {
    if (orientation == nHorizontal) {
      setHScrollBarPolicy(nAlwaysVisible);
    }
  }

  std::vector< CustomItem * > & ListBox::items( )
  {
    return items_;
  }

  void ListBox::resize( )
  {
    ScrollBox::resize();
    if ( items_.size() > 0 ) {
      CustomItem* item = *(items_.begin());
      horBar()->setLargeChange( item->height() );
      horBar()->setSmallChange( item->height() );
      verBar()->setLargeChange( item->height() );
      verBar()->setSmallChange( item->height() );
    }
  }

}


// the class factories
extern "C" ngrs::Object* createListBox() {
  return new ngrs::ListBox();
}

extern "C" void destroyListBox( ngrs::Object* p ) {
  delete p;
}
