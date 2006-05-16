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
#include "ngrs/nnotebook.h"

NNoteBook::NNoteBook()
 :  NPanel(), visiblePage_(0)
{
  setTransparent(true);
}


NNoteBook::~NNoteBook()
{
}

void NNoteBook::add( NPanel * page )
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

void NNoteBook::setActivePage( NPanel * page )
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
      visiblePage_ = static_cast<NPanel*> (*it);
      page->setVisible(true);
      resize();
    }
  }
}

void NNoteBook::setActivePage( unsigned int index )
{
  if (index < visualComponents().size() && index >= 0) {
     if (visiblePage_!=0) {
        visiblePage_->setVisible(false);
      }
     visiblePage_ = static_cast<NPanel*> (visualComponents().at(index));
     visiblePage_->setVisible(true);
     resize();
  }
}

int NNoteBook::preferredWidth( ) const
{
  if (visiblePage_) {
        return std::max(1,visiblePage_->preferredWidth());
  } else return 10;
}

int NNoteBook::preferredHeight( ) const
{
  if (visiblePage_) {
        return std::max(1,visiblePage_->preferredHeight());
  } else return 10;
}


