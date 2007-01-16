/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "splitbar.h"
#include "bevelborder.h"
#include "app.h"
#include "config.h"
#include <algorithm>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace ngrs {

  SplitBar::SplitBar()
    : Panel()
  {
    init();
    setOrientation(nVertical);
  }

  SplitBar::SplitBar( int orientation )
  {
    init();
    setOrientation(orientation);
  }

  SplitBar::~SplitBar()
  {
  }

  void SplitBar::init( )
  {
    setMoveable(Moveable(nMvHorizontal | nMvNoneRepaint | nMvParentLimit));
    setPreferredSize(5,5);
    setMinimumWidth(1);
    setMinimumHeight(1);
    skin_ = App::config()->skin("splitbar");
  }

  void SplitBar::onMove( const MoveEvent & moveEvent )
  {
    if (orientation_ == nVertical) {
      VisualComponent* leftVc = 0;
      VisualComponent* p = (VisualComponent*) parent();

      std::vector<VisualComponent*>::const_iterator itr;

      itr = find(p->visualComponents().begin(),p->visualComponents().end(),this);


      if ( align() == nAlRight ) {
        itr--;
        if (itr != p->visualComponents().end() ) {
          VisualComponent* rightVc = *itr;
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
        VisualComponent* topVc = 0;
        VisualComponent* p = (VisualComponent*) parent();

        std::vector<VisualComponent*>::const_iterator itr;

        itr = find(p->visualComponents().begin(),p->visualComponents().end(),this);


        if ( align() == nAlBottom ) {
          itr--;
          if (itr != p->visualComponents().end() ) {
            VisualComponent* bottomVc = *itr;
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

  void SplitBar::setOrientation( int orientation )
  {
    setGradientOrientation(!orientation);
    orientation_ = orientation;
    setPreferredSize(5,5);
    if (orientation == nVertical) {
      setMoveable(Moveable(nMvHorizontal | nMvNoneRepaint | nMvParentLimit));
      setCursor( nCrVSplit );
    } else
      if (orientation == nHorizontal) {
        setMoveable(Moveable(nMvVertical | nMvNoneRepaint | nMvParentLimit));
        setCursor( nCrHSplit );
      }
  }

}
