/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper                   *
 *   Made in Germany                                                       *
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
#ifndef NCOMBOBOX_H
#define NCOMBOBOX_H

#include "customcombobox.h"

namespace ngrs {

  class CustomItem;
  class Button;
  class Edit;
  class ListBox;
  class ItemEvent;
  class PopupWindow;

  /**
  @author Stefan
  */

  class ComboBox : public CustomComboBox
  {
  public:
    ComboBox();

    ~ComboBox();

    signal1<ItemEvent*> itemSelected;

    const std::string & text() const;

    virtual void resize();
    virtual int preferredHeight() const;

    virtual void add(CustomItem* item);
    virtual void removeChilds();

    void setIndex(int i);
    int selIndex() const;
    int itemCount();
    CustomItem* itemAt(unsigned int index);

    std::vector<CustomItem*> & items();

    virtual void onItemClicked(ItemEvent * ev);

  protected:

    Edit* edit();

  private:

    Bitmap down;

    Edit*   edit_;
    Button* downBtn_;

    ListBox* lbox;
    PopupWindow*  popup;

    void init();

    void onDownBtnClicked(ButtonEvent * ev);

  };

}

#endif
