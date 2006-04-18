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


NTreeNode::NodePanel::NodePanel( NTreeNode * node ) : node_(node)
{
  setLayout(new NAlignLayout());

  pane_ = new NPanel();
     pane_->setLayout(new NAlignLayout(),true);
  add( pane_, nAlTop);

  childNodePane_ = new NPanel();
    childNodePane_->setLayout(new NAlignLayout(),true);
  add( childNodePane_, nAlTop);
}

NTreeNode::NodePanel::~ NodePanel( )
{
}


int NTreeNode::NodePanel::minimumWidth( ) const
{
  return node_->pane()->preferredWidth();
}

int NTreeNode::NodePanel::minimumHeight( ) const
{
  return node_->pane()->preferredHeight();
}



NTreeNode::NTreeNode()
 : NFlipBox()
{
  setLayout(new NAlignLayout(),true);
  setBorder(new NFrameBorder(),true);

  panel_ = new NodePanel(this);

  setFlipPane(panel_);


}


NTreeNode::~NTreeNode()
{
}

NPanel * NTreeNode::pane( )
{
  return panel_->pane_;
}

NPanel * NTreeNode::childNodePane( )
{
  return panel_->childNodePane_;
}










