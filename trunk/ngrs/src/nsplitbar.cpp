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
#include "napp.h"
#include "nconfig.h"

NSplitBar::NSplitBar()
 : NPanel()
{
  init();
  setOrientation(nVertical);
}

NSplitBar::NSplitBar( int orientation )
{
  init();
  setOrientation(orientation);
}

NSplitBar::~NSplitBar()
{
}

void NSplitBar::init( )
{
  setMoveable(NMoveable(nMvHorizontal | nMvNoneRepaint | nMvParentLimit));
  setWidth(5);
  NApp::config()->setSkin(&skin_,"splitbar");
  
}

void NSplitBar::onMove( const NMoveEvent & moveEvent )
{
  if (orientation_ == nVertical) {
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
  } else
  if (orientation_ == nHorizontal) {
    NVisualComponent* topVc = 0;
    NVisualComponent* p = (NVisualComponent*) parent();

    std::vector<NVisualComponent*>::const_iterator itr;

    itr = find(p->visualComponents().begin(),p->visualComponents().end(),this);

    if (itr > p->visualComponents().begin()) {
      itr--;
      topVc = *itr;
    }

    if (topVc!=0) {
      topVc->setPreferredSize( topVc->width(), top() - topVc->top());
      p->resize();
      p->repaint();
    }

  }
}

void NSplitBar::setOrientation( int orientation )
{
  setGradientOrientation(!orientation);
  orientation_ = orientation;
  if (orientation == nVertical) {
     setWidth(5);
     setMoveable(NMoveable(nMvHorizontal | nMvNoneRepaint | nMvParentLimit));
  } else
  if (orientation == nHorizontal) {
     setHeight(5);
     setMoveable(NMoveable(nMvVertical | nMvNoneRepaint | nMvParentLimit));
  }
}






