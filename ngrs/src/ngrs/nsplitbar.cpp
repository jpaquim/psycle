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
#include <algorithm>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

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
  setPreferredSize(5,5);
  setMinimumWidth(1);
  setMinimumHeight(1);
  skin_ = NApp::config()->skin("splitbar");
}

void NSplitBar::onMove( const NMoveEvent & moveEvent )
{
  if (orientation_ == nVertical) {
    NVisualComponent* leftVc = 0;
    NVisualComponent* p = (NVisualComponent*) parent();

    std::vector<NVisualComponent*>::const_iterator itr;

    itr = find(p->visualComponents().begin(),p->visualComponents().end(),this);


    if ( align() == nAlRight ) {
        itr--;
        if (itr != p->visualComponents().end() ) {
          NVisualComponent* rightVc = *itr;
          rightVc->setPreferredSize( std::max( rightVc->width() - ( left() + width() - rightVc->left() ), rightVc->minimumWidth()), rightVc->height() );
          p->resize();
          p->repaint();
        }
    } else
    if (itr > p->visualComponents().begin()) {
      itr--;
      leftVc = *itr;
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


    if ( align() == nAlBottom ) {
        itr--;
        if (itr != p->visualComponents().end() ) {
          NVisualComponent* bottomVc = *itr;
          bottomVc->setPreferredSize( bottomVc->width(), std::max( bottomVc->height() - ( top() + height() - bottomVc->top() ), bottomVc->minimumHeight()) );
          p->resize();
          p->repaint();
        }
    } else
    if (itr > p->visualComponents().begin()) {
      itr--;
      topVc = *itr;
      topVc->setPreferredSize( topVc->width(), top() - topVc->top() );
      p->resize();
      p->repaint();
     }

  }
}

void NSplitBar::setOrientation( int orientation )
{
  setGradientOrientation(!orientation);
  orientation_ = orientation;
  setPreferredSize(5,5);
  if (orientation == nVertical) {
     setMoveable(NMoveable(nMvHorizontal | nMvNoneRepaint | nMvParentLimit));
     setCursor( nCrVSplit );
  } else
  if (orientation == nHorizontal) {
     setMoveable(NMoveable(nMvVertical | nMvNoneRepaint | nMvParentLimit));
     setCursor( nCrHSplit );
  }
}






