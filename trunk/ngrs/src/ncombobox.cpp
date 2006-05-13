/***************************************************************************
 *   Copyright (C) 2005 by Stefan Nattkemper Nattkemper  *
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
#include "ncombobox.h"
#include "nimage.h"
#include "napp.h"
#include "nlabel.h"
#include "nedit.h"
#include "nbutton.h"
#include "nlistbox.h"
#include "nwindow.h"
#include "ncustomitem.h"
#include "nconfig.h"
#include "nframeborder.h"
#include "npopupwindow.h"

const char * arrow_down1_xpm[] = {
"12 6 2 1",
" 	c None",
".	c black",
"            ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            "};

NComboBox::NComboBox()
 : NPanel()
{
  init();
}


NComboBox::~NComboBox()
{
  delete border();
}

void NComboBox::init( )
{
  down.createFromXpmData(arrow_down1_xpm);
  NImage* downImg = new NImage();
  downImg->setBitmap(down);

  edit_    = new NEdit();
  NPanel::add(edit_);
  downBtn_ = new NButton(downImg,12,6);
    downBtn_->setWidth(15);
    downBtn_->setFlat(false);
    downBtn_->click.connect(this,&NComboBox::onDownBtnClicked);
  NPanel::add(downBtn_);

  popup = new NPopupWindow();
  NPanel::add(popup);

  lbox = new NListBox();
    lbox->setAlign(nAlClient);
    lbox->skin_ = NApp::config()->skin("clbox");
  popup->pane()->add(lbox);
}

// the class factories

extern "C" NObject* createComboBox() {
    return new NComboBox();
}

extern "C" void destroyComboBox(NObject* p) {
    delete p;
}


void NComboBox::resize( )
{
  downBtn_->setPosition(clientWidth()-downBtn_->width(),0,downBtn_->width(),clientHeight());
  edit_->setPosition(0,0,clientWidth()-downBtn_->width(),clientHeight());
}

int NComboBox::preferredHeight( ) const
{
  return edit_->preferredHeight() + spacing().top()+ spacing().bottom() + borderTop()+ borderBottom();
}

void NComboBox::onDownBtnClicked( NButtonEvent* ev )
{
  if (!NApp::popupUnmapped_) {
     NWindow* win = window();
     popup->setPosition(win->left()+absoluteLeft(), win->top()+absoluteTop()+height(),width(),100);
     popup->setVisible(true);
  }
}

void NComboBox::add( NCustomItem * item )
{
  lbox->add(item);
  lbox->itemSelected.connect(this,&NComboBox::onItemClicked);
}

void NComboBox::removeChilds( )
{
  lbox->removeChilds();
}

void NComboBox::onItemClicked( NItemEvent * ev)
{
  NApp::unmapPopupWindows();
  edit_->setText(ev->item()->text());
  edit_->repaint();
  itemSelected.emit(ev);
}

void NComboBox::setIndex( int i )
{
  lbox->setIndex(i);
  NCustomItem* item = lbox->itemAt(i);
  if (item!=0) {
    edit_->setText(item->text());
  }
}

int NComboBox::itemCount( )
{
  return lbox->itemCount();
}

int NComboBox::selIndex( ) const
{
  return lbox->selIndex();
}


