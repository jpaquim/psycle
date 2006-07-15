/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "ncolorchooser.h"
#include "nbutton.h"
#include "nimage.h"
#include "npopupwindow.h"
#include "napp.h"
#include "nalignlayout.h"

const char * arrow_down2_xpm[] = {
"12 6 2 1",
" 	c None",
".	c black",
"            ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            "};


NColorComboBox::NColorComboBox()
 : NPanel()
{
  init();
}


NColorComboBox::~NColorComboBox()
{
}

void NColorComboBox::init( )
{
  setLayout( NAlignLayout() );

  down.createFromXpmData(arrow_down2_xpm);
  NImage* downImg = new NImage();
  downImg->setBitmap(down);

  downBtn_ = new NButton(downImg,12,6);
    downBtn_->setWidth(15);
    downBtn_->setFlat(false);
    downBtn_->click.connect(this,&NColorComboBox::onDownBtnClicked);
  NPanel::add(downBtn_, nAlRight);

  colorPanel_   = new NPanel();
    colorPanel_->setTransparent(false);
    colorPanel_->setBackground(NColor(0,0,0));
  NPanel::add(colorPanel_ , nAlClient);

  popup = new NPopupWindow();
  NPanel::add(popup);

  colorChooser = new NColorChooser();
    colorChooser->colorSelected.connect(this,&NColorComboBox::onColorSelected);
  popup->pane()->add(colorChooser, nAlClient);
}

void NColorComboBox::onDownBtnClicked( NButtonEvent * ev )
{
  if (!NApp::popupUnmapped_) {
     NWindow* win = window();
     popup->setPosition(win->left()+absoluteLeft(), win->top()+absoluteTop()+height(),width(),100);
     popup->setVisible(true);
  }
}

void NColorComboBox::onColorSelected( const NColor & color )
{
  colorPanel_->setBackground(color);
  colorPanel_->repaint();
  colorSelected.emit(color);
}



