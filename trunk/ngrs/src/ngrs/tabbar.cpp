/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#include "tabbar.h"
#include "notebook.h"
#include "flowlayout.h"

namespace ngrs {

  TabBar::TabBar()
    : TogglePanel()
  {
    fl.setAlign(nAlLeft);
    fl.setBaseLine(nAlBottom);
    setLayout(fl);
    noteBook_ = 0;
    setTransparent(true);
    orientation_ = nAlTop;
  }


  TabBar::~TabBar()
  {
  }

  void TabBar::addTab( NTab * tab, VisualComponent* page )
  {
    TogglePanel::add(tab);

    if (orientation_ == nAlTop)     tab->setTextHAlign(nAlBottom);
    if (orientation_ == nAlBottom)  tab->setTextHAlign(nAlTop);

    tab->click.connect(this,&TabBar::onTabClick);
    pageMap_[tab] = page;

  }

  void TabBar::setNoteBook( NoteBook * noteBook )
  {
    noteBook_ = noteBook;
  }

  void TabBar::onTabClick( ButtonEvent * ev )
  {
    if (noteBook_!=0) {
      std::map< Object*, VisualComponent* >::iterator itr;
      if ( (itr = pageMap_.find(ev->sender())) == pageMap_.end())
      {
        // not my Tab
      } else {
        noteBook_->setActivePage(itr->second);
        noteBook_->repaint();
      }
    }
  }

  void TabBar::setOrientation( int orientation )
  {
    orientation_ = orientation;
    switch (orientation) {
    case nAlTop:
      fl.setBaseLine(nAlBottom);
      setLayout(fl);
      break;
    case nAlBottom:
      fl.setBaseLine(nAlTop);
      setLayout(fl);
      break;
    default : 
      fl.setBaseLine(nAlBottom);
      setLayout(fl);
    }
  }

  void TabBar::setActiveTab( VisualComponent * page )
  {
    std::map< Object*, VisualComponent* >::iterator itr = pageMap_.begin();
    for ( ; itr != pageMap_.end(); itr++) {
      if (itr->second == page) {
        setDown((CustomButton*)itr->first);
      }
    }
    repaint();
  }

  CustomButton * TabBar::tab( VisualComponent * page )
  {
    std::map< Object*, VisualComponent* >::iterator itr = pageMap_.begin();
    for ( ; itr != pageMap_.end(); itr++) {
      if (itr->second == page) {
        return (CustomButton*) itr->first;
      }
    }
    return 0;
  }

  void TabBar::setActiveTab( unsigned int index )
  {
    if (index < visualComponents().size() && index >= 0) {
      VisualComponent* tab = visualComponents().at(index);
      setDown((CustomButton*)tab);
      repaint();
    }
  }

  void TabBar::removeChild( VisualComponent * child )
  {
    std::map<Object*,VisualComponent*>::iterator itr = pageMap_.begin();
    for ( ; itr != pageMap_.end(); itr++) {
      if (itr->second == child) {
        pageMap_.erase(itr);
        break;
      }
    }

    TogglePanel::removeChild(child);
  }

}
