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
#include "ntabbook.h"
#include "ntabbar.h"
#include "nalignlayout.h"

NTabBook::NTabBook()
 : NPanel()
{
   alignLayout = new NAlignLayout();
   setLayout(alignLayout);

   tabBar_ = new NTabBar();
   tabBar_->setAlign(nAlTop);
   add(tabBar_);

   book_ = new NNoteBook();
   add(book_);
   book_->setAlign(nAlClient);

   tabBar_->setNoteBook(book_);

   setTransparent(true);
}


NTabBook::~NTabBook()
{
  delete alignLayout;
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

void NTabBook::addPage( NPanel * page, const std::string & tabName )
{
  NTab* tab = new NTab();
  tab->setText(tabName);
  tabBar_->addTab(tab, page);
  book_->add(page);
}

void NTabBook::setActivePage( NPanel * page )
{
  book_->setActivePage(page);
  tabBar_->setActiveTab(page);
}

void NTabBook::setTabBarAlign( int align )
{
  tabBar_->setAlign(align);
  tabBar_->setOrientation(align);
  updateAlign();
}

NCustomButton * NTabBook::tab( NPanel * page )
{
  return tabBar_->tab(page);
}


