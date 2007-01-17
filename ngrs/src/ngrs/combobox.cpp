/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper                   *
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
#include "combobox.h"
#include "image.h"
#include "app.h"
#include "label.h"
#include "edit.h"
#include "button.h"
#include "listbox.h"
#include "window.h"
#include "customitem.h"
#include "config.h"
#include "frameborder.h"
#include "popupwindow.h"

namespace ngrs {

  const char * arrow_down1_xpm[] = {
    "12 6 2 1",
    " 	c None",
    ".	c black",
    "            ",
    "  .......   ",
    "   .....    ",
    "    ...     ",
    "     .      ",
    "            "
  };

  ComboBox::ComboBox()
    : CustomComboBox()
  {
    init();
  }

  ComboBox::~ComboBox()
  {
  }

  void ComboBox::init( )
  {
    down.createFromXpmData(arrow_down1_xpm);
    Image* downImg = new Image();
    downImg->setBitmap(down);

    edit_    = new Edit();
    Panel::add(edit_);
    downBtn_ = new Button( downImg, 12, 6 );
    downBtn_->setWidth(15);
    downBtn_->setFlat(false);
    downBtn_->click.connect(this,&ComboBox::onDownBtnClicked);
    downBtn_->setSkin( 
      App::config()->skin( "ccbx_btn_up" ),
      App::config()->skin( "ccbx_btn_down" ),
      App::config()->skin( "ccbx_btn_over" ),
      App::config()->skin( "ccbx_btn_flat" )	
      );
    Panel::add(downBtn_);

    popup = new PopupWindow();
    Panel::add(popup);

    lbox = new ListBox();
    lbox->setAlign(nAlClient);
    lbox->skin_ = App::config()->skin("clbox");
    lbox->itemSelected.connect(this,&ComboBox::onItemClicked);
    popup->pane()->add(lbox);
  }

  void ComboBox::resize( )
  {
    downBtn_->setPosition(clientWidth()-downBtn_->width(),0,downBtn_->width(),clientHeight());
    edit_->setPosition(0,0,clientWidth()-downBtn_->width(),clientHeight());
  }

  int ComboBox::preferredHeight( ) const
  {
    return edit_->preferredHeight() + spacing().top()+ spacing().bottom() + borderTop()+ borderBottom();
  }

  void ComboBox::onDownBtnClicked( ButtonEvent* ev )
  {
    if (!App::popupUnmapped_) {
      Window* win = window();
      popup->setPosition(win->left()+absoluteLeft(), win->top()+absoluteTop()+height(),width(),100);
      popup->setVisible(true);
    }
  }

  void ComboBox::add( CustomItem * item )
  {
    lbox->add(item);
  }

  void ComboBox::removeChilds( )
  {
    lbox->removeChilds();
    edit_->setText("");
  }

  void ComboBox::onItemClicked( ItemEvent * ev)
  {
    App::unmapPopupWindows();
    edit_->setText(ev->item()->text());
    edit_->repaint();
    itemSelected.emit(ev);
  }

  void ComboBox::setIndex( int i )
  {
    lbox->setIndex(i);
    CustomItem* item = lbox->itemAt(i);
    if (item!=0) {
      edit_->setText(item->text());
    }
  }

  int ComboBox::itemCount( )
  {
    return lbox->itemCount();
  }

  int ComboBox::selIndex( ) const
  {
    return lbox->selIndex();
  }

  const std::string & ComboBox::text( ) const
  {
    return edit_->text();
  }

  Edit * ComboBox::edit( )
  {
    return edit_;
  }

  std::vector< CustomItem * > & ComboBox::items( )
  {
    return lbox->items();
  }

  CustomItem * ComboBox::itemAt( unsigned int index )
  {
    return lbox->itemAt( index );
  }

}

// the class factories

extern "C" ngrs::Object* createComboBox() {
  return new ngrs::ComboBox();
}

extern "C" void destroyComboBox( ngrs::Object* p ) {
  delete p;
}