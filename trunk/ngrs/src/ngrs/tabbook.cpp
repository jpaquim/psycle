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
#include "tabbook.h"
#include "tabbar.h"
#include "alignlayout.h"
#include "button.h"

namespace ngrs {

  TabBook::TabBook()
    : Panel()
  {
    setLayout(AlignLayout());

    tabBar_ = new TabBar();
    add(tabBar_, nAlTop);

    book_ = new NoteBook();
    add(book_,nAlClient);

    tabBar_->setNoteBook(book_);
  }


  TabBook::~TabBook()
  {
  }

  Panel * TabBook::addNewPage( const std::string & tabName )
  {
    NTab* tab = new NTab();
    tab->setText(tabName);
    Panel* page = new Panel();
    tabBar_->addTab(tab, page);
    book_->add(page);
    return page;
  }

  void TabBook::removePage( VisualComponent * page )
  {
    NTab* tabbutton = tab( page );
    if ( tabbutton ) {
      tabBar_->removeChild( tabbutton );
      book_->removeChild( page );
      if ( book_->visualComponents().size() > 0 ) {
        setActivePage( book_->visualComponents().back() );
      }
    }
  }

  void TabBook::addPage( VisualComponent * page, const std::string & tabName )
  {
    NTab* tab = new NTab();
    tab->setText(tabName);
    tabBar_->addTab(tab, page);
    book_->add(page);
  }

  void TabBook::setActivePage( VisualComponent * page )
  {
    book_->setActivePage(page);
    tabBar_->setActiveTab(page);
  }

  void TabBook::setActivePage( int index )
  {
    book_->setActivePage(index);
    tabBar_->setActiveTab(index);
  }

  void TabBook::setTabBarAlign( int align )
  {
    tabBar_->setAlign(align);
    tabBar_->setOrientation(align);
    updateAlign();
  }

  NTab * TabBook::tab( VisualComponent * page )
  {
    return (NTab*) tabBar_->tab(page);
  }

  void TabBook::setTabBarVisible( bool on )
  {
    tabBar_->setVisible(on);
  }

  bool TabBook::tabBarVisible( ) const
  {
    return tabBar_->visible();
  }

  VisualComponent * TabBook::activePage( )
  {
    return book_->activePage();
  }

  void TabBook::setTabBar( TabBar* bar ) {
     removeChild( tabBar_ );
     tabBar_ = bar;
     tabBar_->setNoteBook(book_);
  }

}
