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
#include "ncolorcombobox.h"
#include "ncoloritem.h"
#include "nedit.h"
#include "napp.h"
#include "nitemevent.h"
#include <algorithm>

namespace ngrs {

  NColorComboBox::NColorComboBox()
    : NComboBox()
  {
    init();
  }

  NColorComboBox::~NColorComboBox()
  {
  }

  void NColorComboBox::init( )
  {
    add( new NColorItem( NColor(0,0,0), "black" ) );
    add( new NColorItem( NColor(0,0,255), "blue" ) );
    add( new NColorItem( NColor(0,255,0), "green" ) );
    add( new NColorItem( NColor(255,0,0), "red" ) );

    setIndex(0);

    NSkin skin;
    skin.setTransparent(false);
    skin.setBackground(NColor(255,255,255));
    edit()->setSkin(skin);
    edit()->setBackground( NColor(0,0,0) );
  }

  void NColorComboBox::onItemClicked( NItemEvent * ev )
  {
    NApp::unmapPopupWindows();
    std::vector<NColorItem*>::iterator it = find(items.begin(), items.end(), ev->sender());
    if (it != items.end()) {
      NColorItem* item = *it;
      edit()->setBackground( item->color() );
      edit()->repaint();
      colorSelected.emit(item->color());
    }
  }

  void NColorComboBox::add( NColorItem * item )
  {
    NComboBox::add(item);
    items.push_back(item);
  }

  void NColorComboBox::removeChilds( )
  {
    items.clear();
    NComboBox::removeChilds();
  }

  void NColorComboBox::removeChild( NVisualComponent * child )
  {
    std::vector<NColorItem*>::iterator it = find(items.begin(), items.end(), child);
    if (it != items.end()) items.erase(it);
    NComboBox::removeChild(child);
  }

  void NColorComboBox::erase( NVisualComponent * child )
  {
    std::vector<NColorItem*>::iterator it = find(items.begin(), items.end(), child);
    if (it != items.end()) items.erase(it);
    NComboBox::erase(child);
  }

}
