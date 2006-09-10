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
#include "ncustomtreeview.h"
#include "ntreenode.h"
#include "nalignlayout.h"
#include "nautoscrolllayout.h"
#include "nscrollbox.h"
#include "ncustomitem.h"
#include "napp.h"
#include "nconfig.h"
#include "nwindow.h"


NCustomTreeView::NCustomTreeView()
 : NPanel()
{
  setLayout( NAlignLayout() );

  scrollBox_ = new NScrollBox();
    scrollArea_ = new NPanel();
      scrollArea_->setLayout( NAlignLayout() );
      scrollArea_->setClientSizePolicy(nVertical + nHorizontal);
      scrollArea_->setBackground(NColor(255,255,255));
      scrollArea_->setTransparent(false);
    scrollBox_->setScrollPane(scrollArea_);
  add(scrollBox_, nAlClient);

  selectedItem_ = 0;
  selectedTreeNode_ = 0;

  itemBg = NApp::config()->skin("lbitemsel");
  itemFg = NApp::config()->skin("lbitemnone");

}


NCustomTreeView::~NCustomTreeView()
{
}


void NCustomTreeView::addNode( NTreeNode * node )
{
  scrollArea_->add( node, nAlTop );
  node->itemSelected.connect(this,&NCustomTreeView::onSelectedItem);
}

void NCustomTreeView::onSelectedItem(NTreeNode* node, NCustomItem * sender )
{
  selectedTreeNode_ = node;

  sender->setSkin(itemBg);
  sender->repaint();


  if (selectedItem_ && sender!=selectedItem_ ) {
    selectedItem_->setSkin(itemFg);
    selectedItem_->repaint();
  }

  selectedItem_ = sender;

  NItemEvent ev(sender,sender->text());
  itemSelected.emit(&ev);
}

NTreeNode * NCustomTreeView::selectedTreeNode( )
{
  return selectedTreeNode_;
}

NCustomItem * NCustomTreeView::selectedItem( )
{
  return selectedItem_;
}

void NCustomTreeView::removeChilds( )
{
  scrollArea_->removeChilds();
  selectedItem_ = 0;
  selectedTreeNode_ = 0;
}

void NCustomTreeView::removeItem( NCustomItem * item )
{
  if ( item && item->isChildOf( this ) ) {
    if ( item == selectedItem_ ) selectedItem_ = 0;
	  item->erase();
	  if ( window() ) window()->checkForRemove( item );
	  NApp::addRemovePipe(item);
  }
}
