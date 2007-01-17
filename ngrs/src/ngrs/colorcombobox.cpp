/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "colorcombobox.h"
#include "coloritem.h"
#include "edit.h"
#include "app.h"
#include "itemevent.h"
#include <algorithm>

namespace ngrs {

  ColorComboBox::ColorComboBox()
    : ComboBox()
  {
    init();
  }

  ColorComboBox::~ColorComboBox()
  {
  }

  void ColorComboBox::init( )
  {
    add( new ColorItem( Color(0,0,0), "black" ) );
    add( new ColorItem( Color(0,0,255), "blue" ) );
    add( new ColorItem( Color(0,255,0), "green" ) );
    add( new ColorItem( Color(255,0,0), "red" ) );

    setIndex(0);

    Skin skin;
    skin.setTransparent(false);
    skin.setBackground(Color(255,255,255));
    edit()->setSkin(skin);
    edit()->setBackground( Color(0,0,0) );
  }

  void ColorComboBox::onItemClicked( ItemEvent * ev )
  {
    App::unmapPopupWindows();
    std::vector<ColorItem*>::iterator it = find(items.begin(), items.end(), ev->sender());
    if (it != items.end()) {
      ColorItem* item = *it;
      edit()->setBackground( item->color() );
      edit()->repaint();
      colorSelected.emit(item->color());
    }
  }

  void ColorComboBox::add( ColorItem * item )
  {
    ComboBox::add(item);
    items.push_back(item);
  }

  void ColorComboBox::removeChilds( )
  {
    items.clear();
    ComboBox::removeChilds();
  }

  void ColorComboBox::removeChild( VisualComponent * child )
  {
    std::vector<ColorItem*>::iterator it = find(items.begin(), items.end(), child);
    if (it != items.end()) items.erase(it);
    ComboBox::removeChild(child);
  }

  void ColorComboBox::erase( VisualComponent * child )
  {
    std::vector<ColorItem*>::iterator it = find(items.begin(), items.end(), child);
    if (it != items.end()) items.erase(it);
    ComboBox::erase(child);
  }

}
