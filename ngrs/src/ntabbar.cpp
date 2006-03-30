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
#include "ntabbar.h"
#include "nnotebook.h"
#include "nflowlayout.h"


NTabBar::NTabBar()
 : NTogglePanel()
{
  fl = new NFlowLayout(nAlLeft);
  fl->setBaseLine(nAlBottom);
  setLayout(fl);
  noteBook_ = 0;
  setTransparent(true);
  orientation_ = nAlTop;
}


NTabBar::~NTabBar()
{
  delete fl;
}

void NTabBar::addTab( NTab * tab, NPage* page )
{
  NTogglePanel::add(tab);

  if (orientation_ == nAlTop)     tab->setTextHAlign(nAlBottom);
  if (orientation_ == nAlBottom)  tab->setTextHAlign(nAlTop);

  tab->click.connect(this,&NTabBar::onTabClick);
  pageMap_[tab] = page;
}

void NTabBar::setNoteBook( NNoteBook * noteBook )
{
  noteBook_ = noteBook;
}

void NTabBar::onTabClick( NButtonEvent * ev )
{
 if (noteBook_!=0) {
   std::map<NObject*,NPage*>::iterator itr;
   if ( (itr = pageMap_.find(ev->sender())) == pageMap_.end())
   {
       // not my Tab
   } else {
     noteBook_->setActivePage(itr->second);
     noteBook_->repaint();
   }
 }
}

void NTabBar::setOrientation( int orientation )
{
  orientation_ = orientation;
  switch (orientation) {
    case nAlTop:
      fl->setBaseLine(nAlBottom);
    break;
    case nAlBottom:
      fl->setBaseLine(nAlTop);
    break;
    default : fl->setBaseLine(nAlBottom);
  }
}


