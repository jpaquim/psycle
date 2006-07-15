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
#include "ntreenode.h"
#include "nalignlayout.h"
#include "nframeborder.h"
#include "nlabel.h"



NTreeNode::NTreeNode()
 : NFlipBox()
{
  entries_ = new NPanel();
     entries_->setLayout( NAlignLayout() );
     entries_->setSpacing(flipperWidth(),0,0,0);
  pane()->add(entries_, nAlTop);

  subNodes = new NPanel();
    subNodes->setLayout( NAlignLayout() );
  pane()->add(subNodes, nAlTop);

}

NTreeNode::~NTreeNode()
{
}

void NTreeNode::addNode( NTreeNode * node )
{
  subNodes->add(node, nAlTop);
}

void NTreeNode::addEntry( NVisualComponent * entry )
{
  entries_->add(entry, nAlTop);
}











