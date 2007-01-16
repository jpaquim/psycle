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
#ifndef NCOLORCOMBOBOX_H
#define NCOLORCOMBOBOX_H

#include "combobox.h"

/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class ColorComboBox : public ComboBox
  {
  public:
    ColorComboBox();

    ~ColorComboBox();

    signal1<const Color &> colorSelected;

    virtual void onItemClicked(ItemEvent * ev);
    virtual void add(class ColorItem* item);

    virtual void removeChilds();
    virtual void removeChild( VisualComponent * child );
    virtual void erase(VisualComponent* child);

  private:

    void init();

    std::vector<class ColorItem*> items;

  };

}

#endif
