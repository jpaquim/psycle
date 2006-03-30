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
#include "nnotebook.h"

NNoteBook::NNoteBook()
 : visiblePage_(0), NPanel()
{
  setTransparent(true);
}


NNoteBook::~NNoteBook()
{
}

void NNoteBook::add( NPage * page )
{
  NPanel::add(page);
  if (visiblePage_!=0) {
    visiblePage_->setVisible(false);
  }
  visiblePage_ = page;
  page->setVisible(true);
}

void NNoteBook::resize( )
{
  if (visiblePage_!=0) {
    visiblePage_->setPosition(0,0,clientWidth(),clientHeight());
  }
}

void NNoteBook::setActivePage( NPage * page )
{
  if (page != 0) {
    std::vector<NRuntime*>::iterator it = find(components.begin(), components.end(), page);
    if (it == components.end()) {
    // That page is not in this container
    } else
    {
      if (visiblePage_!=0) {
        visiblePage_->setVisible(false);
      }
      visiblePage_ = static_cast<NPage*> (*it);
      page->setVisible(true);
      resize();
    }
  }
}


