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
#include "ntabbook.h"
#include "ntabbar.h"
#include "nalignlayout.h"
#include "nbutton.h"

namespace ngrs {

  NTabBook::NTabBook()
    : NPanel()
  {
    setLayout(NAlignLayout());

    tabBar_ = new NTabBar();
    add(tabBar_, nAlTop);

    book_ = new NNoteBook();
    add(book_,nAlClient);

    tabBar_->setNoteBook(book_);
  }


  NTabBook::~NTabBook()
  {
  }

  NPanel * NTabBook::addNewPage( const std::string & tabName )
  {
    NTab* tab = new NTab();
    tab->setText(tabName);
    NPanel* page = new NPanel();
    tabBar_->addTab(tab, page);
    book_->add(page);
    return page;
  }

  void NTabBook::removePage( NVisualComponent * page )
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

  void NTabBook::addPage( NVisualComponent * page, const std::string & tabName )
  {
    NTab* tab = new NTab();
    tab->setText(tabName);
    tabBar_->addTab(tab, page);
    book_->add(page);
  }

  void NTabBook::setActivePage( NVisualComponent * page )
  {
    book_->setActivePage(page);
    tabBar_->setActiveTab(page);
  }

  void NTabBook::setActivePage( int index )
  {
    book_->setActivePage(index);
    tabBar_->setActiveTab(index);
  }

  void NTabBook::setTabBarAlign( int align )
  {
    tabBar_->setAlign(align);
    tabBar_->setOrientation(align);
    updateAlign();
  }

  NTab * NTabBook::tab( NVisualComponent * page )
  {
    return (NTab*) tabBar_->tab(page);
  }

  void NTabBook::setTabBarVisible( bool on )
  {
    tabBar_->setVisible(on);
  }

  bool NTabBook::tabBarVisible( ) const
  {
    return tabBar_->visible();
  }

  NVisualComponent * NTabBook::activePage( )
  {
    return book_->activePage();
  }

}
