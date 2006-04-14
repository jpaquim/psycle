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
#include "nsplitbar.h"
#include "nbevelborder.h"

NSplitBar::NSplitBar()
 : NPanel()
{
  setMoveable(NMoveable(nMvHorizontal | nMvNoneRepaint));
  splitBorder = new NBevelBorder(nRaised,nLowered);
  setBorder(splitBorder);
  setWidth(5);
}


NSplitBar::~NSplitBar()
{
  delete splitBorder;
}


void NSplitBar::onMove( const NMoveEvent & moveEvent )
{
  NVisualComponent* leftVc = 0;

  NVisualComponent* p = (NVisualComponent*) parent();

  std::vector<NVisualComponent*>::const_iterator itr;

  itr = find(p->visualComponents().begin(),p->visualComponents().end(),this);

  if (itr > p->visualComponents().begin()) {
    itr--;
    leftVc = *itr;
  }

  if (leftVc!=0) {
     leftVc->setPreferredSize( left() - leftVc->left(), leftVc->height());
     p->resize();
     p->repaint();
  }
}


