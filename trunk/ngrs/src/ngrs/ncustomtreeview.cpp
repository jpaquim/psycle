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

NCustomTreeView::NCustomTreeView()
 : NPanel()
{
  setLayout(NAlignLayout());

  scrollBox_ = new NScrollBox();
    scrollArea_ = new NPanel();
      scrollArea_->setLayout( NAlignLayout() );
      scrollArea_->setClientSizePolicy(nVertical + nHorizontal);
    scrollBox_->setScrollPane(scrollArea_);
  add(scrollBox_, nAlClient);

}


NCustomTreeView::~NCustomTreeView()
{
}


void NCustomTreeView::addNode( NTreeNode * node )
{
  scrollArea_->add( node, nAlTop );
}


